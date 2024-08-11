#pragma once
#include <Volk/volk.h>

#include <string>
#include <vector>
namespace vkb {

/**
 * @brief Helper function to convert a VkFormat enum to a string
 * @param format Vulkan format to convert.
 * @return The string to return.
 */
const std::string to_string(VkFormat format);

/**
 * @brief Helper function to convert a VkPresentModeKHR to a string
 * @param present_mode Vulkan present mode to convert.
 * @return The string to return.
 */
const std::string to_string(VkPresentModeKHR present_mode);

/**
 * @brief Helper function to convert a VkSurfaceFormatKHR format to a string
 * @param surface_format Vulkan surface format to convert.
 * @return The string to return.
 */
const std::string to_string(VkSurfaceFormatKHR surface_format);

/**
 * @brief Helper function to convert a VkCompositeAlphaFlagBitsKHR flag to a string
 * @param composite_alpha Vulkan composite alpha flag bit to convert.
 * @return The string to return.
 */
const std::string to_string(VkCompositeAlphaFlagBitsKHR composite_alpha);

/// @brief Helper function to split a single string into a vector of strings by
/// a delimiter
/// @param input The input string to be split
/// @param delim The character to delimit by
/// @return The vector of tokenized strings
std::vector<std::string> split(const std::string &input, char delim);
}  // namespace vkb