#pragma once
#include "runtime/renderer/core/debug.h"
#include "runtime/renderer/core/instance.h"
#include "runtime/renderer/core/physical_device.h"
#include "runtime/renderer/debug_info.h"

#define VKB_VULKAN_DEBUG

class Infrastructure {
  /******************************* Application *******************************/
 public:
  const std::string &get_name() const;

 private:
  std::string name{};

  /****************************** Vulkan Sample ******************************/
 public:
  bool prepare();

  /// @brief Get additional sample-specific instance layers.
  /// @return Vector of additional instance layers. Default is empty vector.
  virtual const std::vector<const char *> get_validation_layers();

  /// @brief Get sample-specific instance extensions.
  /// @return Map of instance extensions and whether or not they are optional.
  /// Default is empty map.
  const std::unordered_map<const char *, bool> get_instance_extensions();

  /// @brief Add a sample-specific instance extension
  /// @param extension The extension name
  /// @param optional (Optional) Whether the extension is optional
  void add_instance_extension(const char *extension, bool optional = false);

 private:
  std::unique_ptr<vkb::Instance> instance{nullptr};

  /** @brief Set of instance extensions to be enabled for this example and
   * whether they are optional (must be set in the derived constructor) */
  std::unordered_map<const char *, bool> instance_extensions;

  /** @brief The Vulkan API version to request for this sample at instance
   * creation time */
  uint32_t api_version = VK_API_VERSION_1_2;

  /********************************* XR.Y *********************************/

 private:
  bool headless = false;

 public:
  inline bool is_headless() { return headless; }
  inline vkb::Instance &get_instance() { return *instance; }
};