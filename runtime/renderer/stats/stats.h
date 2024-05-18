#pragma once
#include <cstdint>

namespace vkb {

class RenderContext;
class Device;
class CommandBuffer;

/// @brief Helper class for querying statistics about the CPU and the GPU
class Stats {
 public:
  /// @brief Constructs a Stats object
  /// @param render_context The RenderContext for this sample
  /// @param buffer_size Size of the circular buffers
  explicit Stats(RenderContext &render_context, size_t buffer_size = 16);

  /// @brief Destroys the Stats object
  ~Stats();

  /**
   * @brief A command buffer that we want to collect stats about has just begun
   *
   * Some stats providers (like the Vulkan extension one) can only collect stats
   * about the execution of a specific command buffer. In those cases we need to
   * know when a command buffer has begun and when it's about to end so that we
   * can inject some extra commands into the command buffer to control the stats
   * collection. This method tells the stats provider that a command buffer has
   * begun so that can happen. The command buffer must be in a recording state
   * when this method is called.
   * @param cb The command buffer
   */
  void begin_sampling(CommandBuffer &cb);

  /**
   * @brief A command buffer that we want to collect stats about is about to be
   * ended
   *
   * Some stats providers (like the Vulkan extension one) can only collect stats
   * about the execution of a specific command buffer. In those cases we need to
   * know when a command buffer has begun and when it's about to end so that we
   * can inject some extra commands into the command buffer to control the stats
   * collection. This method tells the stats provider that a command buffer is
   * about to be ended so that can happen. The command buffer must be in a
   * recording state when this method is called.
   * @param cb The command buffer
   */
  void end_sampling(CommandBuffer &cb);

 private:
  /// The render context
  RenderContext &render_context;

  /// Size of the circular buffers
  size_t buffer_size;
};
}  // namespace vkb