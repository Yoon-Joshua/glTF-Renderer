#include "runtime/renderer/common/error.h"

VulkanException::VulkanException(const VkResult result, const std::string &msg)
    : result{result}, std::runtime_error{msg} {
  error_message = std::string(std::runtime_error::what()) + std::string{" : "} /* +
                  to_string(result) */;
}

const char *VulkanException::what() const noexcept {
  return error_message.c_str();
}