#include "runtime/renderer/common/vk_common.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <stdexcept>

#include "runtime/platform/filesystem.h"
#include "runtime/renderer/common/error.h"

namespace vkb {
bool is_depth_only_format(VkFormat format) {
  return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT;
}

bool is_depth_stencil_format(VkFormat format) {
  return format == VK_FORMAT_D16_UNORM_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT ||
         format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

bool is_depth_format(VkFormat format) {
  return is_depth_only_format(format) || is_depth_stencil_format(format);
}

VkFormat get_suitable_depth_format(
    VkPhysicalDevice physical_device, bool depth_only,
    const std::vector<VkFormat> &depth_format_priority_list) {
  VkFormat depth_format{VK_FORMAT_UNDEFINED};

  for (auto &format : depth_format_priority_list) {
    if (depth_only && !is_depth_only_format(format)) {
      continue;
    }

    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);

    // Format must support depth stencil attachment for optimal tiling
    if (properties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      depth_format = format;
      break;
    }
  }

  if (depth_format != VK_FORMAT_UNDEFINED) {
    /* LOGI("Depth format selected: {}", to_string(depth_format)); */
    return depth_format;
  }

  throw std::runtime_error("No suitable depth format could be determined");
}

bool is_dynamic_buffer_descriptor_type(VkDescriptorType descriptor_type) {
  return descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
         descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
}

bool is_buffer_descriptor_type(VkDescriptorType descriptor_type) {
  return descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
         descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
         is_dynamic_buffer_descriptor_type(descriptor_type);
}

namespace {
VkShaderStageFlagBits find_shader_stage(const std::string &ext) {
  if (ext == "vert") {
    return VK_SHADER_STAGE_VERTEX_BIT;
  } else if (ext == "frag") {
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  } else if (ext == "comp") {
    return VK_SHADER_STAGE_COMPUTE_BIT;
  } else if (ext == "geom") {
    return VK_SHADER_STAGE_GEOMETRY_BIT;
  } else if (ext == "tesc") {
    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
  } else if (ext == "tese") {
    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
  } else if (ext == "rgen") {
    return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
  } else if (ext == "rahit") {
    return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
  } else if (ext == "rchit") {
    return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
  } else if (ext == "rmiss") {
    return VK_SHADER_STAGE_MISS_BIT_KHR;
  } else if (ext == "rint") {
    return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
  } else if (ext == "rcall") {
    return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
  } else if (ext == "mesh") {
    return VK_SHADER_STAGE_MESH_BIT_EXT;
  } else if (ext == "task") {
    return VK_SHADER_STAGE_TASK_BIT_EXT;
  }
  throw std::runtime_error("File extension `" + ext +
                           "` does not have a vulkan shader stage.");
}
}  // namespace

VkShaderModule load_shader(const std::string &filename, VkDevice device,
                           VkShaderStageFlagBits stage) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = buffer.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(buffer.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

VkSurfaceFormatKHR select_surface_format(
    VkPhysicalDevice gpu, VkSurfaceKHR surface,
    std::vector<VkFormat> const &preferred_formats) {
  uint32_t surface_format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surface_format_count,
                                       nullptr);
  assert(0 < surface_format_count);
  std::vector<VkSurfaceFormatKHR> supported_surface_formats(
      surface_format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surface_format_count,
                                       supported_surface_formats.data());

  auto it = std::find_if(
      supported_surface_formats.begin(), supported_surface_formats.end(),
      [&preferred_formats](VkSurfaceFormatKHR surface_format) {
        return std::any_of(preferred_formats.begin(), preferred_formats.end(),
                           [&surface_format](VkFormat format) {
                             return format == surface_format.format;
                           });
      });

  // We use the first supported format as a fallback in case none of the
  // preferred formats is available
  return it != supported_surface_formats.end() ? *it
                                               : supported_surface_formats[0];
}

VkAccessFlags getAccessFlags(VkImageLayout layout) {
  switch (layout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      return 0;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      return VK_ACCESS_HOST_WRITE_BIT;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
      return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
      return VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      return VK_ACCESS_TRANSFER_READ_BIT;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      return VK_ACCESS_TRANSFER_WRITE_BIT;
    case VK_IMAGE_LAYOUT_GENERAL:
      assert(false &&
             "Don't know how to get a meaningful VkAccessFlags for "
             "VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
      return 0;
    default:
      assert(false);
      return 0;
  }
}

VkPipelineStageFlags getPipelineStageFlags(VkImageLayout layout) {
  switch (layout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
      return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      return VK_PIPELINE_STAGE_HOST_BIT;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
      return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
             VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
      return VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    case VK_IMAGE_LAYOUT_GENERAL:
      assert(false &&
             "Don't know how to get a meaningful VkPipelineStageFlags for "
             "VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
      return 0;
    default:
      assert(false);
      return 0;
  }
}

// Create an image memory barrier for changing the layout of
// an image and put it into an active command buffer
// See chapter 12.4 "Image Layout" for details
void image_layout_transition(VkCommandBuffer command_buffer, VkImage image,
                             VkPipelineStageFlags src_stage_mask,
                             VkPipelineStageFlags dst_stage_mask,
                             VkAccessFlags src_access_mask,
                             VkAccessFlags dst_access_mask,
                             VkImageLayout old_layout, VkImageLayout new_layout,
                             VkImageSubresourceRange const &subresource_range) {
  // Create an image barrier object
  VkImageMemoryBarrier image_memory_barrier{};
  image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  image_memory_barrier.srcAccessMask = src_access_mask;
  image_memory_barrier.dstAccessMask = dst_access_mask;
  image_memory_barrier.oldLayout = old_layout;
  image_memory_barrier.newLayout = new_layout;
  image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_memory_barrier.image = image;
  image_memory_barrier.subresourceRange = subresource_range;

  // Put barrier inside setup command buffer
  vkCmdPipelineBarrier(command_buffer, src_stage_mask, dst_stage_mask, 0, 0,
                       nullptr, 0, nullptr, 1, &image_memory_barrier);
}

void image_layout_transition(VkCommandBuffer command_buffer, VkImage image,
                             VkImageLayout old_layout, VkImageLayout new_layout,
                             VkImageSubresourceRange const &subresource_range) {
  VkPipelineStageFlags src_stage_mask = getPipelineStageFlags(old_layout);
  VkPipelineStageFlags dst_stage_mask = getPipelineStageFlags(new_layout);
  VkAccessFlags src_access_mask = getAccessFlags(old_layout);
  VkAccessFlags dst_access_mask = getAccessFlags(new_layout);

  image_layout_transition(command_buffer, image, src_stage_mask, dst_stage_mask,
                          src_access_mask, dst_access_mask, old_layout,
                          new_layout, subresource_range);
}

// Fixed sub resource on first mip level and layer
void image_layout_transition(VkCommandBuffer command_buffer, VkImage image,
                             VkImageLayout old_layout,
                             VkImageLayout new_layout) {
  VkImageSubresourceRange subresource_range = {};
  subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresource_range.baseMipLevel = 0;
  subresource_range.levelCount = 1;
  subresource_range.baseArrayLayer = 0;
  subresource_range.layerCount = 1;
  image_layout_transition(command_buffer, image, old_layout, new_layout,
                          subresource_range);
}

namespace gbuffer {
std::vector<LoadStoreInfo> get_load_all_store_swapchain() {
  // Load every attachment and store only swapchain
  std::vector<LoadStoreInfo> load_store{4};

  // Swapchain
  load_store[0].load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  load_store[0].store_op = VK_ATTACHMENT_STORE_OP_STORE;

  // Depth
  load_store[1].load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
  load_store[1].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Albedo
  load_store[2].load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
  load_store[2].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Normal
  load_store[3].load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
  load_store[3].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  return load_store;
}

std::vector<LoadStoreInfo> get_clear_all_store_swapchain() {
  // Clear every attachment and store only swapchain
  std::vector<LoadStoreInfo> load_store{5};

  // Swapchain
  load_store[0].load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  load_store[0].store_op = VK_ATTACHMENT_STORE_OP_STORE;

  // Depth
  load_store[1].load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  load_store[1].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Albedo
  load_store[2].load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  load_store[2].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Normal
  load_store[3].load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  load_store[3].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Metallic-Roughness
  load_store[4].load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  load_store[4].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  return load_store;
}

std::vector<LoadStoreInfo> get_clear_store_all() {
  // Clear and store every attachment
  std::vector<LoadStoreInfo> load_store{4};

  // Swapchain
  load_store[0].load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  load_store[0].store_op = VK_ATTACHMENT_STORE_OP_STORE;

  // Depth
  load_store[1].load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  load_store[1].store_op = VK_ATTACHMENT_STORE_OP_STORE;

  // Albedo
  load_store[2].load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  load_store[2].store_op = VK_ATTACHMENT_STORE_OP_STORE;

  // Normal
  load_store[3].load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  load_store[3].store_op = VK_ATTACHMENT_STORE_OP_STORE;

  return load_store;
}

std::vector<VkClearValue> get_clear_value() {
  // Clear values
  std::vector<VkClearValue> clear_value{5};
  clear_value[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clear_value[1].depthStencil = {0.0f, ~0U};
  clear_value[2].color = {{1.0f, 1.0f, 1.0f, 1.0f}};
  clear_value[3].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clear_value[4].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

  return clear_value;
}
}  // namespace gbuffer

}  // namespace vkb
