#pragma once

#define VKB_DEBUG
#define VKB_VALIDATION_LAYERS
#include <Volk/volk.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace vkb {
	class PhysicalDevice;

	/// @brief Returns a list of Khronos/LunarG supported validation layers
	///       Attempting to enable them in order of preference, starting with later Vulkan SDK versions
	/// @param supported_instance_layers A list of validation layers to check against
	std::vector<const char*> get_optimal_validation_layers(const std::vector<VkLayerProperties>& supported_instance_layers);

	class Instance {
	public:
		Instance(const std::string& application_name,
			const std::unordered_map<const char*, bool>& required_extensions,
			const std::vector<const char*>& required_validation_layers,
			bool headless, uint32_t api_version);

		/// @brief Queries the GPUs of a VkInstance that is already created
		/// @param instance A valid VkInstance
		Instance(VkInstance instance);

		~Instance();

		/// @brief Queries the instance for the physical devices on the machine
		void query_gpus();

		/// @brief Tries to find the first available discrete GPU that can render to the given surface
		/// @param surface to test against
		/// @returns A valid physical device
		PhysicalDevice& get_suitable_gpu(VkSurfaceKHR);

		/// @brief Checks if the given extension is enabled in the VkInstance
		/// @param extension An extension to check
		bool is_enabled(const char* extension) const;

		VkInstance get_handle() const;

	private:
		VkInstance handle{ VK_NULL_HANDLE };
		std::vector<const char*> enabled_extensions;

#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
		/// @brief Debug utils messenger callback for VK_EXT_Debug_Utils
		VkDebugUtilsMessengerEXT debug_utils_messenger{ VK_NULL_HANDLE };

		/// @brief The debug report callback
		VkDebugReportCallbackEXT debug_report_callback{ VK_NULL_HANDLE };
#endif

		/// @brief The physical devices found on the machine
		std::vector<std::unique_ptr<PhysicalDevice>> gpus;
	};
}