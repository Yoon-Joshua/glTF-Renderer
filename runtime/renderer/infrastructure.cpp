#include "runtime/renderer/infrastructure.h"

#include <cstring>

#include "runtime/platform/window.h"
#include "runtime/renderer/common/error.h"

#define VKB_VULKAN_DEBUG

const std::string& Infrastructure::get_name() const { return name; }

bool Infrastructure::prepare() {
  /********************* Vulkan Sample *********************/
  LOGI("Initializing Vulkan sample");

  VkResult result = volkInitialize();
  if (result) {
    throw VulkanException(result, "Failed to initialize volk.");
  }

  //std::unique_ptr<vkb::DebugUtils> debug_utils{};

  // Creating the vulkan instance
  for (const char* extension_name :
       Window::get_required_instance_extensions()) {
    add_instance_extension(extension_name);
  }

#ifdef VKB_VULKAN_DEBUG
  {
    uint32_t instance_extension_count;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(
        nullptr, &instance_extension_count, nullptr));

    std::vector<VkExtensionProperties> available_instance_extensions(
        instance_extension_count);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(
        nullptr, &instance_extension_count,
        available_instance_extensions.data()));

    for (const auto& it : available_instance_extensions) {
      if (strcmp(it.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
        LOGI("Vulkan debug utils enabled ({})",
             VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        //debug_utils = std::make_unique<vkb::DebugUtilsExtDebugUtils>();
        add_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        break;
      }
    }
  }
#endif

  if (!instance) {
    instance = std::make_unique<vkb::Instance>(
        get_name(), get_instance_extensions(), get_validation_layers(),
        headless, api_version);
  }
  return true;
}

const std::vector<const char*> Infrastructure::get_validation_layers() {
  return {};
}

const std::unordered_map<const char*, bool>
Infrastructure::get_instance_extensions() {
  return instance_extensions;
}

void Infrastructure::add_instance_extension(const char* extension,
                                             bool optional) {
  instance_extensions[extension] = optional;
}
