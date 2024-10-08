#include "descriptor_set.h"

#include "descriptor_pool.h"
#include "runtime/renderer/common/resource_caching.h"

namespace vkb {
DescriptorSet::DescriptorSet(
    Device &device, const DescriptorSetLayout &descriptor_set_layout,
    DescriptorPool &descriptor_pool,
    const BindingMap<VkDescriptorBufferInfo> &buffer_infos,
    const BindingMap<VkDescriptorImageInfo> &image_infos)
    : device{device},
      descriptor_set_layout{descriptor_set_layout},
      descriptor_pool{descriptor_pool},
      buffer_infos{buffer_infos},
      image_infos{image_infos},
      handle{descriptor_pool.allocate()} {
  prepare();
}

DescriptorSet::DescriptorSet(DescriptorSet &&other)
    : device{other.device},
      descriptor_set_layout{other.descriptor_set_layout},
      descriptor_pool{other.descriptor_pool},
      buffer_infos{std::move(other.buffer_infos)},
      image_infos{std::move(other.image_infos)},
      handle{other.handle},
      write_descriptor_sets{std::move(other.write_descriptor_sets)},
      updated_bindings{std::move(other.updated_bindings)} {
  other.handle = VK_NULL_HANDLE;
}

void DescriptorSet::reset(
    const BindingMap<VkDescriptorBufferInfo> &new_buffer_infos,
    const BindingMap<VkDescriptorImageInfo> &new_image_infos) {
  if (!new_buffer_infos.empty() || !new_image_infos.empty()) {
    buffer_infos = new_buffer_infos;
    image_infos = new_image_infos;
  } else {
    LOGW(
        "Calling reset on Descriptor Set with no new buffer infos and no new "
        "image infos.");
  }
  this->write_descriptor_sets.clear();
  this->updated_bindings.clear();
  prepare();
}

void DescriptorSet::update(const std::vector<uint32_t> &bindings_to_update) {
  std::vector<VkWriteDescriptorSet> write_operations;
  std::vector<size_t> write_operation_hashes;

  // If the 'bindings_to_update' vector is empty, we want to write to all the
  // bindings (but skipping all to-update bindings that haven't been written
  // yet)
  if (bindings_to_update.empty()) {
    for (size_t i = 0; i < write_descriptor_sets.size(); i++) {
      const auto &write_operation = write_descriptor_sets[i];

      size_t write_operation_hash = 0;
      hash_param(write_operation_hash, write_operation);

      auto update_pair_it = updated_bindings.find(write_operation.dstBinding);
      if (update_pair_it == updated_bindings.end() ||
          update_pair_it->second != write_operation_hash) {
        write_operations.push_back(write_operation);
        write_operation_hashes.push_back(write_operation_hash);
      }
    }
  } else {
    // Otherwise we want to update the binding indices present in the
    // 'bindings_to_update' vector. (again, skipping those to update but not
    // updated yet)
    for (size_t i = 0; i < write_descriptor_sets.size(); i++) {
      const auto &write_operation = write_descriptor_sets[i];

      if (std::find(bindings_to_update.begin(), bindings_to_update.end(),
                    write_operation.dstBinding) != bindings_to_update.end()) {
        size_t write_operation_hash = 0;
        hash_param(write_operation_hash, write_operation);

        auto update_pair_it = updated_bindings.find(write_operation.dstBinding);
        if (update_pair_it == updated_bindings.end() ||
            update_pair_it->second != write_operation_hash) {
          write_operations.push_back(write_operation);
          write_operation_hashes.push_back(write_operation_hash);
        }
      }
    }
  }

  // Perform the Vulkan call to update the DescriptorSet by executing the write
  // operations
  if (!write_operations.empty()) {
    vkUpdateDescriptorSets(device.get_handle(), to_u32(write_operations.size()),
                           write_operations.data(), 0, nullptr);
  }

  // Store the bindings from the write operations that were executed by
  // vkUpdateDescriptorSets (and their hash) to prevent overwriting by future
  // calls to "update()"
  for (size_t i = 0; i < write_operations.size(); i++) {
    updated_bindings[write_operations[i].dstBinding] =
        write_operation_hashes[i];
  }
}

void DescriptorSet::apply_writes() const {
  vkUpdateDescriptorSets(device.get_handle(),
                         to_u32(write_descriptor_sets.size()),
                         write_descriptor_sets.data(), 0, nullptr);
}

const DescriptorSetLayout &DescriptorSet::get_layout() const {
  return descriptor_set_layout;
}

VkDescriptorSet DescriptorSet::get_handle() const { return handle; }

BindingMap<VkDescriptorBufferInfo> &DescriptorSet::get_buffer_infos() {
  return buffer_infos;
}

BindingMap<VkDescriptorImageInfo> &DescriptorSet::get_image_infos() {
  return image_infos;
}

void DescriptorSet::prepare() {
  // We don't want to prepare twice during the life cycle of a Descriptor Set
  if (!write_descriptor_sets.empty()) {
    LOGW(
        "Trying to prepare a descriptor set that has already been prepared, "
        "skipping.");
    return;
  }

  // Iterate over all buffer bindings
  for (auto &binding_it : buffer_infos) {
    auto binding_index = binding_it.first;
    auto &buffer_bindings = binding_it.second;

    if (auto binding_info =
            descriptor_set_layout.get_layout_binding(binding_index)) {
      // Iterate over all binding buffers in array
      for (auto &element_it : buffer_bindings) {
        auto &buffer_info = element_it.second;

        size_t uniform_buffer_range_limit =
            device.get_gpu().get_properties().limits.maxUniformBufferRange;
        size_t storage_buffer_range_limit =
            device.get_gpu().get_properties().limits.maxStorageBufferRange;

        size_t buffer_range_limit = static_cast<size_t>(buffer_info.range);

        if ((binding_info->descriptorType ==
                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
             binding_info->descriptorType ==
                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) &&
            buffer_range_limit > uniform_buffer_range_limit) {
          LOGE(
              "Set {} binding {} cannot be updated: buffer size {} exceeds the "
              "uniform buffer range limit {}",
              descriptor_set_layout.get_index(), binding_index,
              buffer_info.range, uniform_buffer_range_limit);
          buffer_range_limit = uniform_buffer_range_limit;
        } else if ((binding_info->descriptorType ==
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
                    binding_info->descriptorType ==
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) &&
                   buffer_range_limit > storage_buffer_range_limit) {
          LOGE(
              "Set {} binding {} cannot be updated: buffer size {} exceeds the "
              "storage buffer range limit {}",
              descriptor_set_layout.get_index(), binding_index,
              buffer_info.range, storage_buffer_range_limit);
          buffer_range_limit = storage_buffer_range_limit;
        }

        // Clip the buffers range to the limit if one exists as otherwise we
        // will receive a Vulkan validation error
        buffer_info.range = buffer_range_limit;

        VkWriteDescriptorSet write_descriptor_set{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

        write_descriptor_set.dstBinding = binding_index;
        write_descriptor_set.descriptorType = binding_info->descriptorType;
        write_descriptor_set.pBufferInfo = &buffer_info;
        write_descriptor_set.dstSet = handle;
        write_descriptor_set.dstArrayElement = element_it.first;
        write_descriptor_set.descriptorCount = 1;

        write_descriptor_sets.push_back(write_descriptor_set);
      }
    } else {
      LOGE("Shader layout set does not use buffer binding at #{}",
           binding_index);
    }
  }

  // Iterate over all image bindings
  for (auto &binding_it : image_infos) {
    auto binding_index = binding_it.first;
    auto &binding_resources = binding_it.second;

    if (auto binding_info =
            descriptor_set_layout.get_layout_binding(binding_index)) {
      // Iterate over all binding images in array
      for (auto &element_it : binding_resources) {
        auto &image_info = element_it.second;

        VkWriteDescriptorSet write_descriptor_set{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

        write_descriptor_set.dstBinding = binding_index;
        write_descriptor_set.descriptorType = binding_info->descriptorType;
        write_descriptor_set.pImageInfo = &image_info;
        write_descriptor_set.dstSet = handle;
        write_descriptor_set.dstArrayElement = element_it.first;
        write_descriptor_set.descriptorCount = 1;

        write_descriptor_sets.push_back(write_descriptor_set);
      }
    } else {
      LOGE("Shader layout set does not use image binding at #{}",
           binding_index);
    }
  }
}

}  // namespace vkb