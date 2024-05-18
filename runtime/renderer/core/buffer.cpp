#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VK_NO_PROTOTYPES
#include "runtime/renderer/core/buffer.h"

#include <vma/vk_mem_alloc.h>

#include "runtime/renderer/common/error.h"
#include "runtime/renderer/core/device.h"

namespace vkb {
namespace core {
Buffer::Buffer(Device const& device, VkDeviceSize size,
               VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage,
               VmaAllocationCreateFlags flags,
               const std::vector<uint32_t>& queue_family_indices)
    : VulkanResource{VK_NULL_HANDLE, &device}, size{size} {
  persistent = (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0;

  VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  buffer_info.usage = buffer_usage;
  buffer_info.size = size;
  if (queue_family_indices.size() >= 2) {
    buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
    buffer_info.queueFamilyIndexCount =
        static_cast<uint32_t>(queue_family_indices.size());
    buffer_info.pQueueFamilyIndices = queue_family_indices.data();
  }

  VmaAllocationCreateInfo memory_info{};
  memory_info.flags = flags;
  memory_info.usage = memory_usage;

  VmaAllocationInfo allocation_info{};
  auto result =
      vmaCreateBuffer(device.get_memory_allocator(), &buffer_info, &memory_info,
                      &handle, &allocation, &allocation_info);

  if (result != VK_SUCCESS) {
    throw VulkanException{result, "Cannot create Buffer"};
  }

  memory = allocation_info.deviceMemory;

  if (persistent) {
    mapped_data = static_cast<uint8_t*>(allocation_info.pMappedData);
  }
}

Buffer::Buffer(Buffer&& other)
    : VulkanResource(other.handle, other.device),
      allocation(other.allocation),
      memory(other.memory),
      size(other.size),
      mapped_data(other.mapped_data),
      mapped(other.mapped) {
  // Reset other handles to avoid releasing on destruction
  other.allocation = VK_NULL_HANDLE;
  other.memory = VK_NULL_HANDLE;
  other.mapped_data = nullptr;
  other.mapped = false;
}

Buffer::~Buffer() {
  if (handle != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE) {
    unmap();
    vmaDestroyBuffer(device->get_memory_allocator(), handle, allocation);
  }
}

void Buffer::flush() const {
  vmaFlushAllocation(device->get_memory_allocator(), allocation, 0, size);
}

uint8_t* Buffer::map() {
  if (!mapped && !mapped_data) {
    VK_CHECK(vmaMapMemory(device->get_memory_allocator(), allocation,
                          reinterpret_cast<void**>(&mapped_data)));
    mapped = true;
  }
  return mapped_data;
}

void Buffer::unmap() {
  if (mapped) {
    vmaUnmapMemory(device->get_memory_allocator(), allocation);
    mapped_data = nullptr;
    mapped = false;
  }
}

VkDeviceSize Buffer::get_size() const { return size; }

void Buffer::update(const uint8_t* data, const size_t size,
                    const size_t offset) {
  if (persistent) {
    std::copy(data, data + size, mapped_data + offset);
    flush();
  } else {
    map();
    std::copy(data, data + size, mapped_data + offset);
    flush();
    unmap();
  }
}

void Buffer::update(void const* data, size_t size, size_t offset) {
  update(reinterpret_cast<const uint8_t*>(data), size, offset);
}

void Buffer::update(const std::vector<uint8_t>& data, size_t offset) {
  update(data.data(), data.size(), offset);
}

uint64_t Buffer::get_device_address() {
  VkBufferDeviceAddressInfoKHR buffer_device_address_info{};
  buffer_device_address_info.sType =
      VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
  buffer_device_address_info.buffer = handle;
  return vkGetBufferDeviceAddressKHR(device->get_handle(),
                                     &buffer_device_address_info);
}

}  // namespace core
}  // namespace vkb