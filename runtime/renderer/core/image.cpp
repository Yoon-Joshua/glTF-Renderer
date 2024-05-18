#include "runtime/renderer/core/image.h"
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VK_NO_PROTOTYPES
#include <vma/vk_mem_alloc.h>

#include <cassert>
#include <utility>

#include "runtime/renderer/common/error.h"
#include "runtime/renderer/core/device.h"
#include "runtime/renderer/core/image_view.h"

namespace vkb {
namespace {
inline VkImageType find_image_type(VkExtent3D extent) {
  VkImageType result{};

  uint32_t dim_num{0};

  if (extent.width >= 1) {
    dim_num++;
  }

  if (extent.height >= 1) {
    dim_num++;
  }

  if (extent.depth > 1) {
    dim_num++;
  }

  switch (dim_num) {
    case 1:
      result = VK_IMAGE_TYPE_1D;
      break;
    case 2:
      result = VK_IMAGE_TYPE_2D;
      break;
    case 3:
      result = VK_IMAGE_TYPE_3D;
      break;
    default:
      throw std::runtime_error("No image type found.");
      break;
  }

  return result;
}
}  // namespace

namespace core {

Image::Image(Device const& device, VkImage handle, const VkExtent3D& extent,
             VkFormat format, VkImageUsageFlags image_usage,
             VkSampleCountFlagBits sample_count)
    : VulkanResource{handle, &device},
      type{find_image_type(extent)},
      extent{extent},
      format{format},
      sample_count{sample_count},
      usage{image_usage} {
  subresource.mipLevel = 1;
  subresource.arrayLayer = 1;
}

Image::Image(Device const& device, const VkExtent3D& extent, VkFormat format,
             VkImageUsageFlags image_usage, VmaMemoryUsage memory_usage,
             VkSampleCountFlagBits sample_count, const uint32_t mip_levels,
             const uint32_t array_layers, VkImageTiling tiling,
             VkImageCreateFlags flags, uint32_t num_queue_families,
             const uint32_t* queue_families)
    : VulkanResource{VK_NULL_HANDLE, &device},
      type{find_image_type(extent)},
      extent{extent},
      format{format},
      sample_count{sample_count},
      usage{image_usage},
      array_layer_count{array_layers},
      tiling{tiling} {
  assert(mip_levels > 0 && "Image should have at least one level");
  assert(array_layers > 0 && "Image should have at least one layer");

  subresource.mipLevel = mip_levels;
  subresource.arrayLayer = array_layers;

  VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  image_info.flags = flags;
  image_info.imageType = type;
  image_info.format = format;
  image_info.extent = extent;
  image_info.mipLevels = mip_levels;
  image_info.arrayLayers = array_layers;
  image_info.samples = sample_count;
  image_info.tiling = tiling;
  image_info.usage = image_usage;

  if (num_queue_families != 0) {
    image_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
    image_info.queueFamilyIndexCount = num_queue_families;
    image_info.pQueueFamilyIndices = queue_families;
  }

  VmaAllocationCreateInfo memory_info{};
  memory_info.usage = memory_usage;

  if (image_usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
    memory_info.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
  }

  auto result = vmaCreateImage(device.get_memory_allocator(), &image_info,
                               &memory_info, &handle, &memory, nullptr);

  if (result != VK_SUCCESS) {
    throw VulkanException{result, "Cannot create Image"};
  }
}

Image::Image(Image&& other)
    : VulkanResource{std::move(other)},
      memory{other.memory},
      type{other.type},
      extent{other.extent},
      format{other.format},
      sample_count{other.sample_count},
      usage{other.usage},
      tiling{other.tiling},
      subresource{other.subresource},
      views(std::exchange(other.views, {})),
      mapped_data{other.mapped_data},
      mapped{other.mapped} {
  other.memory = VK_NULL_HANDLE;
  other.mapped_data = nullptr;
  other.mapped = false;

  // Update image views references to this image to avoid dangling pointers
  for (auto& view : views) {
    view->set_image(*this);
  }
}

Image::~Image() {
  if (handle != VK_NULL_HANDLE && memory != VK_NULL_HANDLE) {
    unmap();
    vmaDestroyImage(device->get_memory_allocator(), handle, memory);
  }
}

void Image::unmap() {
  if (mapped) {
    vmaUnmapMemory(device->get_memory_allocator(), memory);
    mapped_data = nullptr;
    mapped = false;
  }
}

VkImageType Image::get_type() const { return type; }

const VkExtent3D& Image::get_extent() const { return extent; }

VkFormat Image::get_format() const { return format; }

VkSampleCountFlagBits Image::get_sample_count() const { return sample_count; }

VkImageUsageFlags Image::get_usage() const { return usage; }

VkImageSubresource Image::get_subresource() const { return subresource; }

std::unordered_set<ImageView*>& Image::get_views() { return views; }

}  // namespace core
}  // namespace vkb