#pragma once
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace fs {

namespace path {
enum Type {
  // Relative paths
  Assets,
  Shaders,
  Storage,
  Screenshots,
  Logs,
  /* NewFolder */
  TotalRelativePathTypes,

  // Special paths
  ExternalStorage,
  WorkingDir = ExternalStorage,
  Temp
};

extern const std::unordered_map<Type, std::string> relative_paths;

/// @brief Gets the absolute path of a given type or a specific file
/// @param type The type of file path
/// @param file (Optional) The filename
/// @throws runtime_error if the platform didn't initialize each path properly,
/// path wasn't found or the path was found but is empty
/// @return Path to the directory of a certain type
const std::string get(const Type type, const std::string &file = "");

}  // namespace path

/// @brief Helper to tell if a given path is a directory
/// @param path A path to a directory
/// @return True if the path points to a valid directory, false if not
bool is_directory(const std::string &path);

/// @brief Platform specific implementation to create a directory
/// @param path A path to a directory
void create_directory(const std::string &path);

/// @brief Recursively creates a directory
/// @param root The root directory that the path is relative to
/// @param path A path in the format 'this/is/an/example/path/'
void create_path(const std::string &root, const std::string &path);

/// @brief Helper to read an asset file into a byte-array
/// @param filename The path to the file (relative to the assets directory)
/// @param count (optional) How many bytes to read. If 0 or not specified, the size of the file will be used.
/// @return A vector filled with data read from the file
std::vector<uint8_t> read_asset(const std::string &filename, const uint32_t count = 0);

/// @brief Helper to read a shader file into a single string
/// @param filename The path to the file (relative to the assets directory)
/// @return A string of the text in the shader file
std::string read_shader(const std::string &filename);
}  // namespace fs