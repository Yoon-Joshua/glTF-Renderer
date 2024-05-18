#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VK_NO_PROTOTYPES
#include <vma/vk_mem_alloc.h>

#include <unordered_set>

#include "runtime/renderer/core/vulkan_resource.h"

namespace vkb {

class Device;

namespace core {

class ImageView;

class Image
    : public VulkanResource<VkImage, VK_OBJECT_TYPE_IMAGE, const Device> {
 public:
  Image(Device const& device, VkImage handle, const VkExtent3D& extent,
        VkFormat format, VkImageUsageFlags image_usage,
        VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT);

  Image(Device const& device, const VkExtent3D& extent, VkFormat format,
        VkImageUsageFlags image_usage, VmaMemoryUsage memory_usage,
        VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
        uint32_t mip_levels = 1, uint32_t array_layers = 1,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        VkImageCreateFlags flags = 0, uint32_t num_queue_families = 0,
        const uint32_t* queue_families = nullptr);
  Image(const Image&) = delete;
  Image(Image&& other);
  ~Image() override;

  /// @brief Unmaps vulkan memory from the host visible address
  void unmap();

  VkImageType get_type() const;
  const VkExtent3D& get_extent() const;
  VkFormat get_format() const;
  VkSampleCountFlagBits get_sample_count() const;
  VkImageUsageFlags get_usage() const;
  VkImageSubresource get_subresource() const;
  std::unordered_set<ImageView*>& get_views();

 private:
  VmaAllocation memory{VK_NULL_HANDLE};

  VkImageType type{};

  VkExtent3D extent{};

  VkFormat format{};

  VkImageUsageFlags usage{};

  VkSampleCountFlagBits sample_count{};

  VkImageTiling tiling{};

  VkImageSubresource subresource{};

  uint32_t array_layer_count{0};

  /// Image views referring to this image
  std::unordered_set<ImageView*> views;

  uint8_t* mapped_data{nullptr};

  /// Whether it was mapped with vmaMapMemory
  bool mapped{false};
};

}  // namespace core
}  // namespace vkb