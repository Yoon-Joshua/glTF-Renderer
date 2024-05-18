#include "runtime/platform/filesystem.h"

#include <fileapi.h>
#include <sys/stat.h>

#include <algorithm>
#include <cassert>

#include "runtime/platform/platform.h"
namespace fs {

namespace path {
const std::unordered_map<Type, std::string> relative_paths = {
    {Type::Assets, "E:/projects/PanGu/assets/"},
    {Type::Shaders, "E:/projects/PanGu/shaders/"},
    {Type::Storage, "output/"},
    {Type::Screenshots, "output/images/"},
    {Type::Logs, "output/logs/"},
};

const std::string get(const Type type, const std::string &file) {
  assert(relative_paths.size() == Type::TotalRelativePathTypes &&
         "Not all paths are defined in filesystem, please check that each enum "
         "is specified");

  // Check for special cases first
  if (type == Type::WorkingDir) {
    return Platform::get_external_storage_directory();
  } else if (type == Type::Temp) {
    return Platform::get_temp_directory();
  }

  // Check for relative paths
  auto it = relative_paths.find(type);

  if (relative_paths.size() < Type::TotalRelativePathTypes) {
    throw std::runtime_error("Platform hasn't initialized the paths correctly");
  } else if (it == relative_paths.end()) {
    throw std::runtime_error(
        "Path enum doesn't exist, or wasn't specified in the path map");
  } else if (it->second.empty()) {
    throw std::runtime_error("Path was found, but it is empty");
  }

  auto path = Platform::get_external_storage_directory() + it->second;

  if (!is_directory(path)) {
    create_path(Platform::get_external_storage_directory(), it->second);
  }

  return path + file;
}
}  // namespace path

std::string read_text_file(const std::string &filename) {
  std::vector<std::string> data;

  std::ifstream file;

  file.open(filename, std::ios::in);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  return std::string{(std::istreambuf_iterator<char>(file)),
                     (std::istreambuf_iterator<char>())};
}

std::vector<uint8_t> read_binary_file(const std::string &filename,
                                      const uint32_t count) {
  std::vector<uint8_t> data;

  std::ifstream file;

  file.open(filename, std::ios::in | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  uint64_t read_count = count;
  if (count == 0) {
    file.seekg(0, std::ios::end);
    read_count = static_cast<uint64_t>(file.tellg());
    file.seekg(0, std::ios::beg);
  }

  data.resize(static_cast<size_t>(read_count));
  file.read(reinterpret_cast<char *>(data.data()), read_count);
  file.close();

  return data;
}

bool is_directory(const std::string &path) {
  struct stat info;
  if (stat(path.c_str(), &info) != 0) {
    return false;
  } else if (info.st_mode & S_IFDIR) {
    return true;
  } else {
    return false;
  }
}

#define TEMP
void create_directory(const std::string &path) {
#ifndef TEMP
  if (!is_directory(path)) {
    CreateDirectory(path.c_str(), NULL);
  }
#else
  throw std::runtime_error(
      "Currently no solution to create directories. by XR.Y");
#endif
}

void create_path(const std::string &root, const std::string &path) {
  for (auto it = path.begin(); it != path.end(); ++it) {
    it = std::find(it, path.end(), '/');
    create_directory(root + std::string(path.begin(), it));
  }
}

std::vector<uint8_t> read_asset(const std::string &filename,
                                const uint32_t count) {
  return read_binary_file(path::get(path::Type::Assets) + filename, count);
}

std::string read_shader(const std::string &filename) {
  return read_text_file(path::get(path::Type::Shaders) + filename);
}
}  // namespace fs
