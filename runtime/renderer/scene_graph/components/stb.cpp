#include "stb.h"

#include <stb_image.h>

#include "runtime/renderer/common/helpers.h"
namespace sg {
Stb::Stb(const std::string &name, const std::vector<uint8_t> &data,
         ContentType content_type)
    : Image{name} {
  int width;
  int height;
  int comp;
  int req_comp = 4;

  auto data_buffer = reinterpret_cast<const stbi_uc *>(data.data());
  auto data_size = static_cast<int>(data.size());

  auto raw_data = stbi_load_from_memory(data_buffer, data_size, &width, &height,
                                        &comp, req_comp);

  if (!raw_data) {
    throw std::runtime_error{"Failed to load " + name + ": " +
                             stbi_failure_reason()};
  }

  set_data(raw_data, width * height * req_comp);
  stbi_image_free(raw_data);

  set_format(content_type == Color ? VK_FORMAT_R8G8B8A8_SRGB
                                   : VK_FORMAT_R8G8B8A8_UNORM);
  set_width(vkb::to_u32(width));
  set_height(vkb::to_u32(height));
  set_depth(1u);
}

}  // namespace sg