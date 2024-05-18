#pragma once
#include <vector>
#include <string>
namespace vkb {
/// @brief Helper function to split a single string into a vector of strings by a delimiter
/// @param input The input string to be split
/// @param delim The character to delimit by
/// @return The vector of tokenized strings
std::vector<std::string> split(const std::string &input, char delim);
}  // namespace vkb