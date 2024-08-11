// #include "wavefront_loader.h"

// #include <future>
// #include <glm/gtc/type_ptr.hpp>
// #include <thread>
// #include <vector>

// #include "runtime/platform/filesystem.h"
// #include "runtime/renderer/common/error.h"
// #include "runtime/renderer/common/glm_common.h"
// #include "runtime/renderer/common/utils.h"
// #include "runtime/renderer/core/device.h"
// #include "runtime/renderer/core/queue.h"
// #include "runtime/renderer/core/sampler.h"
// #include "runtime/renderer/scene_graph/components/astc.h"
// #include "runtime/renderer/scene_graph/components/image.h"
// #include "runtime/renderer/scene_graph/components/light.h"
// #include "runtime/renderer/scene_graph/components/pbr_material.h"
// #include "runtime/renderer/scene_graph/components/perspective_camera.h"
// #include "runtime/renderer/scene_graph/components/texture.h"
// #include "runtime/renderer/scene_graph/scene.h"

// #define TINYOBJLOADER_IMPLEMENTATION
// #include <tiny_obj_loader.h>

// namespace vkb {

// std::unique_ptr<sg::PBRMaterial> parse_material(tinyobj::material_t mat) {
//   auto material = std::make_unique<sg::PBRMaterial>(mat.name);
//   material->base_color_factor =
//       glm::vec4(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.0f);
//   material->metallic_factor = mat.metallic;
//   material->roughness_factor = mat.roughness;
//   material->emissive =
//       glm::vec3(mat.emission[0], mat.emission[1], mat.emission[2]);
// #pragma message("I don't know how to fill these values. ")
//   LOGW("{}: I don't know how to fill these values. ", __FILE__);
//   material->alpha_mode = sg::AlphaMode::Opaque;
//   material->alpha_cutoff = 0;
//   material->double_sided = false;

//   return material;
// }

// WaveFrontLoader::WaveFrontLoader(Device const &device) : device(device) {}

// std::unique_ptr<sg::Scene> WaveFrontLoader::read_scene_from_file(
//     const std::string &file_name) {
//   size_t pos = file_name.find_last_of('/');
//   std::string mtl_base_dir = file_name.substr(0, pos);

//   tinyobj::attrib_t attrib;
//   std::vector<tinyobj::shape_t> shapes;
//   std::vector<tinyobj::material_t> mats;
//   std::string warn, err;
//   if (!tinyobj::LoadObj(&attrib, &shapes, &mats, &warn, &err, file_name.c_str(),
//                         mtl_base_dir.c_str())) {
//     throw std::runtime_error(warn + err);
//   }

//   auto scene = sg::Scene();
//   scene.set_name("wavefront_scene");

//   // 加载 采样器
//   std::vector<std::unique_ptr<sg::Sampler>> sampler_components(
//       model.samplers.size());
//   for (size_t sampler_index = 0; sampler_index < model.samplers.size();
//        sampler_index++) {
//     auto sampler = parse_sampler(model.samplers[sampler_index]);
//     sampler_components[sampler_index] = std::move(sampler);
//   }

//   scene.set_components(std::move(sampler_components));

//   // 加载图片
//   std::vector<std::unique_ptr<sg::Image>> image_components;

//   auto image_count = to_u32(model.images.size());

//   // Upload images to GPU. We do this in batches of 64MB of data to avoid
//   // needing double the amount of memory (all the images and all the
//   // corresponding buffers). This helps keep memory footprint lower which is
//   // helpful on smaller devices.
//   size_t image_index = 0;
//   while (image_index < image_count) {
//     std::vector<core::Buffer> transient_buffers;

//     auto &command_buffer = device.request_command_buffer();

//     command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 0);

//     size_t batch_size = 0;

//     // Deal with 64MB of image data at a time to keep memory footprint low
//     while (image_index < image_count && batch_size < 64 * 1024 * 1024) {
//       // Wait for this image to complete loading, then stage for upload

//       auto fut = parse_image(model.images[image_index]);
//       LOGI("Loaded gltf image #{} ({})", image_index,
//            model.images[image_index].uri.c_str());
//       image_components.push_back(std::move(fut));


//       auto &image = image_components[image_index];

//       core::Buffer stage_buffer{device, image->get_data().size(),
//                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//                                 VMA_MEMORY_USAGE_CPU_ONLY};

//       batch_size += image->get_data().size();

//       stage_buffer.update(image->get_data());

//       upload_image_to_gpu(command_buffer, stage_buffer, *image);

//       transient_buffers.push_back(std::move(stage_buffer));

//       image_index++;
//     }

//     command_buffer.end();

//     auto &queue = device.get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0);

//     queue.submit(command_buffer, device.request_fence());

//     device.get_fence_pool().wait();
//     device.get_fence_pool().reset();
//     device.get_command_pool().reset_pool();
//     device.wait_idle();

//     // Remove the staging buffers for the batch we just processed
//     transient_buffers.clear();
//   }

//   scene.set_components(std::move(image_components));

//   // 加载material
//   bool has_textures = scene.has_component<sg::Texture>();
//   std::vector<sg::Texture *> textures;
//   if (has_textures) {
//     textures = scene.get_components<sg::Texture>();
//   }
//   for (auto &mat : mats) {
//     auto material = parse_material(mat);

//     // 漫反射贴图
//     if (!mat.diffuse_texname.empty()) {
//       std::string tex_name = mat.diffuse_texname;
//     }

//     scene.add_component(std::move(material));
//   }

//   // 加载mesh
//   auto default_material = std::make_unique<sg::PBRMaterial>("default material");
//   auto materials = scene.get_components<sg::PBRMaterial>();
//   // shape对应mesh，shape中不同材质的部分为submesh
//   for (auto &shape : shapes) {
//     std::unordered_map<int, std::vector<size_t>> mat_to_face;
//     for (size_t i = 0; i < shape.mesh.material_ids.size(); ++i) {
//       if (mat_to_face.count(shape.mesh.material_ids[i]) == 0) {
//         std::vector<size_t> temp;
//         temp.push_back(i);
//         mat_to_face[shape.mesh.material_ids[i]] = temp;
//       } else {
//         mat_to_face[shape.mesh.material_ids[i]].push_back(i);
//       }
//     }

//     auto mesh = std::make_unique<sg::Mesh>(shape.name);

//     // submesh计数
//     size_t i_primitive = 0;

//     // 遍历处理每个submesh
//     for (auto &p : mat_to_face) {
//       std::string submesh_name;
//       if (p.first == -1)
//         submesh_name = "none";
//       else
//         submesh_name = mats[p.first].name;
//       auto submesh = std::make_unique<sg::SubMesh>(std::move(submesh_name));

//       // 固定不变的属性 position normal texcoords
//       std::vector<float> position, normal, texcoord;
//       for (const auto &face_index : p.second) {
//         for (int i = 0; i < 3; ++i) {
//           size_t indices_index = face_index * 3 + i;
//           auto &index = shape.mesh.indices[indices_index];
//           position.push_back(attrib.vertices[3 * index.vertex_index + 0]);
//           position.push_back(attrib.vertices[3 * index.vertex_index + 1]);
//           position.push_back(attrib.vertices[3 * index.vertex_index + 2]);

//           texcoord.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
//           texcoord.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);

//           normal.push_back(attrib.normals[3 * index.normal_index + 0]);
//           normal.push_back(attrib.normals[3 * index.normal_index + 1]);
//           normal.push_back(attrib.normals[3 * index.normal_index + 2]);
//         }
//       }

//       std::vector<uint8_t> vertex_data(position.size() * sizeof(float));
//       memcpy(vertex_data.data(), position.data(), vertex_data.size());
//       {
//         std::string attrib_name = "position";
//         submesh->vertices_count = shape.mesh.indices.size();

//         core::Buffer buffer(device, vertex_data.size(),
//                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//                             VMA_MEMORY_USAGE_AUTO);
//         buffer.update(vertex_data);

//         buffer.set_debug_name(
//             fmt::format("'{}' mesh, primitive #{}: '{}' vertex buffer",
//                         shape.name, i_primitive, attrib_name));
//         submesh->vertex_buffers.insert(
//             std::make_pair(attrib_name, std::move(buffer)));

//         sg::VertexAttribute attrib;
//         attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
//         attrib.stride = 12;

//         submesh->set_attribute(attrib_name, attrib);
//       }

//       vertex_data.resize(normal.size() * sizeof(float));
//       memcpy(vertex_data.data(), normal.data(), vertex_data.size());
//       {
//         std::string attrib_name = "normal";

//         core::Buffer buffer(device, vertex_data.size(),
//                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//                             VMA_MEMORY_USAGE_AUTO);
//         buffer.update(vertex_data);

//         buffer.set_debug_name(
//             fmt::format("'{}' mesh, primitive #{}: '{}' vertex buffer",
//                         shape.name, i_primitive, attrib_name));
//         submesh->vertex_buffers.insert(
//             std::make_pair(attrib_name, std::move(buffer)));

//         sg::VertexAttribute attrib;
//         attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
//         attrib.stride = 12;

//         submesh->set_attribute(attrib_name, attrib);
//       }

//       vertex_data.resize(texcoord.size() * sizeof(float));
//       memcpy(vertex_data.data(), texcoord.data(), vertex_data.size());
//       {
//         std::string attrib_name = "texcoord_0";

//         core::Buffer buffer(device, vertex_data.size(),
//                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//                             VMA_MEMORY_USAGE_AUTO);
//         buffer.update(vertex_data);

//         buffer.set_debug_name(
//             fmt::format("'{}' mesh, primitive #{}: '{}' vertex buffer",
//                         shape.name, i_primitive, attrib_name));
//         submesh->vertex_buffers.insert(
//             std::make_pair(attrib_name, std::move(buffer)));

//         sg::VertexAttribute attrib;
//         attrib.format = VK_FORMAT_R32G32_SFLOAT;
//         attrib.stride = 8;

//         submesh->set_attribute(attrib_name, attrib);
//       }

//       // 顶点索引
//       std::vector<uint32_t> indices(p.second.size() * 3);
//       for (size_t i = 0; i < indices.size(); ++i) {
//         indices[i] = i;
//       }
//       submesh->vertex_indices = indices.size();
//       VkFormat format = VK_FORMAT_R32_UINT;
//       std::vector<uint8_t> index_data(indices.size() * sizeof(uint32_t));
//       memcpy(index_data.data(), indices.data(), index_data.size());
//       submesh->index_type = VK_INDEX_TYPE_UINT32;

//       submesh->index_buffer = std::make_unique<core::Buffer>(
//           device, index_data.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
//           VMA_MEMORY_USAGE_AUTO);
//       submesh->index_buffer->set_debug_name(fmt::format(
//           "'{}' mesh, primitive #{}: index buffer", shape.name, i_primitive));

//       submesh->index_buffer->update(index_data);

//       // 绑定材质
//       if (p.first < 0)
//         submesh->set_material(*default_material);
//       else
//         submesh->set_material(*materials[p.first]);

//       mesh->add_submesh(*submesh);

//       scene.add_component(std::move(submesh));

//       i_primitive++;
//     }
//     scene.add_component(std::move(mesh));
//   }

//   return std::make_unique<sg::Scene>(scene);
// }
// };  // namespace vkb