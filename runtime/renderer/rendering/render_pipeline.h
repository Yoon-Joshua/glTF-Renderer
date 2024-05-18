#include <Volk/volk.h>

#include <memory>
#include <vector>

#include "runtime/renderer/common/vk_common.h"
#include "runtime/renderer/rendering/subpass.h"

namespace vkb {
/**
 * @brief A RenderPipeline is a sequence of Subpass objects.
 * Subpass holds shaders and can draw the core::sg::Scene.
 * More subpasses can be added to the sequence if required.
 * For example, postprocessing can be implemented with two pipelines which
 * share render targets.
 *
 * GeometrySubpass -> Processes Scene for Shaders, use by itself if shader
 * requires no lighting ForwardSubpass -> Binds lights at the beginning of a
 * GeometrySubpass to create Forward Rendering, should be used with most default
 * shaders LightingSubpass -> Holds a Global Light uniform, Can be combined with
 * GeometrySubpass to create Deferred Rendering
 */
class RenderPipeline {
 public:
  RenderPipeline(std::vector<std::unique_ptr<Subpass>> &&subpasses = {});

  RenderPipeline(const RenderPipeline &) = delete;

  RenderPipeline(RenderPipeline &&) = default;

  virtual ~RenderPipeline() = default;

  RenderPipeline &operator=(const RenderPipeline &) = delete;

  RenderPipeline &operator=(RenderPipeline &&) = default;

  void prepare();

  /// @return Load store info
  const std::vector<LoadStoreInfo> &get_load_store() const;

  /// @param load_store Load store info to set
  void set_load_store(const std::vector<LoadStoreInfo> &load_store);

  /// @return Clear values
  const std::vector<VkClearValue> &get_clear_value() const;

  /// @param clear_values Clear values to set
  void set_clear_value(const std::vector<VkClearValue> &clear_values);

  /// @brief Record draw commands for each Subpass
  void draw(CommandBuffer &command_buffer, RenderTarget &render_target,
            VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

 private:
  /// Default to two load store
  std::vector<std::unique_ptr<Subpass>> subpasses;

  /// Default to two load store
  std::vector<LoadStoreInfo> load_store = std::vector<LoadStoreInfo>(2);

  /// Default to two clear values
  std::vector<VkClearValue> clear_value = std::vector<VkClearValue>(2);

  size_t active_subpass_index{0};
};
}  // namespace vkb