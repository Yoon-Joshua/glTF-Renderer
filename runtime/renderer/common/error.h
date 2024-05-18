#pragma once
#include <Volk/volk.h>

#include <stdexcept>
#include <string>
#include <cstdio>

#include "runtime/renderer/common/logging.h"

class VulkanException : public std::runtime_error {
 public:
  /// @brief Vulkan exception constructor
  VulkanException(VkResult result, const std::string &msg = "Vulkan error");

  /// @brief Returns the Vulkan error code as string
  /// @return String message of exception
  const char *what() const noexcept override;

  VkResult result;

 private:
  std::string error_message;
};

#define VK_CHECK(x)                      \
  do {                                   \
    VkResult err = x;                    \
    if (err) {                           \
      LOGE("Detected Vulkan error: {}"); \
      abort();                           \
    }                                    \
  } while (0)
