#include "shader_module.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "config/shader_macro_config.h"
#include "runtime/platform/filesystem.h"
#include "runtime/renderer/common/strings.h"

#include "spdlog/fmt/fmt.h"

using json = nlohmann::json;

extern struct ShaderMacroConfig shader_macro_config;

namespace vkb {

/// @brief Pre-compiles project shader files to include header code
/// @param source The shader file
/// @returns A byte array of the final shader
inline std::vector<std::string> precompile_shader(const std::string &source) {
  std::vector<std::string> final_file;

  auto lines = split(source, '\n');

  for (auto &line : lines) {
    if (line.find("#include \"") == 0) {
      // Include paths are relative to the base shader directory
      std::string include_path = line.substr(10);
      size_t last_quote = include_path.find("\"");
      if (!include_path.empty() && last_quote != std::string::npos) {
        include_path = include_path.substr(0, last_quote);
      }

      auto include_file = precompile_shader(fs::read_shader(include_path));
      for (auto &include_file_line : include_file) {
        final_file.push_back(include_file_line);
      }
    } else {
      final_file.push_back(line);
    }
  }

  return final_file;
}

ShaderVariant::ShaderVariant(std::string &&preamble,
                             std::vector<std::string> &&processes)
    : preamble{std::move(preamble)}, processes{std::move(processes)} {
  update_id();
}

size_t ShaderVariant::get_id() const { return id; }

void ShaderVariant::add_definitions(
    const std::vector<std::string> &definitions) {
  for (auto &definition : definitions) {
    add_define(definition);
  }
}

void ShaderVariant::add_define(const std::string &def) {
  processes.push_back("D" + def);

  std::string tmp_def = def;

  // The "=" needs to turn into a space
  size_t pos_equal = tmp_def.find_first_of("=");
  if (pos_equal != std::string::npos) {
    tmp_def[pos_equal] = ' ';
  }

  preamble.append("#define " + tmp_def + "\n");

  update_id();
}

void ShaderVariant::add_undefine(const std::string &undef) {
  processes.push_back("U" + undef);
  preamble.append("#undef " + undef + "\n");
  update_id();
}

void ShaderVariant::add_runtime_array_size(
    const std::string &runtime_array_name, size_t size) {
  if (runtime_array_sizes.find(runtime_array_name) ==
      runtime_array_sizes.end()) {
    runtime_array_sizes.insert({runtime_array_name, size});
  } else {
    runtime_array_sizes[runtime_array_name] = size;
  }
}

void ShaderVariant::set_runtime_array_sizes(
    const std::unordered_map<std::string, size_t> &sizes) {
  this->runtime_array_sizes = sizes;
}

const std::string &ShaderVariant::get_preamble() const { return preamble; }

const std::vector<std::string> &ShaderVariant::get_processes() const {
  return processes;
}

const std::unordered_map<std::string, size_t> &
ShaderVariant::get_runtime_array_sizes() const {
  return runtime_array_sizes;
}

void ShaderVariant::clear() {
  preamble.clear();
  processes.clear();
  runtime_array_sizes.clear();
  update_id();
}

void ShaderVariant::update_id() {
  std::hash<std::string> hasher{};
  id = hasher(preamble);
}

ShaderSource::ShaderSource(const std::string &filename)
    : filename(filename), source(fs::read_shader(filename)) {
  std::hash<std::string> hasher{};
  id = hasher(std::string{this->source.cbegin(), this->source.cend()});
}

size_t ShaderSource::get_id() const { return id; }

const std::string &ShaderSource::get_filename() const { return filename; }

void ShaderSource::set_source(const std::string &source_) {
  source = source_;
  std::hash<std::string> hasher{};
  id = hasher(std::string{this->source.cbegin(), this->source.cend()});
}

const std::string &ShaderSource::get_source() const { return source; }

ShaderModule::ShaderModule(Device &device, VkShaderStageFlagBits stage,
                           const std::string &path,
                           const std::string &entry_point,
                           const ShaderVariant &shader_variant)
    : device{device}, stage{stage}, entry_point{entry_point} {
  debug_name = "what a fuck?!";
  printf("fuck fuck fuck\n");
  if (entry_point.empty()) {
    throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
  }

  auto readFile = [=](const std::string &path) -> std::vector<uint32_t> {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
      throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t)file.tellg();
    if (fileSize % 4 != 0) {
      throw std::runtime_error("SPIRV 编译错了！！！！");
    }
    std::vector<uint32_t> buffer(fileSize / 4);
    file.seekg(0);
    file.read((char *)buffer.data(), fileSize);
    file.close();
    return buffer;
  };

  std::vector<std::string> processes = shader_variant.get_processes();
  std::string processes_index = shader_macro_config.shitf_processes_to_index(
      fs::path::get(fs::path::Type::Shaders) + path, processes);

  std::string spirv_dir =
      fs::path::get(fs::path::Type::Shaders) + path + ".bin2/";

  std::string file_name = spirv_dir + processes_index + ".spv";
  spirv = readFile(file_name);

  std::hash<std::string> hasher{};
  id = hasher(
      std::string{reinterpret_cast<const char *>(spirv.data()),
                  reinterpret_cast<const char *>(spirv.data() + spirv.size())});

  // 读取资源
  std::string resource_dir =
      fs::path::get(fs::path::Type::Shaders) + path + ".res2/";
  file_name = resource_dir + processes_index + ".json";
  std::ifstream f(file_name);
  json data = json::parse(f);
  auto res = data["resources"];
  for (auto &e : res) {
    ShaderResource temp;
    unsigned stage = e.at("stages");
    switch (stage) {
      case 1:
        temp.stages = VK_SHADER_STAGE_VERTEX_BIT;
        break;
      case 16:
        temp.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
      default:
        LOGE("no this type of shader");
        exit(-1);
    }

    std::string type = e.at("type");
    if (type == "Input") {
      temp.type = ShaderResourceType::Input;
    } else if (type == "Output") {
      temp.type = ShaderResourceType::Output;
    } else if (type == "BufferUniform") {
      temp.type = ShaderResourceType::BufferUniform;
    } else if (type == "ImageSampler") {
      temp.type = ShaderResourceType::ImageSampler;
    } else if (type == "PushConstant") {
      temp.type = ShaderResourceType::PushConstant;
    } else if (type == "InputAttachment") {
      temp.type = ShaderResourceType::InputAttachment;
    } else if (type == "SpecializationConstant") {
      temp.type = ShaderResourceType::SpecializationConstant;
    } else {
      LOGE("Error from shader_module.cpp\n");
      exit(-1);
    }

    std::string mode = e.at("mode");
    if (mode == "Static") {
      temp.mode = ShaderResourceMode::Static;
    } else if (mode == "Dynamic") {
      temp.mode = ShaderResourceMode::Dynamic;
    } else if (mode == "UpdateAfterBind") {
      temp.mode = ShaderResourceMode::UpdateAfterBind;
    } else {
      LOGE("Error from shader_module.cpp\n");
      exit(-1);
    }

    temp.set = e.at("set");
    temp.binding = e.at("binding");
    temp.location = e.at("location");
    temp.input_attachment_index = e.at("input_attachment_index");
    temp.vec_size = e.at("vec_size");
    temp.columns = e.at("columns");
    temp.array_size = e.at("array_size");
    temp.offset = e.at("offset");
    temp.size = e.at("size");
    temp.constant_id = e.at("constant_id");
    temp.qualifiers = e.at("qualifiers");
    temp.name = e.at("name");

    resources.push_back(temp);
  }
}

#define TEMP
ShaderModule::ShaderModule(Device &device, VkShaderStageFlagBits stage,
                           const ShaderSource &glsl_source,
                           const std::string &entry_point,
                           const ShaderVariant &shader_variant)
    : device{device}, stage{stage}, entry_point{entry_point} {
  debug_name = fmt::format("{} [variant {:X}] [entrypoint {}]",
                           glsl_source.get_filename(), shader_variant.get_id(),
                           entry_point);
#ifndef TEMP
  // Compiling from GLSL source requires the entry point
  if (entry_point.empty()) {
    throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
  }

  auto &source = glsl_source.get_source();

  // Check if application is passing in GLSL source code to compile to SPIR-V
  if (source.empty()) {
    throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
  }

  // Precompile source into the final spirv bytecode
  auto glsl_final_source = precompile_shader(source);

  // Compile the GLSL source
  GLSLCompiler glsl_compiler;

  if (!glsl_compiler.compile_to_spirv(
          stage, convert_to_bytes(glsl_final_source), entry_point,
          shader_variant, spirv, info_log)) {
    LOGE("Shader compilation failed for shader \"{}\"",
         glsl_source.get_filename());
    LOGE("{}", info_log);
    throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
  }

  SPIRVReflection spirv_reflection;

  // Reflect all shader resources
  if (!spirv_reflection.reflect_shader_resources(stage, spirv, resources,
                                                 shader_variant)) {
    throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
  }

  // Generate a unique id, determined by source and variant
  std::hash<std::string> hasher{};
  id = hasher(
      std::string{reinterpret_cast<const char *>(spirv.data()),
                  reinterpret_cast<const char *>(spirv.data() + spirv.size())});
#endif
}
#undef TEMP

ShaderModule::ShaderModule(ShaderModule &&other)
    : device{other.device},
      id{other.id},
      stage{other.stage},
      entry_point{other.entry_point},
      debug_name{other.debug_name},
      spirv{other.spirv},
      resources{other.resources},
      info_log{other.info_log} {
  other.stage = {};
}

size_t ShaderModule::get_id() const { return id; }

VkShaderStageFlagBits ShaderModule::get_stage() const { return stage; }

const std::string &ShaderModule::get_entry_point() const { return entry_point; }

const std::vector<ShaderResource> &ShaderModule::get_resources() const {
  return resources;
}

const std::string &ShaderModule::get_info_log() const { return info_log; }

const std::vector<uint32_t> &ShaderModule::get_binary() const { return spirv; }

void ShaderModule::set_resource_mode(const std::string &resource_name,
                                     const ShaderResourceMode &resource_mode) {
  auto it = std::find_if(resources.begin(), resources.end(),
                         [&resource_name](const ShaderResource &resource) {
                           return resource.name == resource_name;
                         });

  if (it != resources.end()) {
    if (resource_mode == ShaderResourceMode::Dynamic) {
      if (it->type == ShaderResourceType::BufferUniform ||
          it->type == ShaderResourceType::BufferStorage) {
        it->mode = resource_mode;
      } else {
        LOGW("Resource `%s` does not support dynamic.\n", resource_name);
      }
    } else {
      it->mode = resource_mode;
    }
  } else {
    LOGW("Resource `%s` not found for shader.\n", resource_name);
  }
}

}  // namespace vkb