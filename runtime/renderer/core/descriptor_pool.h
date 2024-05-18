#pragma once

#include "runtime/renderer/core/device.h"

namespace vkb {

class DescriptorSetLayout;

/// @brief Manages an array of fixed size VkDescriptorPool and is able to
/// allocate descriptor sets
class DescriptorPool {
    public:
	static const uint32_t MAX_SETS_PER_POOL = 16;

	DescriptorPool(Device &                   device,
	               const DescriptorSetLayout &descriptor_set_layout,
	               uint32_t                   pool_size = MAX_SETS_PER_POOL);

	DescriptorPool(const DescriptorPool &) = delete;

	DescriptorPool(DescriptorPool &&) = default;

	~DescriptorPool();

	DescriptorPool &operator=(const DescriptorPool &) = delete;

	DescriptorPool &operator=(DescriptorPool &&) = delete;

	void reset();

	const DescriptorSetLayout &get_descriptor_set_layout() const;

	void set_descriptor_set_layout(const DescriptorSetLayout &set_layout);

	VkDescriptorSet allocate();

	VkResult free(VkDescriptorSet descriptor_set);

  private:
	Device &device;

	const DescriptorSetLayout *descriptor_set_layout{nullptr};

	// Descriptor pool size
	std::vector<VkDescriptorPoolSize> pool_sizes;

	// Number of sets to allocate for each pool
	uint32_t pool_max_sets{0};

	// Total descriptor pools created
	std::vector<VkDescriptorPool> pools;

	// Count sets for each pool
	std::vector<uint32_t> pool_sets_count;

	// Current pool index to allocate descriptor set
	uint32_t pool_index{0};

	// Map between descriptor set and pool index
	std::unordered_map<VkDescriptorSet, uint32_t> set_pool_mapping;

	// Find next pool index or create new pool
	uint32_t find_available_pool(uint32_t pool_index);
};

}  // namespace vkb