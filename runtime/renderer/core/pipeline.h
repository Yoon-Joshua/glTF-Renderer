#pragma once
#include "runtime/renderer/common/helpers.h"
#include "runtime/renderer/common/vk_common.h"
#include "runtime/renderer/rendering/pipeline_state.h"

namespace vkb {

class Device;

class Pipeline {

 public:
  Pipeline(Device &device);
  Pipeline(const Pipeline &) = delete;
  Pipeline(Pipeline &&other);
  virtual ~Pipeline();
  Pipeline &operator=(const Pipeline &) = delete;
  Pipeline &operator=(Pipeline &&) = delete;
  VkPipeline get_handle() const;
  const PipelineState &get_state() const;

 protected:
  Device &device;
  VkPipeline handle = VK_NULL_HANDLE;
  PipelineState state;
};

class ComputePipeline : public Pipeline {
 public:
  ComputePipeline(ComputePipeline &&) = default;
  virtual ~ComputePipeline() = default;
  ComputePipeline(Device &device, VkPipelineCache pipeline_cache,
                  PipelineState &pipeline_state);
};

class GraphicsPipeline : public Pipeline {
 public:
  GraphicsPipeline(GraphicsPipeline &&) = default;
  virtual ~GraphicsPipeline() = default;
  GraphicsPipeline(Device &device, VkPipelineCache pipeline_cache,
                   PipelineState &pipeline_state);
};

}  // namespace vkb