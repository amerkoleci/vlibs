// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(VGPU_VULKAN_DRIVER)

#include "vgpu_driver.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#if defined(_WIN32)
// Use the C++ standard templated min/max
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#elif defined(__APPLE__)
#include <vulkan/vulkan_metal.h>
#include <vulkan/vulkan_beta.h>
#else
typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_window_t;
typedef uint32_t xcb_visualid_t;

#include <vulkan/vulkan_xcb.h>

struct wl_display;
struct wl_surface;
#include <vulkan/vulkan_wayland.h>
#endif

#if !defined(_WIN32)
#include <dlfcn.h>
#endif

//#elif defined(__linux__)
//#define VK_USE_PLATFORM_XCB_KHR
//#endif

VGPU_DISABLE_WARNINGS()

//#include "third_party/volk.h"
#define VMA_STATS_STRING_ENABLED 0
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION
#include "third_party/vk_mem_alloc.h"
VGPU_ENABLE_WARNINGS()

// Functions that don't require an instance
#define GPU_FOREACH_ANONYMOUS(X)\
  X(vkEnumerateInstanceVersion)\
  X(vkEnumerateInstanceLayerProperties)\
  X(vkEnumerateInstanceExtensionProperties)\
  X(vkCreateInstance)

// Functions that require an instance but don't require a device
#define GPU_FOREACH_INSTANCE(X)\
  X(vkDestroyInstance)\
  X(vkCreateDebugUtilsMessengerEXT)\
  X(vkDestroyDebugUtilsMessengerEXT)\
  X(vkDestroySurfaceKHR)\
  X(vkEnumeratePhysicalDevices)\
  X(vkGetPhysicalDeviceProperties)\
  X(vkGetPhysicalDeviceProperties2)\
  X(vkGetPhysicalDeviceFeatures2)\
  X(vkGetPhysicalDeviceMemoryProperties2)\
  X(vkGetPhysicalDeviceFormatProperties)\
  X(vkGetPhysicalDeviceQueueFamilyProperties)\
  X(vkGetPhysicalDeviceQueueFamilyProperties2)\
  X(vkGetPhysicalDeviceSurfaceSupportKHR)\
  X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)\
  X(vkGetPhysicalDeviceSurfaceFormatsKHR)\
  X(vkGetPhysicalDeviceSurfacePresentModesKHR)\
  X(vkEnumerateDeviceExtensionProperties)\
  X(vkGetPhysicalDeviceImageFormatProperties2)\
  X(vkCreateDevice)\
  X(vkDestroyDevice)\
  X(vkGetDeviceProcAddr) 

#if defined(_WIN32)
#define GPU_FOREACH_INSTANCE_PLATFORM(X)\
  X(vkGetPhysicalDeviceWin32PresentationSupportKHR)\
  X(vkCreateWin32SurfaceKHR)\
  X(vkGetMemoryWin32HandleKHR)
#elif defined(__ANDROID__)
#define GPU_FOREACH_INSTANCE_PLATFORM(X)\
  X(vkCreateAndroidSurfaceKHR)
#elif defined(__APPLE__)
#define GPU_FOREACH_INSTANCE_PLATFORM(X)\
  X(vkCreateMetalSurfaceEXT)
#else
#define GPU_FOREACH_INSTANCE_PLATFORM(X)\
  X(vkCreateXlibSurfaceKHR)\
  X(vkCreateXcbSurfaceKHR)\
  X(vkCreateWaylandSurfaceKHR)
#endif

// Functions that require a device
#define GPU_FOREACH_DEVICE(X)\
  X(vkGetDeviceQueue)\
  X(vkSetDebugUtilsObjectNameEXT)\
  X(vkDeviceWaitIdle)\
  X(vkQueueSubmit)\
  X(vkQueuePresentKHR)\
  X(vkCreateSwapchainKHR)\
  X(vkDestroySwapchainKHR)\
  X(vkGetSwapchainImagesKHR)\
  X(vkAcquireNextImageKHR)\
  X(vkCreateCommandPool)\
  X(vkDestroyCommandPool)\
  X(vkResetCommandPool)\
  X(vkAllocateCommandBuffers)\
  X(vkBeginCommandBuffer)\
  X(vkEndCommandBuffer)\
  X(vkCreateFence)\
  X(vkDestroyFence)\
  X(vkResetFences)\
  X(vkGetFenceStatus)\
  X(vkWaitForFences)\
  X(vkCreateSemaphore)\
  X(vkDestroySemaphore)\
  X(vkCmdPipelineBarrier)\
  X(vkCreateQueryPool)\
  X(vkDestroyQueryPool)\
  X(vkCmdResetQueryPool)\
  X(vkCmdBeginQuery)\
  X(vkCmdEndQuery)\
  X(vkCmdWriteTimestamp)\
  X(vkCmdCopyQueryPoolResults)\
  X(vkGetQueryPoolResults)\
  X(vkCreateBuffer)\
  X(vkDestroyBuffer)\
  X(vkGetBufferMemoryRequirements)\
  X(vkBindBufferMemory)\
  X(vkCreateImage)\
  X(vkDestroyImage)\
  X(vkGetImageMemoryRequirements)\
  X(vkBindImageMemory)\
  X(vkCmdCopyBuffer)\
  X(vkCmdCopyImage)\
  X(vkCmdBlitImage)\
  X(vkCmdCopyBufferToImage)\
  X(vkCmdCopyImageToBuffer)\
  X(vkCmdFillBuffer)\
  X(vkCmdClearColorImage)\
  X(vkCmdClearDepthStencilImage)\
  X(vkAllocateMemory)\
  X(vkFreeMemory)\
  X(vkMapMemory)\
  X(vkCreateSampler)\
  X(vkDestroySampler)\
  X(vkCreateRenderPass2)\
  X(vkDestroyRenderPass)\
  X(vkCmdBeginRenderPass2)\
  X(vkCmdEndRenderPass2)\
  X(vkCreateImageView)\
  X(vkDestroyImageView)\
  X(vkCreateFramebuffer)\
  X(vkDestroyFramebuffer)\
  X(vkCreateShaderModule)\
  X(vkDestroyShaderModule)\
  X(vkCreateDescriptorSetLayout)\
  X(vkDestroyDescriptorSetLayout)\
  X(vkCreatePipelineLayout)\
  X(vkDestroyPipelineLayout)\
  X(vkCreateDescriptorPool)\
  X(vkDestroyDescriptorPool)\
  X(vkAllocateDescriptorSets)\
  X(vkResetDescriptorPool)\
  X(vkUpdateDescriptorSets)\
  X(vkCreatePipelineCache)\
  X(vkDestroyPipelineCache)\
  X(vkGetPipelineCacheData)\
  X(vkCreateGraphicsPipelines)\
  X(vkCreateComputePipelines)\
  X(vkDestroyPipeline)\
  X(vkCmdSetViewport)\
  X(vkCmdSetScissor)\
  X(vkCmdSetBlendConstants)\
  X(vkCmdSetStencilReference)\
  X(vkCmdSetDepthBounds)\
  X(vkCmdPushConstants)\
  X(vkCmdBindPipeline)\
  X(vkCmdBindDescriptorSets)\
  X(vkCmdBindVertexBuffers)\
  X(vkCmdBindIndexBuffer)\
  X(vkCmdDraw)\
  X(vkCmdDrawIndexed)\
  X(vkCmdDrawIndirect)\
  X(vkCmdDrawIndexedIndirect)\
  X(vkCmdDispatch)\
  X(vkCmdDispatchIndirect)\
  X(vkCmdBeginDebugUtilsLabelEXT)\
  X(vkCmdEndDebugUtilsLabelEXT)\
  X(vkCmdInsertDebugUtilsLabelEXT)\
  X(vkCmdBeginRendering)\
  X(vkCmdEndRendering)\
  X(vkFreeDescriptorSets) \
  X(vkQueueWaitIdle) \
  X(vkCreateBufferView) \
  X(vkDestroyBufferView) \
  X(vkGetBufferDeviceAddress)

// Functions that require a device and mesh shader extension
#define GPU_FOREACH_DEVICE_MESH_SHADER(X)\
  X(vkCmdDrawMeshTasksEXT)\
  X(vkCmdDrawMeshTasksIndirectEXT)\
  X(vkCmdDrawMeshTasksIndirectCountEXT)

// Used to load/declare Vulkan functions without lots of clutter
#define GPU_LOAD_ANONYMOUS(fn) fn = (PFN_##fn) vkGetInstanceProcAddr(NULL, #fn);
#define GPU_LOAD_INSTANCE(fn) fn = (PFN_##fn) vkGetInstanceProcAddr(instance, #fn);
#define GPU_LOAD_DEVICE(fn) fn = (PFN_##fn) vkGetDeviceProcAddr(device, #fn);
#define GPU_DECLARE(fn) static PFN_##fn fn;

// Declare function pointers
GPU_DECLARE(vkGetInstanceProcAddr)
GPU_FOREACH_ANONYMOUS(GPU_DECLARE)
GPU_FOREACH_INSTANCE(GPU_DECLARE)
GPU_FOREACH_INSTANCE_PLATFORM(GPU_DECLARE)
GPU_DECLARE(vkGetMemoryFdKHR)
GPU_FOREACH_DEVICE(GPU_DECLARE)

// Functions that require a device with 1.3 or VK_KHR_synchronization2
GPU_DECLARE(vkCmdPipelineBarrier2);
GPU_DECLARE(vkCmdWriteTimestamp2);
GPU_DECLARE(vkQueueSubmit2);

GPU_FOREACH_DEVICE_MESH_SHADER(GPU_DECLARE)


#if defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
#include <xcb/xcb.h>
#include <X11/Xlib.h>

typedef xcb_connection_t* (*PFN_XGetXCBConnection)(Display* dpy);
#endif

#include <mutex>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>

// Requires {}
#define APPEND_EXT(desc) \
    *tail = &desc; \
    tail = &desc.pNext

namespace
{
    inline const char* ToString(VkResult result)
    {
        switch (result)
        {
#define STR(r)   \
	case VK_##r: \
		return #r
            STR(NOT_READY);
            STR(TIMEOUT);
            STR(EVENT_SET);
            STR(EVENT_RESET);
            STR(INCOMPLETE);
            STR(ERROR_OUT_OF_HOST_MEMORY);
            STR(ERROR_OUT_OF_DEVICE_MEMORY);
            STR(ERROR_INITIALIZATION_FAILED);
            STR(ERROR_DEVICE_LOST);
            STR(ERROR_MEMORY_MAP_FAILED);
            STR(ERROR_LAYER_NOT_PRESENT);
            STR(ERROR_EXTENSION_NOT_PRESENT);
            STR(ERROR_FEATURE_NOT_PRESENT);
            STR(ERROR_INCOMPATIBLE_DRIVER);
            STR(ERROR_TOO_MANY_OBJECTS);
            STR(ERROR_FORMAT_NOT_SUPPORTED);
            STR(ERROR_SURFACE_LOST_KHR);
            STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
            STR(SUBOPTIMAL_KHR);
            STR(ERROR_OUT_OF_DATE_KHR);
            STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
            STR(ERROR_VALIDATION_FAILED_EXT);
            STR(ERROR_INVALID_SHADER_NV);
#undef STR
            default:
                return "UNKNOWN_ERROR";
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        VGPU_UNUSED(pUserData);

        const char* messageTypeStr = "General";

        if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            messageTypeStr = "Validation";
        else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            messageTypeStr = "Performance";

        // Log debug messge
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            vgpuLogWarn("Vulkan - %s: %s", messageTypeStr, pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            vgpuLogError("Vulkan - %s: %s", messageTypeStr, pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    inline bool ValidateLayers(const std::vector<const char*>& required, const std::vector<VkLayerProperties>& available)
    {
        for (auto layer : required)
        {
            bool found = false;
            for (auto& available_layer : available)
            {
                if (strcmp(available_layer.layerName, layer) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                vgpuLogWarn("Validation Layer '%s' not found", layer);
                return false;
            }
        }

        return true;
    }

    inline std::vector<const char*> GetOptimalValidationLayers(const std::vector<VkLayerProperties>& supported_instance_layers)
    {
        std::vector<std::vector<const char*>> validation_layer_priority_list =
        {
            // The preferred validation layer is "VK_LAYER_KHRONOS_validation"
            {"VK_LAYER_KHRONOS_validation"},

            // Otherwise we fallback to using the LunarG meta layer
            {"VK_LAYER_LUNARG_standard_validation"},

            // Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
            {
                "VK_LAYER_GOOGLE_threading",
                "VK_LAYER_LUNARG_parameter_validation",
                "VK_LAYER_LUNARG_object_tracker",
                "VK_LAYER_LUNARG_core_validation",
                "VK_LAYER_GOOGLE_unique_objects",
            },

            // Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
            {"VK_LAYER_LUNARG_core_validation"}
        };

        for (auto& validation_layers : validation_layer_priority_list)
        {
            if (ValidateLayers(validation_layers, supported_instance_layers))
            {
                return validation_layers;
            }

            vgpuLogWarn("Couldn't enable validation layers (see log for error) - falling back");
        }

        // Else return nothing
        return {};
    }

    inline VkBool32 vulkan_queryPresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
    {
        VGPU_UNUSED(physicalDevice);
        VGPU_UNUSED(queueFamilyIndex);

#if defined(__ANDROID__)
        // All Android queues surfaces support present.
        return true;
#elif defined(_WIN32)
        return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        // TODO: vkGetPhysicalDeviceXcbPresentationSupportKHR
        return true;
#else
        return true;
#endif
    }

    struct PhysicalDeviceVideoExtensions final
    {
        bool queue;
        bool decode_queue;
        bool decode_h264;
        bool decode_h265;
        bool encode_queue;
        bool encode_h264;
        bool encode_h265;
    };

    struct PhysicalDeviceExtensions final
    {
        // Core in 1.3
        bool maintenance4;
        bool dynamicRendering;
        bool synchronization2;
        bool extendedDynamicState;
        bool extendedDynamicState2;
        bool pipelineCreationCacheControl;
        bool formatFeatureFlags2;

        bool swapchain;
        bool depthClipEnable;
        bool conservativeRasterization;
        bool memoryBudget;
        bool AMD_device_coherent_memory;
        bool memory_priority;
        bool performanceQuery;
        bool hostQueryReset;
        bool deferred_host_operations;
        bool textureCompressionAstcHdr;
        bool accelerationStructure;
        bool raytracingPipeline;
        bool rayQuery;
        bool fragment_shading_rate;
        bool meshShader;
        bool conditionalRendering;

        bool externalMemory;
        bool externalSemaphore;
        bool externalFence;

        bool maintenance5;

        bool win32_full_screen_exclusive;
        PhysicalDeviceVideoExtensions video{};
    };

    inline PhysicalDeviceExtensions QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice)
    {
        uint32_t count = 0;
        VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
        if (result != VK_SUCCESS)
            return {};

        std::vector<VkExtensionProperties> vk_extensions(count);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, vk_extensions.data());

        PhysicalDeviceExtensions extensions{};

        for (uint32_t i = 0; i < count; ++i)
        {
            // Core 1.3
            if (strcmp(vk_extensions[i].extensionName, VK_KHR_MAINTENANCE_4_EXTENSION_NAME) == 0)
            {
                extensions.maintenance4 = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
            {
                extensions.dynamicRendering = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0)
            {
                extensions.synchronization2 = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0)
            {
                extensions.extendedDynamicState = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME) == 0)
            {
                extensions.extendedDynamicState2 = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME) == 0)
            {
                extensions.pipelineCreationCacheControl = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME) == 0)
            {
                extensions.formatFeatureFlags2 = true;
            }

            if (strcmp(vk_extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            {
                extensions.swapchain = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME) == 0)
            {
                extensions.depthClipEnable = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME) == 0)
            {
                extensions.conservativeRasterization = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0)
            {
                extensions.memoryBudget = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME) == 0)
            {
                extensions.AMD_device_coherent_memory = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) == 0)
            {
                extensions.memory_priority = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME) == 0)
            {
                extensions.performanceQuery = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME) == 0)
            {
                extensions.hostQueryReset = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0)
            {
                extensions.deferred_host_operations = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME) == 0)
            {
                extensions.textureCompressionAstcHdr = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0)
            {
                extensions.accelerationStructure = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0)
            {
                extensions.raytracingPipeline = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0)
            {
                extensions.rayQuery = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME) == 0)
            {
                extensions.fragment_shading_rate = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0)
            {
                extensions.meshShader = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME) == 0)
            {
                extensions.conditionalRendering = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_QUEUE_EXTENSION_NAME) == 0)
            {
                extensions.video.queue = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME) == 0)
            {
                extensions.video.decode_queue = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME) == 0)
            {
                extensions.video.decode_h264 = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME) == 0)
            {
                extensions.video.decode_h265 = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME) == 0)
            {
                extensions.externalMemory = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME) == 0)
            {
                extensions.externalSemaphore = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME) == 0)
            {
                extensions.externalFence = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_MAINTENANCE_5_EXTENSION_NAME) == 0)
            {
                extensions.maintenance5 = true;
            }

#if defined(_WIN32)
            if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME) == 0)
            {
                extensions.externalMemory = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME) == 0)
            {
                extensions.externalSemaphore = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME) == 0)
            {
                extensions.externalFence = true;
            }
            else if (strcmp(vk_extensions[i].extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0)
            {
                extensions.win32_full_screen_exclusive = true;
            }
#endif
        }

        VkPhysicalDeviceProperties gpuProps;
        vkGetPhysicalDeviceProperties(physicalDevice, &gpuProps);

        // Core 1.3
        if (gpuProps.apiVersion >= VK_API_VERSION_1_3)
        {
            extensions.maintenance4 = true;
            extensions.dynamicRendering = true;
            extensions.synchronization2 = true;
            extensions.extendedDynamicState = true;
            extensions.extendedDynamicState2 = true;
            extensions.pipelineCreationCacheControl = true;
            extensions.formatFeatureFlags2 = true;
        }

        return extensions;
    }

    inline bool IsDepthStencilFormatSupported(VkPhysicalDevice physicalDevice, VkFormat format)
    {
        VGPU_ASSERT(
            format == VK_FORMAT_D16_UNORM_S8_UINT ||
            format == VK_FORMAT_D24_UNORM_S8_UINT ||
            format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            format == VK_FORMAT_S8_UINT
        );
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
        return properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    constexpr VkFormat ToVkFormat(VGPUTextureFormat format)
    {
        switch (format)
        {
            // 8-bit formats
            case VGPUTextureFormat_R8Unorm:         return VK_FORMAT_R8_UNORM;
            case VGPUTextureFormat_R8Snorm:         return VK_FORMAT_R8_SNORM;
            case VGPUTextureFormat_R8Uint:          return VK_FORMAT_R8_UINT;
            case VGPUTextureFormat_R8Sint:          return VK_FORMAT_R8_SINT;
                // 16-bit formats
            case VGPUTextureFormat_R16Unorm:        return VK_FORMAT_R16_UNORM;
            case VGPUTextureFormat_R16Snorm:        return VK_FORMAT_R16_SNORM;
            case VGPUTextureFormat_R16Uint:         return VK_FORMAT_R16_UINT;
            case VGPUTextureFormat_R16Sint:         return VK_FORMAT_R16_SINT;
            case VGPUTextureFormat_R16Float:        return VK_FORMAT_R16_SFLOAT;
            case VGPUTextureFormat_RG8Unorm:        return VK_FORMAT_R8G8_UNORM;
            case VGPUTextureFormat_RG8Snorm:        return VK_FORMAT_R8G8_SNORM;
            case VGPUTextureFormat_RG8Uint:         return VK_FORMAT_R8G8_UINT;
            case VGPUTextureFormat_RG8Sint:         return VK_FORMAT_R8G8_SINT;
                // Packed 16-Bit Pixel Formats
            case VGPUTextureFormat_BGRA4Unorm:       return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
            case VGPUTextureFormat_B5G6R5Unorm:      return VK_FORMAT_B5G6R5_UNORM_PACK16;
            case VGPUTextureFormat_B5G5R5A1Unorm:    return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
                // 32-bit formats
            case VGPUTextureFormat_R32Uint:          return VK_FORMAT_R32_UINT;
            case VGPUTextureFormat_R32Sint:          return VK_FORMAT_R32_SINT;
            case VGPUTextureFormat_R32Float:         return VK_FORMAT_R32_SFLOAT;
            case VGPUTextureFormat_RG16Unorm:        return VK_FORMAT_R16G16_UNORM;
            case VGPUTextureFormat_RG16Snorm:        return VK_FORMAT_R16G16_SNORM;
            case VGPUTextureFormat_RG16Uint:         return VK_FORMAT_R16G16_UINT;
            case VGPUTextureFormat_RG16Sint:         return VK_FORMAT_R16G16_SINT;
            case VGPUTextureFormat_RG16Float:        return VK_FORMAT_R16G16_SFLOAT;
            case VGPUTextureFormat_RGBA8Uint:        return VK_FORMAT_R8G8B8A8_UINT;
            case VGPUTextureFormat_RGBA8Sint:        return VK_FORMAT_R8G8B8A8_SINT;
            case VGPUTextureFormat_BGRA8Unorm:       return VK_FORMAT_B8G8R8A8_UNORM;
            case VGPUTextureFormat_RGBA8Unorm:       return VK_FORMAT_R8G8B8A8_UNORM;
            case VGPUTextureFormat_RGBA8UnormSrgb:   return VK_FORMAT_R8G8B8A8_SRGB;
            case VGPUTextureFormat_RGBA8Snorm:       return VK_FORMAT_R8G8B8A8_SNORM;
            case VGPUTextureFormat_BGRA8UnormSrgb:   return VK_FORMAT_B8G8R8A8_SRGB;
                // Packed 32-Bit formats
            case VGPUTextureFormat_RGB9E5Ufloat:        return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            case VGPUTextureFormat_RGB10A2Unorm:        return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            case VGPUTextureFormat_RGB10A2Uint:         return VK_FORMAT_A2R10G10B10_UINT_PACK32;
            case VGPUTextureFormat_RG11B10Float:        return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
                // 64-Bit formats
            case VGPUTextureFormat_RG32Uint:         return VK_FORMAT_R32G32_UINT;
            case VGPUTextureFormat_RG32Sint:         return VK_FORMAT_R32G32_SINT;
            case VGPUTextureFormat_RG32Float:        return VK_FORMAT_R32G32_SFLOAT;
            case VGPUTextureFormat_RGBA16Unorm:      return VK_FORMAT_R16G16B16A16_UNORM;
            case VGPUTextureFormat_RGBA16Snorm:      return VK_FORMAT_R16G16B16A16_SNORM;
            case VGPUTextureFormat_RGBA16Uint:       return VK_FORMAT_R16G16B16A16_UINT;
            case VGPUTextureFormat_RGBA16Sint:       return VK_FORMAT_R16G16B16A16_SINT;
            case VGPUTextureFormat_RGBA16Float:      return VK_FORMAT_R16G16B16A16_SFLOAT;
                // 128-Bit formats
            case VGPUTextureFormat_RGBA32Uint:       return VK_FORMAT_R32G32B32A32_UINT;
            case VGPUTextureFormat_RGBA32Sint:       return VK_FORMAT_R32G32B32A32_SINT;
            case VGPUTextureFormat_RGBA32Float:      return VK_FORMAT_R32G32B32A32_SFLOAT;
                // Depth-stencil formats
            case VGPUTextureFormat_Depth16Unorm:            return VK_FORMAT_D16_UNORM;
            case VGPUTextureFormat_Depth32Float:            return VK_FORMAT_D32_SFLOAT;
            case VGPUTextureFormat_Stencil8:                return VK_FORMAT_S8_UINT;
            case VGPUTextureFormat_Depth24UnormStencil8:    return VK_FORMAT_D24_UNORM_S8_UINT;
            case VGPUTextureFormat_Depth32FloatStencil8:    return VK_FORMAT_D32_SFLOAT_S8_UINT;
                // Compressed BC formats
            case VGPUTextureFormat_Bc1RgbaUnorm:                return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            case VGPUTextureFormat_Bc1RgbaUnormSrgb:            return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
            case VGPUTextureFormat_Bc2RgbaUnorm:                return VK_FORMAT_BC2_UNORM_BLOCK;
            case VGPUTextureFormat_Bc2RgbaUnormSrgb:            return VK_FORMAT_BC2_SRGB_BLOCK;
            case VGPUTextureFormat_Bc3RgbaUnorm:                return VK_FORMAT_BC3_UNORM_BLOCK;
            case VGPUTextureFormat_Bc3RgbaUnormSrgb:            return VK_FORMAT_BC3_SRGB_BLOCK;
            case VGPUTextureFormat_Bc4RSnorm:                   return VK_FORMAT_BC4_SNORM_BLOCK;
            case VGPUTextureFormat_Bc4RUnorm:                   return VK_FORMAT_BC4_UNORM_BLOCK;
            case VGPUTextureFormat_Bc5RgSnorm:                  return VK_FORMAT_BC5_SNORM_BLOCK;
            case VGPUTextureFormat_Bc5RgUnorm:                  return VK_FORMAT_BC5_UNORM_BLOCK;
            case VGPUTextureFormat_Bc6hRgbUfloat:               return VK_FORMAT_BC6H_UFLOAT_BLOCK;
            case VGPUTextureFormat_Bc6hRgbSfloat:               return VK_FORMAT_BC6H_SFLOAT_BLOCK;
            case VGPUTextureFormat_Bc7RgbaUnorm:                return VK_FORMAT_BC7_UNORM_BLOCK;
            case VGPUTextureFormat_Bc7RgbaUnormSrgb:            return VK_FORMAT_BC7_SRGB_BLOCK;
                // EAC/ETC compressed formats
            case VGPUTextureFormat_Etc2Rgb8Unorm:               return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            case VGPUTextureFormat_Etc2Rgb8UnormSrgb:           return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
            case VGPUTextureFormat_Etc2Rgb8A1Unorm:             return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
            case VGPUTextureFormat_Etc2Rgb8A1UnormSrgb:         return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
            case VGPUTextureFormat_Etc2Rgba8Unorm:              return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
            case VGPUTextureFormat_Etc2Rgba8UnormSrgb:          return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
            case VGPUTextureFormat_EacR11Unorm:                 return VK_FORMAT_EAC_R11_UNORM_BLOCK;
            case VGPUTextureFormat_EacR11Snorm:                 return VK_FORMAT_EAC_R11_SNORM_BLOCK;
            case VGPUTextureFormat_EacRg11Unorm:                return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
            case VGPUTextureFormat_EacRg11Snorm:                return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
                // ASTC compressed formats
            case VGPUTextureFormat_Astc4x4Unorm:               return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
            case VGPUTextureFormat_Astc4x4UnormSrgb:           return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
            case VGPUTextureFormat_Astc5x4Unorm:               return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
            case VGPUTextureFormat_Astc5x4UnormSrgb:           return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
            case VGPUTextureFormat_Astc5x5Unorm:               return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
            case VGPUTextureFormat_Astc5x5UnormSrgb:           return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
            case VGPUTextureFormat_Astc6x5Unorm:               return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
            case VGPUTextureFormat_Astc6x5UnormSrgb:           return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
            case VGPUTextureFormat_Astc6x6Unorm:               return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
            case VGPUTextureFormat_Astc6x6UnormSrgb:           return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
            case VGPUTextureFormat_Astc8x5Unorm:               return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
            case VGPUTextureFormat_Astc8x5UnormSrgb:           return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
            case VGPUTextureFormat_Astc8x6Unorm:               return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
            case VGPUTextureFormat_Astc8x6UnormSrgb:           return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
            case VGPUTextureFormat_Astc8x8Unorm:               return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
            case VGPUTextureFormat_Astc8x8UnormSrgb:           return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
            case VGPUTextureFormat_Astc10x5Unorm:              return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
            case VGPUTextureFormat_Astc10x5UnormSrgb:          return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
            case VGPUTextureFormat_Astc10x6Unorm:              return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
            case VGPUTextureFormat_Astc10x6UnormSrgb:          return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
            case VGPUTextureFormat_Astc10x8Unorm:              return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
            case VGPUTextureFormat_Astc10x8UnormSrgb:          return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
            case VGPUTextureFormat_Astc10x10Unorm:             return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
            case VGPUTextureFormat_Astc10x10UnormSrgb:         return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
            case VGPUTextureFormat_Astc12x10Unorm:             return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
            case VGPUTextureFormat_Astc12x10UnormSrgb:         return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
            case VGPUTextureFormat_Astc12x12Unorm:             return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
            case VGPUTextureFormat_Astc12x12UnormSrgb:         return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

            default:
                return VK_FORMAT_UNDEFINED;
        }
    }

    constexpr VkAttachmentLoadOp ToVkAttachmentLoadOp(VGPULoadAction op)
    {
        switch (op)
        {
            default:
            case VGPULoadAction_Load:
                return VK_ATTACHMENT_LOAD_OP_LOAD;

            case VGPULoadAction_Clear:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;

            case VGPULoadAction_DontCare:
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
    }

    constexpr VkAttachmentStoreOp ToVkAttachmentStoreOp(VGPUStoreAction op)
    {
        switch (op)
        {
            default:
            case VGPUStoreAction_Store:
                return VK_ATTACHMENT_STORE_OP_STORE;

            case VGPUStoreAction_DontCare:
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
    }

    constexpr VkPrimitiveTopology ToVk(VGPUPrimitiveTopology type)
    {
        switch (type)
        {
            case VGPUPrimitiveTopology_PointList:      return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case VGPUPrimitiveTopology_LineList:       return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case VGPUPrimitiveTopology_LineStrip:      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case VGPUPrimitiveTopology_TriangleList:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case VGPUPrimitiveTopology_TriangleStrip:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case VGPUPrimitiveTopology_PatchList:      return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            default:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    constexpr VkPolygonMode ToVk(VGPUFillMode mode, VkBool32 fillModeNonSolid)
    {
        switch (mode)
        {
            default:
            case VGPUFillMode_Solid:
                return VK_POLYGON_MODE_FILL;

            case VGPUFillMode_Wireframe:
                if (!fillModeNonSolid)
                {
                    vgpuLogWarn("Vulkan: Wireframe fill mode is being used but it's not supported on this device");
                    return VK_POLYGON_MODE_FILL;
                }

                return VK_POLYGON_MODE_LINE;
        }
    }

    constexpr VkCullModeFlags ToVk(VGPUCullMode mode)
    {
        switch (mode)
        {
            default:
            case VGPUCullMode_Back:
                return VK_CULL_MODE_BACK_BIT;
            case VGPUCullMode_None:
                return VK_CULL_MODE_NONE;
            case VGPUCullMode_Front:
                return VK_CULL_MODE_FRONT_BIT;
        }
    }

    constexpr VkFrontFace ToVk(VGPUFrontFace mode)
    {
        switch (mode)
        {
            default:
            case VGPUFrontFace_CounterClockwise:
                return VK_FRONT_FACE_COUNTER_CLOCKWISE;
            case VGPUFrontFace_Clockwise:
                return VK_FRONT_FACE_CLOCKWISE;
        }
    }

    constexpr VkBlendFactor ToVk(VGPUBlendFactor value)
    {
        switch (value)
        {
            case VGPUBlendFactor_Zero:                          return VK_BLEND_FACTOR_ZERO;
            case VGPUBlendFactor_One:                           return VK_BLEND_FACTOR_ONE;
            case VGPUBlendFactor_SourceColor:                   return VK_BLEND_FACTOR_SRC_COLOR;
            case VGPUBlendFactor_OneMinusSourceColor:           return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case VGPUBlendFactor_SourceAlpha:                   return VK_BLEND_FACTOR_SRC_ALPHA;
            case VGPUBlendFactor_OneMinusSourceAlpha:           return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case VGPUBlendFactor_DestinationColor:              return VK_BLEND_FACTOR_DST_COLOR;
            case VGPUBlendFactor_OneMinusDestinationColor:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case VGPUBlendFactor_DestinationAlpha:              return VK_BLEND_FACTOR_DST_ALPHA;
            case VGPUBlendFactor_OneMinusDestinationAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case VGPUBlendFactor_SourceAlphaSaturated:          return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
            case VGPUBlendFactor_BlendColor:                    return VK_BLEND_FACTOR_CONSTANT_COLOR;
            case VGPUBlendFactor_OneMinusBlendColor:            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
            case VGPUBlendFactor_BlendAlpha:                    return VK_BLEND_FACTOR_CONSTANT_ALPHA;
            case VGPUBlendFactor_OneMinusBlendAlpha:            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
            case VGPUBlendFactor_Source1Color:                  return VK_BLEND_FACTOR_SRC1_COLOR;
            case VGPUBlendFactor_OneMinusSource1Color:          return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
            case VGPUBlendFactor_Source1Alpha:                  return VK_BLEND_FACTOR_SRC1_ALPHA;
            case VGPUBlendFactor_OneMinusSource1Alpha:          return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
            default:
                VGPU_UNREACHABLE();
        }
    }

    constexpr VkBlendOp ToVk(VGPUBlendOperation value)
    {
        switch (value)
        {
            case VGPUBlendOperation_Add:              return VK_BLEND_OP_ADD;
            case VGPUBlendOperation_Subtract:         return VK_BLEND_OP_SUBTRACT;
            case VGPUBlendOperation_ReverseSubtract:  return VK_BLEND_OP_REVERSE_SUBTRACT;
            case VGPUBlendOperation_Min:              return VK_BLEND_OP_MIN;
            case VGPUBlendOperation_Max:              return VK_BLEND_OP_MAX;
            default:
                VGPU_UNREACHABLE();
        }
    }

    constexpr VkColorComponentFlags ToVk(VGPUColorWriteMaskFlags writeMask)
    {
        VkColorComponentFlags result = 0;
        if (writeMask & VGPUColorWriteMask_Red) {
            result |= VK_COLOR_COMPONENT_R_BIT;
        }
        if (writeMask & VGPUColorWriteMask_Green) {
            result |= VK_COLOR_COMPONENT_G_BIT;
        }
        if (writeMask & VGPUColorWriteMask_Blue) {
            result |= VK_COLOR_COMPONENT_B_BIT;
        }
        if (writeMask & VGPUColorWriteMask_Alpha) {
            result |= VK_COLOR_COMPONENT_A_BIT;
        }

        return result;
    }

    constexpr VkImageAspectFlags GetImageAspectFlags(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_UNDEFINED:
                return 0;

            case VK_FORMAT_S8_UINT:
                return VK_IMAGE_ASPECT_STENCIL_BIT;

            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;

            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
                return VK_IMAGE_ASPECT_DEPTH_BIT;

            default:
                return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    constexpr VkCompareOp ToVk(VGPUCompareFunction function)
    {
        switch (function)
        {
            case VGPUCompareFunction_Never:        return VK_COMPARE_OP_NEVER;
            case VGPUCompareFunction_Less:         return VK_COMPARE_OP_LESS;
            case VGPUCompareFunction_Equal:        return VK_COMPARE_OP_EQUAL;
            case VGPUCompareFunction_LessEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
            case VGPUCompareFunction_Greater:      return VK_COMPARE_OP_GREATER;
            case VGPUCompareFunction_NotEqual:     return VK_COMPARE_OP_NOT_EQUAL;
            case VGPUCompareFunction_GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case VGPUCompareFunction_Always:       return VK_COMPARE_OP_ALWAYS;
            default:
                return VK_COMPARE_OP_NEVER;
        }
    }

    constexpr VkStencilOp ToVk(VGPUStencilOperation op)
    {
        switch (op)
        {
            case VGPUStencilOperation_Keep:            return VK_STENCIL_OP_KEEP;
            case VGPUStencilOperation_Zero:            return VK_STENCIL_OP_ZERO;
            case VGPUStencilOperation_Replace:         return VK_STENCIL_OP_REPLACE;
            case VGPUStencilOperation_Invert:          return VK_STENCIL_OP_INVERT;
            case VGPUStencilOperation_IncrementClamp:  return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
            case VGPUStencilOperation_DecrementClamp:  return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
            case VGPUStencilOperation_IncrementWrap:   return VK_STENCIL_OP_INCREMENT_AND_WRAP;
            case VGPUStencilOperation_DecrementWrap:   return VK_STENCIL_OP_DECREMENT_AND_WRAP;
            default:
                return VK_STENCIL_OP_KEEP;
        }
    }

    constexpr VkFilter ToVkFilter(VGPUSamplerFilter mode)
    {
        switch (mode)
        {
            case VGPUSamplerFilter_Linear:
                return VK_FILTER_LINEAR;
            default:
                return VK_FILTER_NEAREST;
        }
    }

    constexpr VkSamplerMipmapMode ToVkMipmapMode(VGPUSamplerMipFilter mode)
    {
        switch (mode)
        {
            case VGPUSamplerMipFilter_Linear:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            default:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }
    }

    constexpr VkSamplerAddressMode ToVkSamplerAddressMode(VGPUSamplerAddressMode mode)
    {
        switch (mode)
        {
            case VGPUSamplerAddressMode_Mirror:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

            case VGPUSamplerAddressMode_Clamp:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

            case VGPUSamplerAddressMode_Border:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

            default:
            case VGPUSamplerAddressMode_Wrap:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    constexpr VkBorderColor ToVkBorderColor(VGPUSamplerBorderColor value)
    {
        switch (value)
        {
            case VGPUSamplerBorderColor_OpaqueBlack:
                return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
            case VGPUSamplerBorderColor_OpaqueWhite:
                return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            default:
            case VGPUSamplerBorderColor_TransparentBlack:
                return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        }
    }

    constexpr VkQueryType ToVk(VGPUQueryType type)
    {
        switch (type)
        {
            case VGPUQueryType_Occlusion:
            case VGPUQueryType_BinaryOcclusion:
                return VK_QUERY_TYPE_OCCLUSION;

            case VGPUQueryType_Timestamp:
                return VK_QUERY_TYPE_TIMESTAMP;

            case VGPUQueryType_PipelineStatistics:
                return VK_QUERY_TYPE_PIPELINE_STATISTICS;

            default:
                VGPU_UNREACHABLE();
        }
    }

    constexpr uint32_t GetQueryResultSize(VGPUQueryType type)
    {
        const uint32_t highestBitInPipelineStatsBits = 11;

        switch (type)
        {
            case VGPUQueryType_Occlusion:
            case VGPUQueryType_BinaryOcclusion:
            case VGPUQueryType_Timestamp:
                return sizeof(uint64_t);

            case VGPUQueryType_PipelineStatistics:
                return highestBitInPipelineStatsBits * sizeof(uint64_t);

            default:
                VGPU_UNREACHABLE();
        }
    }


    constexpr VkFormat ToVkFormat(VGPUVertexFormat format)
    {
        switch (format)
        {
            case VGPUVertexFormat_UByte2:              return VK_FORMAT_R8G8_UINT;
            case VGPUVertexFormat_UByte4:              return VK_FORMAT_R8G8B8A8_UINT;
            case VGPUVertexFormat_Byte2:               return VK_FORMAT_R8G8_SINT;
            case VGPUVertexFormat_Byte4:               return VK_FORMAT_R8G8B8A8_SINT;
            case VGPUVertexFormat_UByte2Normalized:    return VK_FORMAT_R8G8_UNORM;
            case VGPUVertexFormat_UByte4Normalized:    return VK_FORMAT_R8G8B8A8_UNORM;
            case VGPUVertexFormat_Byte2Normalized:     return VK_FORMAT_R8G8_SNORM;
            case VGPUVertexFormat_Byte4Normalized:     return VK_FORMAT_R8G8B8A8_SNORM;

            case VGPUVertexFormat_UShort2:             return VK_FORMAT_R16G16_UINT;
            case VGPUVertexFormat_UShort4:             return VK_FORMAT_R16G16B16A16_UINT;
            case VGPUVertexFormat_Short2:              return VK_FORMAT_R16G16_SINT;
            case VGPUVertexFormat_Short4:              return VK_FORMAT_R16G16B16A16_SINT;
            case VGPUVertexFormat_UShort2Normalized:   return VK_FORMAT_R16G16_UNORM;
            case VGPUVertexFormat_UShort4Normalized:   return VK_FORMAT_R16G16B16A16_UNORM;
            case VGPUVertexFormat_Short2Normalized:    return VK_FORMAT_R16G16_SNORM;
            case VGPUVertexFormat_Short4Normalized:    return VK_FORMAT_R16G16B16A16_SNORM;
            case VGPUVertexFormat_Half2:               return VK_FORMAT_R16G16_SFLOAT;
            case VGPUVertexFormat_Half4:               return VK_FORMAT_R16G16B16A16_SFLOAT;

            case VGPUVertexFormat_Float:               return VK_FORMAT_R32_SFLOAT;
            case VGPUVertexFormat_Float2:              return VK_FORMAT_R32G32_SFLOAT;
            case VGPUVertexFormat_Float3:              return VK_FORMAT_R32G32B32_SFLOAT;
            case VGPUVertexFormat_Float4:              return VK_FORMAT_R32G32B32A32_SFLOAT;

            case VGPUVertexFormat_UInt:                return VK_FORMAT_R32_UINT;
            case VGPUVertexFormat_UInt2:               return VK_FORMAT_R32G32_UINT;
            case VGPUVertexFormat_UInt3:               return VK_FORMAT_R32G32B32_UINT;
            case VGPUVertexFormat_UInt4:               return VK_FORMAT_R32G32B32A32_UINT;

            case VGPUVertexFormat_Int:                 return VK_FORMAT_R32_SINT;
            case VGPUVertexFormat_Int2:                return VK_FORMAT_R32G32_SINT;
            case VGPUVertexFormat_Int3:                return VK_FORMAT_R32G32B32_SINT;
            case VGPUVertexFormat_Int4:                return VK_FORMAT_R32G32B32A32_SINT;

            case VGPUVertexFormat_Int1010102Normalized:    return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
            case VGPUVertexFormat_UInt1010102Normalized:   return VK_FORMAT_A2B10G10R10_UNORM_PACK32;

            default:
                VGPU_UNREACHABLE();
        }
    }

    constexpr VkVertexInputRate ToVkVertexInputRate(VGPUVertexStepMode mode)
    {
        switch (mode)
        {
            default:
            case VGPUVertexStepMode_Vertex:
                return VK_VERTEX_INPUT_RATE_VERTEX;
            case VGPUVertexStepMode_Instance:
                return VK_VERTEX_INPUT_RATE_INSTANCE;
        }
    }

    constexpr VkShaderStageFlags ToVkShaderStageFlags(VGPUShaderStageFlags stage)
    {
        if (stage & VGPUShaderStage_All)
            return VK_SHADER_STAGE_ALL;

        VkShaderStageFlags flags = 0;
        if (stage & VGPUShaderStage_Vertex)
            flags |= VK_SHADER_STAGE_VERTEX_BIT;
        if (stage & VGPUShaderStage_Hull)
            flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if (stage & VGPUShaderStage_Domain)
            flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        if (stage & VGPUShaderStage_Geometry)
            flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
        if (stage & VGPUShaderStage_Fragment)
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if (stage & VGPUShaderStage_Compute)
            flags |= VK_SHADER_STAGE_COMPUTE_BIT;
        if (stage & VGPUShaderStage_Amplification)
            flags |= VK_SHADER_STAGE_TASK_BIT_EXT;
        if (stage & VGPUShaderStage_Mesh)
            flags |= VK_SHADER_STAGE_MESH_BIT_EXT;
        return flags;
    }


    static constexpr uint32_t kVulkanBindingShiftBuffer = 0u;
    static constexpr uint32_t kVulkanBindingShiftSRV = 1000u;
    static constexpr uint32_t kVulkanBindingShiftUAV = 2000u;
    static constexpr uint32_t kVulkanBindingShiftSampler = 3000u;



    inline uint32_t VkGetRegisterOffset(VkDescriptorType type)
    {
        // This needs to map with ShaderCompiler
        switch (type)
        {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                return kVulkanBindingShiftSampler;

            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                return kVulkanBindingShiftSRV;

            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                return kVulkanBindingShiftUAV;

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                return kVulkanBindingShiftBuffer;

            case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                return kVulkanBindingShiftSRV;

            default:
                return 0;
        }
    }

    static_assert(sizeof(VGPUViewport) == sizeof(VkViewport), "Viewport mismatch");
    static_assert(offsetof(VGPUViewport, x) == offsetof(VkViewport, x), "Layout mismatch");
    static_assert(offsetof(VGPUViewport, y) == offsetof(VkViewport, y), "Layout mismatch");
    static_assert(offsetof(VGPUViewport, width) == offsetof(VkViewport, width), "Layout mismatch");
    static_assert(offsetof(VGPUViewport, height) == offsetof(VkViewport, height), "Layout mismatch");
    static_assert(offsetof(VGPUViewport, minDepth) == offsetof(VkViewport, minDepth), "Layout mismatch");
    static_assert(offsetof(VGPUViewport, maxDepth) == offsetof(VkViewport, maxDepth), "Layout mismatch");

    static_assert(sizeof(VGPURect) == sizeof(VkRect2D), "VGPURect mismatch");
    static_assert(offsetof(VGPURect, x) == offsetof(VkRect2D, offset.x), "VGPURect Layout mismatch");
    static_assert(offsetof(VGPURect, y) == offsetof(VkRect2D, offset.y), "VGPURect Layout mismatch");
    static_assert(offsetof(VGPURect, width) == offsetof(VkRect2D, extent.width), "VGPURect Layout mismatch");
    static_assert(offsetof(VGPURect, height) == offsetof(VkRect2D, extent.height), "VGPURect Layout mismatch");

    static_assert(sizeof(VGPUDispatchIndirectCommand) == sizeof(VkDispatchIndirectCommand), "DispatchIndirectCommand mismatch");
    static_assert(offsetof(VGPUDispatchIndirectCommand, x) == offsetof(VkDispatchIndirectCommand, x), "Layout mismatch");
    static_assert(offsetof(VGPUDispatchIndirectCommand, y) == offsetof(VkDispatchIndirectCommand, y), "Layout mismatch");
    static_assert(offsetof(VGPUDispatchIndirectCommand, z) == offsetof(VkDispatchIndirectCommand, z), "Layout mismatch");

    static_assert(sizeof(VGPUDrawIndirectCommand) == sizeof(VkDrawIndirectCommand), "DrawIndirectCommand mismatch");
    static_assert(offsetof(VGPUDrawIndirectCommand, vertexCount) == offsetof(VkDrawIndirectCommand, vertexCount), "Layout mismatch");
    static_assert(offsetof(VGPUDrawIndirectCommand, instanceCount) == offsetof(VkDrawIndirectCommand, instanceCount), "Layout mismatch");
    static_assert(offsetof(VGPUDrawIndirectCommand, firstVertex) == offsetof(VkDrawIndirectCommand, firstVertex), "Layout mismatch");
    static_assert(offsetof(VGPUDrawIndirectCommand, firstInstance) == offsetof(VkDrawIndirectCommand, firstInstance), "Layout mismatch");

    static_assert(sizeof(VGPUDrawIndexedIndirectCommand) == sizeof(VkDrawIndexedIndirectCommand), "DrawIndexedIndirectCommand mismatch");
    static_assert(offsetof(VGPUDrawIndexedIndirectCommand, indexCount) == offsetof(VkDrawIndexedIndirectCommand, indexCount), "Layout mismatch");
    static_assert(offsetof(VGPUDrawIndexedIndirectCommand, instanceCount) == offsetof(VkDrawIndexedIndirectCommand, instanceCount), "Layout mismatch");
    static_assert(offsetof(VGPUDrawIndexedIndirectCommand, firstIndex) == offsetof(VkDrawIndexedIndirectCommand, firstIndex), "Layout mismatch");
    static_assert(offsetof(VGPUDrawIndexedIndirectCommand, baseVertex) == offsetof(VkDrawIndexedIndirectCommand, vertexOffset), "Layout mismatch");
    static_assert(offsetof(VGPUDrawIndexedIndirectCommand, firstInstance) == offsetof(VkDrawIndexedIndirectCommand, firstInstance), "Layout mismatch");
}

/// Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x) \
	do \
	{ \
		VkResult err = x; \
		if (err) \
		{ \
			vgpuLogError("Detected Vulkan error: %s", ToString(err)); \
		} \
	} while (0)

#define VK_LOG_ERROR(result, message) vgpuLogError("Vulkan: %s, error: %s", message, ToString(result));

struct VulkanDevice;

struct VulkanBuffer final : public VGPUBufferImpl
{
    VulkanDevice* renderer = nullptr;
    VkBuffer handle = VK_NULL_HANDLE;
    VmaAllocation  allocation = nullptr;
    uint64_t size = 0;
    VGPUBufferUsageFlags usage = 0;
    uint64_t allocatedSize = 0;
    VkDeviceAddress gpuAddress = 0;
    void* pMappedData = nullptr;

    ~VulkanBuffer() override;
    void SetLabel(const char* label) override;

    uint64_t GetSize() const override { return size; }
    VGPUBufferUsageFlags GetUsage() const override { return usage; }
    VGPUDeviceAddress GetGpuAddress() const override { return gpuAddress; }
};

struct VulkanTexture final : public VGPUTextureImpl
{
    VulkanDevice* renderer = nullptr;
    VkImage handle = VK_NULL_HANDLE;
    VmaAllocation  allocation = VK_NULL_HANDLE;

    VGPUTextureDimension dimension = VGPUTextureDimension_2D;
    VGPUTextureFormat format{};
    uint32_t width = 0;
    uint32_t height = 0;
    VkFormat vkFormat = VK_FORMAT_UNDEFINED;
    std::unordered_map<size_t, VkImageView> viewCache;
    void* sharedHandle = nullptr;

    ~VulkanTexture() override;
    void SetLabel(const char* label) override;

    VGPUTextureDimension GetDimension() const override { return dimension; }
    VGPUTextureFormat GetFormat() const override { return format; }

    VkImageView GetView(uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount);
    VkImageView GetRTV(uint32_t level, uint32_t slice);
};

struct VulkanSampler final : public VGPUSamplerImpl
{
    VulkanDevice* renderer = nullptr;
    VkSampler handle = VK_NULL_HANDLE;

    ~VulkanSampler() override;
    void SetLabel(const char* label) override;
};

struct VulkanBindGroupLayout final : public VGPUBindGroupLayoutImpl
{
    VulkanDevice* device = nullptr;
    VkDescriptorSetLayout handle = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    std::vector<uint32_t> layoutBindingsOriginal;
    bool isBindless = false;

    ~VulkanBindGroupLayout() override;
    void SetLabel(const char* label) override;
};

struct VulkanPipelineLayout final : public VGPUPipelineLayoutImpl
{
    VulkanDevice* device = nullptr;
    VkPipelineLayout handle = VK_NULL_HANDLE;

    uint32_t bindGroupLayoutCount = 0;
    std::vector<VkPushConstantRange>  pushConstantRanges;

    ~VulkanPipelineLayout() override;
    void SetLabel(const char* label) override;
};

struct VulkanBindGroup final : public VGPUBindGroupImpl
{
    VulkanDevice* device = nullptr;
    VulkanBindGroupLayout* bindGroupLayout = nullptr;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    ~VulkanBindGroup() override;
    void SetLabel(const char* label) override;
    void Update(size_t entryCount, const VGPUBindGroupEntry* entries) override;
};

struct VulkanPipeline final : public VGPUPipelineImpl
{
    VulkanDevice* renderer = nullptr;
    VGPUPipelineType type = VGPUPipelineType_Render;
    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    VulkanPipelineLayout* pipelineLayout = nullptr;
    VkPipeline handle = VK_NULL_HANDLE;

    ~VulkanPipeline() override;
    void SetLabel(const char* label) override;
    VGPUPipelineType GetType() const override { return type; }
};

struct VulkanQueryHeap final : public VGPUQueryHeapImpl
{
    VulkanDevice* renderer = nullptr;
    VGPUQueryType type = _VGPUQueryType_Force32;
    uint32_t count = 0;
    VkQueryPool handle = VK_NULL_HANDLE;
    VkDeviceSize resultSize = 0;

    ~VulkanQueryHeap() override;
    void SetLabel(const char* label) override;
    VGPUQueryType GetType() const override { return type; }
    uint32_t GetCount() const override { return count; }
};

struct VulkanSwapChain final : public VGPUSwapChainImpl
{
    VulkanDevice* renderer = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR handle = VK_NULL_HANDLE;
    VkExtent2D extent;
    bool vsync = false;
    VGPUTextureFormat colorFormat;
    bool allowHDR = true;
    uint32_t imageIndex;
    std::vector<VulkanTexture*> backbufferTextures;

    VkSemaphore acquireSemaphore = VK_NULL_HANDLE;
    VkSemaphore releaseSemaphore = VK_NULL_HANDLE;

    ~VulkanSwapChain() override;
    bool Update();
    void SetLabel(const char* label) override;
    VGPUTextureFormat GetFormat() const override { return colorFormat; }
    uint32_t GetWidth() const override { return extent.width; }
    uint32_t GetHeight() const override { return extent.height; }
};

class VulkanCommandBuffer final : public VGPUCommandBufferImpl
{
public:
    VulkanDevice* renderer = nullptr;
    VGPUCommandQueue queueType;

    VkCommandPool commandPools[VGPU_MAX_INFLIGHT_FRAMES];
    VkCommandBuffer commandBuffers[VGPU_MAX_INFLIGHT_FRAMES];
    VkCommandBuffer commandBuffer;
    VkSemaphore semaphore = VK_NULL_HANDLE;

    uint32_t clearValueCount = 0;
    VkClearValue clearValues[VGPU_MAX_COLOR_ATTACHMENTS + 1];

    VulkanPipeline* currentPipeline = nullptr;
    bool hasLabel = false;
    bool insideRenderPass = false;
    bool hasRenderPassLabel = false;
    std::vector<VulkanSwapChain*> presentSwapChains;

    bool bindGroupsDirty{ false };
    uint32_t numBoundBindGroups{ 0 };
    VulkanBindGroup* boundBindGroups[VGPU_MAX_BIND_GROUPS] = {};
    VkDescriptorSet descriptorSets[VGPU_MAX_BIND_GROUPS] = {};

    ~VulkanCommandBuffer() override;

    void Reset();
    void Begin(uint32_t frameIndex, const char* label);

    void InsertImageMemoryBarrier(
        VkImage                 image,
        VkAccessFlags           src_access_mask,
        VkAccessFlags           dst_access_mask,
        VkImageLayout           old_layout,
        VkImageLayout           new_layout,
        VkPipelineStageFlags    src_stage_mask,
        VkPipelineStageFlags    dst_stage_mask,
        VkImageSubresourceRange subresource_range)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.srcAccessMask = src_access_mask;
        barrier.dstAccessMask = dst_access_mask;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.image = image;
        barrier.subresourceRange = subresource_range;

        vkCmdPipelineBarrier(
            commandBuffer,
            src_stage_mask,
            dst_stage_mask,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }

    void PushDebugGroup(const char* groupLabel) override;
    void PopDebugGroup() override;
    void InsertDebugMarker(const char* debugLabel) override;
    void ClearBuffer(VGPUBuffer buffer, uint64_t offset, uint64_t size) override;

    void SetPipeline(VGPUPipeline pipeline) override;
    void SetBindGroup(uint32_t groupIndex, VGPUBindGroup bindGroup) override;
    void SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size) override;

    void FlushBindGroups();
    void PrepareDispatch();
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    void DispatchIndirect(VGPUBuffer buffer, uint64_t offset) override;

    VGPUTexture AcquireSwapchainTexture(VGPUSwapChain swapChain) override;

    void BeginRenderPass(const VGPURenderPassDesc* desc) override;
    void EndRenderPass() override;

    void SetViewport(const VGPUViewport* viewport) override;
    void SetViewports(uint32_t count, const VGPUViewport* viewports) override;
    void SetScissorRect(const VGPURect* rects) override;
    void SetScissorRects(uint32_t count, const VGPURect* rects) override;

    void SetVertexBuffer(uint32_t index, VGPUBuffer buffer, uint64_t offset) override;
    void SetIndexBuffer(VGPUBuffer buffer, VGPUIndexType type, uint64_t offset) override;
    void SetStencilReference(uint32_t reference) override;

    void BeginQuery(VGPUQueryHeap heap, uint32_t index) override;
    void EndQuery(VGPUQueryHeap heap, uint32_t index) override;
    void ResolveQuery(VGPUQueryHeap heap, uint32_t index, uint32_t count, VGPUBuffer destinationBuffer, uint64_t destinationOffset) override;
    void ResetQuery(VGPUQueryHeap heap, uint32_t index, uint32_t count) override;

    void PrepareDraw();
    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) override;
    void DrawIndirect(VGPUBuffer indirectBuffer, uint64_t indirectBufferOffset) override;
    void DrawIndexedIndirect(VGPUBuffer indirectBuffer, uint64_t indirectBufferOffset) override;

    void DispatchMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;
    void DispatchMeshIndirect(VGPUBuffer indirectBuffer, uint64_t indirectBufferOffset) override;
    void DispatchMeshIndirectCount(VGPUBuffer indirectBuffer, uint64_t indirectBufferOffset, VGPUBuffer countBuffer, uint64_t countBufferOffset, uint32_t maxCount) override;
};

struct VulkanUploadContext final
{
    VkCommandPool transferCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer transferCommandBuffer = VK_NULL_HANDLE;
    VkCommandPool transitionCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer transitionCommandBuffer = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkSemaphore semaphores[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE }; // graphics, compute, video

    uint64_t uploadBufferSize = 0;
    VulkanBuffer* uploadBuffer = nullptr;
    void* uploadBufferData = nullptr;

    inline bool IsValid() const { return transferCommandBuffer != VK_NULL_HANDLE; }
};

struct VulkanQueue final
{
    VkQueue queue = VK_NULL_HANDLE;

    std::vector<VulkanSwapChain*> swapchainUpdates;
    std::vector<VkSwapchainKHR> submitSwapchains;
    std::vector<uint32_t> submitSwapchainImageIndices;

    std::vector<VkSemaphore> submitWaitSemaphores;
    std::vector<VkPipelineStageFlags> submitWaitStages;
    std::vector<VkCommandBuffer> submitCommandBuffers;
    std::vector<VkSemaphore> submitSignalSemaphores;
    // KHR_synchronization2
    std::vector<VkSemaphoreSubmitInfo> submitWaitSemaphoreInfos;
    std::vector<VkSemaphoreSubmitInfo> submitSignalSemaphoreInfos;
    std::vector<VkCommandBufferSubmitInfo> submitCommandBufferInfos;

    bool sparseBindingSupported = false;
    std::mutex locker;

    VkFence frameFences[VGPU_MAX_INFLIGHT_FRAMES] = {};

    void Submit(VulkanDevice* device, VkFence fence);
};

struct VulkanDevice final : public VGPUDeviceImpl
{
public:
    ~VulkanDevice() override;

    bool Init(const VGPUDeviceDesc* desc);
    void SetLabel(const char* label) override;
    void WaitIdle() override;
    VGPUBackend GetBackendType() const override { return VGPUBackend_Vulkan; }
    VGPUBool32 QueryFeatureSupport(VGPUFeature feature) const override;
    void GetAdapterProperties(VGPUAdapterProperties* properties) const override;
    void GetLimits(VGPULimits* limits) const override;
    uint64_t GetTimestampFrequency() const  override { return timestampFrequency; }

    VGPUBuffer CreateBuffer(const VGPUBufferDesc* desc, const void* pInitialData) override;
    VGPUTexture CreateTexture(const VGPUTextureDesc* desc, const VGPUTextureData* pInitialData) override;
    VGPUSampler CreateSampler(const VGPUSamplerDesc* desc) override;

    VGPUBindGroupLayout CreateBindGroupLayout(const VGPUBindGroupLayoutDesc* desc) override;
    VGPUPipelineLayout CreatePipelineLayout(const VGPUPipelineLayoutDesc* desc) override;
    VGPUBindGroup CreateBindGroup(const VGPUBindGroupLayout layout, const VGPUBindGroupDesc* desc) override;

    VGPUPipeline CreateRenderPipeline(const VGPURenderPipelineDesc* desc) override;
    VGPUPipeline CreateComputePipeline(const VGPUComputePipelineDesc* desc) override;
    VGPUPipeline CreateRayTracingPipeline(const VGPURayTracingPipelineDesc* desc) override;

    VGPUQueryHeap CreateQueryHeap(const VGPUQueryHeapDesc* desc) override;

    VGPUSwapChain CreateSwapChain(const VGPUSwapChainDesc* desc) override;

    VGPUCommandBuffer BeginCommandBuffer(VGPUCommandQueue queueType, const char* label) override;
    uint64_t Submit(VGPUCommandBuffer* commandBuffers, uint32_t count) override;

    VulkanUploadContext Allocate(uint64_t size);
    void UploadSubmit(VulkanUploadContext context);
    void SetObjectName(VkObjectType type, uint64_t handle, const char* name);
    void ProcessDeletionQueue();

    void* GetNativeObject(VGPUNativeObjectType objectType) const override;

    VkSurfaceKHR CreateSurface(const VGPUSwapChainDesc* desc);
    bool GetImageFormatProperties(const VkImageCreateInfo& createInfo, const void* pNext, VkImageFormatProperties2* imageFormatProperties2) const;
    bool GetImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, const void* pNext, VkImageFormatProperties2* imageFormatProperties2) const;
    VkDescriptorPool CreateDescriptorSetPool();

public:
#if defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
    struct {
        void* handle;
        PFN_XGetXCBConnection GetXCBConnection;
    } x11xcb;
#endif

    bool debugUtils;
    bool xlib_surface;
    bool xcb_surface;
    bool wayland_surface;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugUtilsMessenger;

    PhysicalDeviceExtensions supportedExtensions;

    // Features
    VkPhysicalDeviceFeatures2 features2 = {};
    VkPhysicalDeviceVulkan11Features features1_1 = {};
    VkPhysicalDeviceVulkan12Features features1_2 = {};
    VkPhysicalDeviceVulkan13Features features1_3 = {};

    // Core in 1.3
    VkPhysicalDeviceMaintenance4Features maintenance4Features = {};
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
    VkPhysicalDeviceSynchronization2Features synchronization2Features = {};
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {};
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features = {};

    VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = {};
    VkPhysicalDevicePerformanceQueryFeaturesKHR perf_counter_features = {};
    VkPhysicalDeviceHostQueryResetFeatures host_query_reset_features = {};
    VkPhysicalDeviceTextureCompressionASTCHDRFeatures astc_hdrFeatures = {};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features = {};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_features = {};
    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = {};
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = {};
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};
    VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeatures = {};

    // Properties
    VkPhysicalDeviceProperties2 properties2 = {};
    VkPhysicalDeviceVulkan11Properties properties_1_1 = {};
    VkPhysicalDeviceVulkan12Properties properties_1_2 = {};
    VkPhysicalDeviceVulkan13Properties properties_1_3 = {};

    VkPhysicalDeviceDriverProperties driverProperties = {};
    VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties = {};
    VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProperties = {};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = {};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = {};
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties = {};
    VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties = {};
    VkPhysicalDeviceMemoryProperties2 memoryProperties2 = {};
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterProps = {};

    VkDeviceSize minAllocationAlignment{ 0 };
    std::string driverDescription;
    bool synchronization2{ false };
    bool dynamicRendering{ false };

    VkPhysicalDevice physicalDevice;
    struct QueueFamilyIndices {
        uint32_t queueFamilyCount = 0;

        uint32_t familyIndices[_VGPUCommandQueue_Count];
        uint32_t queueIndices[_VGPUCommandQueue_Count] = {};
        uint32_t counts[_VGPUCommandQueue_Count] = {};

        uint32_t timestampValidBits = 0;

        std::vector<uint32_t> queueOffsets;
        std::vector<std::vector<float>> queuePriorities;

        QueueFamilyIndices()
        {
            for (auto& index : familyIndices)
            {
                index = VK_QUEUE_FAMILY_IGNORED;
            }
        }
    } queueFamilyIndices{};

    VkDevice device = VK_NULL_HANDLE;
    VulkanQueue queues[_VGPUCommandQueue_Count];
    VmaAllocator allocator = VK_NULL_HANDLE;
    VmaAllocator externalAllocator = VK_NULL_HANDLE;
    uint64_t timestampFrequency = 0;

    /* Command contexts */
    std::mutex cmdBuffersLocker;
    uint32_t cmdBuffersCount{ 0 };
    std::vector<VulkanCommandBuffer*> commandBuffersPool;

    std::mutex uploadLocker;
    std::vector<VulkanUploadContext> uploadFreeList;

    VkBuffer		nullBuffer = VK_NULL_HANDLE;
    VmaAllocation	nullBufferAllocation = VK_NULL_HANDLE;
    VkBufferView	nullBufferView = VK_NULL_HANDLE;
    VkSampler		nullSampler = VK_NULL_HANDLE;
    VmaAllocation	nullImageAllocation1D = VK_NULL_HANDLE;
    VmaAllocation	nullImageAllocation2D = VK_NULL_HANDLE;
    VmaAllocation	nullImageAllocation3D = VK_NULL_HANDLE;
    VkImage			nullImage1D = VK_NULL_HANDLE;
    VkImage			nullImage2D = VK_NULL_HANDLE;
    VkImage			nullImage3D = VK_NULL_HANDLE;
    VkImageView		nullImageView1D = VK_NULL_HANDLE;
    VkImageView		nullImageView1DArray = VK_NULL_HANDLE;
    VkImageView		nullImageView2D = VK_NULL_HANDLE;
    VkImageView		nullImageView2DArray = VK_NULL_HANDLE;
    VkImageView		nullImageViewCube = VK_NULL_HANDLE;
    VkImageView		nullImageViewCubeArray = VK_NULL_HANDLE;
    VkImageView		nullImageView3D = VK_NULL_HANDLE;

    std::vector<VkDynamicState> psoDynamicStates;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};

    // Caches
    std::vector<VkDescriptorPool> descriptorSetPools;

    // Deletion queue objects
    std::mutex destroyMutex;
    std::deque<std::pair<VmaAllocation, uint64_t>> destroyedAllocations;
    std::deque<std::pair<std::pair<VkBuffer, VmaAllocation>, uint64_t>> destroyedBuffers;
    std::deque<std::pair<std::pair<VkImage, VmaAllocation>, uint64_t>> destroyedImages;
    std::deque<std::pair<VkImageView, uint64_t>> destroyedImageViews;
    std::deque<std::pair<VkSampler, uint64_t>> destroyedSamplers;
    std::deque<std::pair<VkDescriptorSetLayout, uint64_t>> destroyedDescriptorSetLayouts;
    std::deque<std::pair<VkPipelineLayout, uint64_t>> destroyedPipelineLayouts;
    std::deque<std::pair<VkShaderModule, uint64_t>> destroyedShaderModules;
    std::deque<std::pair<VkPipeline, uint64_t>> destroyedPipelines;
    std::deque<std::pair<std::pair<VkDescriptorPool, VkDescriptorSet>, uint64_t>> destroyedDescriptorSets;
    std::deque<std::pair<VkQueryPool, uint64_t>> destroyedQueryPools;
};

VulkanUploadContext VulkanDevice::Allocate(uint64_t size)
{
    VulkanUploadContext context;

    uploadLocker.lock();
    // Try to search for a staging buffer that can fit the request:
    for (size_t i = 0; i < uploadFreeList.size(); ++i)
    {
        if (uploadFreeList[i].uploadBufferSize >= size)
        {
            if (vkGetFenceStatus(device, uploadFreeList[i].fence) == VK_SUCCESS)
            {
                context = std::move(uploadFreeList[i]);
                std::swap(uploadFreeList[i], uploadFreeList.back());
                uploadFreeList.pop_back();
                break;
            }
        }
    }
    uploadLocker.unlock();

    // If no buffer was found that fits the data then create new one.
    if (!context.IsValid())
    {
        VkCommandPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        poolCreateInfo.queueFamilyIndex = queueFamilyIndices.familyIndices[VGPUCommandQueue_Copy];
        VK_CHECK(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &context.transferCommandPool));

        poolCreateInfo.queueFamilyIndex = queueFamilyIndices.familyIndices[VGPUCommandQueue_Graphics];
        VK_CHECK(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &context.transitionCommandPool));

        VkCommandBufferAllocateInfo commandBufferInfo = {};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.commandPool = context.transferCommandPool;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandBufferCount = 1u;
        VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferInfo, &context.transferCommandBuffer));

        commandBufferInfo.commandPool = context.transitionCommandPool;
        VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferInfo, &context.transitionCommandBuffer));

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &context.fence));

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &context.semaphores[0]));
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &context.semaphores[1]));
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &context.semaphores[2]));

        context.uploadBufferSize = VmaNextPow2(size);
        context.uploadBufferSize = _VGPU_MAX(context.uploadBufferSize, uint64_t(65536));

        VGPUBufferDesc uploadBufferDesc{};
        uploadBufferDesc.label = "CopyAllocator::UploadBuffer";
        uploadBufferDesc.size = context.uploadBufferSize;
        uploadBufferDesc.cpuAccess = VGPUCpuAccessMode_Write;
        context.uploadBuffer = (VulkanBuffer*)CreateBuffer(&uploadBufferDesc, nullptr);
        context.uploadBufferData = context.uploadBuffer->pMappedData;
    }

    // Begin command list in valid state.
    VK_CHECK(vkResetCommandPool(device, context.transferCommandPool, 0));
    VK_CHECK(vkResetCommandPool(device, context.transitionCommandPool, 0));

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    VK_CHECK(vkBeginCommandBuffer(context.transferCommandBuffer, &beginInfo));
    VK_CHECK(vkBeginCommandBuffer(context.transitionCommandBuffer, &beginInfo));
    VK_CHECK(vkResetFences(device, 1, &context.fence));

    return context;
}

void VulkanDevice::UploadSubmit(VulkanUploadContext context)
{
    VK_CHECK(vkEndCommandBuffer(context.transferCommandBuffer));
    VK_CHECK(vkEndCommandBuffer(context.transitionCommandBuffer));

    VkSemaphoreSubmitInfo waitSemaphoreInfo{};
    waitSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;

    // Copy queue first
    {
        VkCommandBufferSubmitInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferInfo.commandBuffer = context.transferCommandBuffer;

        VkSemaphoreSubmitInfo signalSemaphoreInfo = {};
        signalSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalSemaphoreInfo.semaphore = context.semaphores[0]; // Signal for graphics queue
        signalSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = 0;
        submitInfo.pWaitSemaphoreInfos = nullptr;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandBufferInfo;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos = &signalSemaphoreInfo;

        std::scoped_lock lock(queues[VGPUCommandQueue_Copy].locker);
        VK_CHECK(vkQueueSubmit2(queues[VGPUCommandQueue_Copy].queue, 1, &submitInfo, VK_NULL_HANDLE));
    }

    // Graphics queue
    {
        waitSemaphoreInfo.semaphore = context.semaphores[0]; // Wait for copy queue
        waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkCommandBufferSubmitInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferInfo.commandBuffer = context.transitionCommandBuffer;

        VkSemaphoreSubmitInfo signalSemaphoreInfos[2] = {};
        signalSemaphoreInfos[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalSemaphoreInfos[0].semaphore = context.semaphores[1]; // Signal for compute queue
        signalSemaphoreInfos[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // Signal for compute queue

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = 1;
        submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandBufferInfo;

        //if (device->queues[QUEUE_VIDEO_DECODE].queue != VK_NULL_HANDLE)
        //{
        //    signalSemaphoreInfos[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        //    signalSemaphoreInfos[1].semaphore = cmd.semaphores[2]; // signal for video decode queue
        //    signalSemaphoreInfos[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // signal for video decode queue
        //    submitInfo.signalSemaphoreInfoCount = 2;
        //}
        //else
        {
            submitInfo.signalSemaphoreInfoCount = 1;
        }
        submitInfo.pSignalSemaphoreInfos = signalSemaphoreInfos;

        std::scoped_lock lock(queues[VGPUCommandQueue_Graphics].locker);
        VK_CHECK(vkQueueSubmit2(queues[VGPUCommandQueue_Graphics].queue, 1, &submitInfo, VK_NULL_HANDLE));
    }

    //if (device->queues[QUEUE_VIDEO_DECODE].queue != VK_NULL_HANDLE)
    //{
    //    waitSemaphoreInfo.semaphore = cmd.semaphores[2]; // wait for graphics queue
    //    waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    //
    //    submitInfo.waitSemaphoreInfoCount = 1;
    //    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
    //    submitInfo.commandBufferInfoCount = 0;
    //    submitInfo.pCommandBufferInfos = nullptr;
    //    submitInfo.signalSemaphoreInfoCount = 0;
    //    submitInfo.pSignalSemaphoreInfos = nullptr;
    //
    //    std::scoped_lock lock(device->queues[QUEUE_VIDEO_DECODE].locker);
    //    res = vkQueueSubmit2(device->queues[QUEUE_VIDEO_DECODE].queue, 1, &submitInfo, VK_NULL_HANDLE);
    //    assert(res == VK_SUCCESS);
    //}

    // This must be final submit in this function because it will also signal a fence for state tracking by CPU!
    {
        waitSemaphoreInfo.semaphore = context.semaphores[1]; // wait for graphics queue
        waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = 1;
        submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
        submitInfo.commandBufferInfoCount = 0;
        submitInfo.pCommandBufferInfos = nullptr;
        submitInfo.signalSemaphoreInfoCount = 0;
        submitInfo.pSignalSemaphoreInfos = nullptr;

        // Final submit also signals fence!
        std::scoped_lock lock(queues[VGPUCommandQueue_Compute].locker);
        VK_CHECK(vkQueueSubmit2(queues[VGPUCommandQueue_Compute].queue, 1, &submitInfo, context.fence));
    }

    std::scoped_lock lock(uploadLocker);
    uploadFreeList.push_back(context);
}

void VulkanDevice::SetObjectName(VkObjectType type, uint64_t handle, const char* name)
{
    if (!debugUtils)
    {
        return;
    }

    VkDebugUtilsObjectNameInfoEXT info;
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.pNext = nullptr;
    info.objectType = type;
    info.objectHandle = handle;
    info.pObjectName = name;
    VK_CHECK(vkSetDebugUtilsObjectNameEXT(device, &info));
}

void* VulkanDevice::GetNativeObject(VGPUNativeObjectType objectType) const
{
    switch (objectType)
    {
        case VGPUNativeObjectType_VkDevice:
            return device;
        case VGPUNativeObjectType_VkPhysicalDevice:
            return physicalDevice;
        case VGPUNativeObjectType_VkInstance:
            return instance;
        default:
            return nullptr;
    }
}

bool VulkanDevice::GetImageFormatProperties(const VkImageCreateInfo& createInfo, const void* pNext, VkImageFormatProperties2* imageFormatProperties2) const
{
    return GetImageFormatProperties(createInfo.format, createInfo.imageType, createInfo.tiling, createInfo.usage, createInfo.flags, pNext, imageFormatProperties2);
}

bool VulkanDevice::GetImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, const void* pNext, VkImageFormatProperties2* imageFormatProperties2) const
{
    VGPU_ASSERT(imageFormatProperties2->sType == VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2);

    VkPhysicalDeviceImageFormatInfo2 info = {};
    info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    info.pNext = pNext;
    info.format = format;
    info.type = type;
    info.tiling = tiling;
    info.usage = usage;
    info.flags = flags;

    const VkResult result = vkGetPhysicalDeviceImageFormatProperties2(physicalDevice, &info, imageFormatProperties2);
    return result == VK_SUCCESS;
}

VkDescriptorPool VulkanDevice::CreateDescriptorSetPool()
{
    const std::vector<VkDescriptorPoolSize> poolSizes{
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 512 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 16 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 512 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 128 },
            { VK_DESCRIPTOR_TYPE_SAMPLER, 8 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 8 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 1024;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT (bindless)
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Error when creating descriptor pool: {}");
        return VK_NULL_HANDLE;
    }

    return pool;
}

VkSurfaceKHR VulkanDevice::CreateSurface(const VGPUSwapChainDesc* desc)
{
    VkResult result = VK_SUCCESS;
    VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

#if defined(__ANDROID__)
    VkAndroidSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.window = (ANativeWindow*)desc->windowHandle;
    result = vkCreateAndroidSurfaceKHR(instance, &createInfo, NULL, &vk_surface);
#elif defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd = (HWND)desc->windowHandle;
    result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &vk_surface);
#elif defined(__APPLE__)
    VkMetalSurfaceCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    createInfo.pLayer = (const CAMetalLayer*)desc->windowHandle;
    result = vkCreateMetalSurfaceEXT(instance, &createInfo, nullptr, &vk_surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
    if (xlib_surface)
    {
        VkXlibSurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        createInfo.dpy = static_cast<Display*>(desc->displayHandle);
        createInfo.window = (uint32_t)desc->windowHandle;
        result = vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &vk_surface);
    }
    else if (xcb_surface)
    {
        VkXcbSurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        createInfo.connection = x11xcb.GetXCBConnection(static_cast<Display*>(desc->displayHandle));
        createInfo.window = (uint32_t)desc->windowHandle;
        result = vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &vk_surface);
    }
    else
    {
        VK_LOG_ERROR(result, "Vulkan: Both VK_KHR_xlib_surface and VK_KHR_xcb_surface are not supported");
    }
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    VkWaylandSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.display = static_cast<struct wl_display*>(desc->displayHandle);
    createInfo.surface = static_cast<struct wl_surface*>(desc->windowHandle);
    result = vkCreateWaylandSurfaceKHR(instance, &createInfo, nullptr, &vk_surface);
#endif

    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Failed to create surface");
    }

    return vk_surface;
}

void VulkanDevice::ProcessDeletionQueue()
{
    const auto destroy = [&](auto&& queue, auto&& handler) {
        while (!queue.empty()) {
            if (queue.front().second + VGPU_MAX_INFLIGHT_FRAMES < frameCount)
            {
                auto item = queue.front();
                queue.pop_front();
                handler(item.first);
            }
            else
            {
                break;
            }
        }
        };

    destroyMutex.lock();
    destroy(destroyedAllocations, [&](auto& item) { vmaFreeMemory(allocator, item); });
    destroy(destroyedBuffers, [&](auto& item) { vmaDestroyBuffer(allocator, item.first, item.second); });
    destroy(destroyedImages, [&](auto& item) { vmaDestroyImage(allocator, item.first, item.second); });
    destroy(destroyedImageViews, [&](auto& item) { vkDestroyImageView(device, item, nullptr); });
    destroy(destroyedSamplers, [&](auto& item) { vkDestroySampler(device, item, nullptr); });
    destroy(destroyedDescriptorSetLayouts, [&](auto& item) { vkDestroyDescriptorSetLayout(device, item, nullptr); });
    destroy(destroyedPipelineLayouts, [&](auto& item) { vkDestroyPipelineLayout(device, item, nullptr); });
    destroy(destroyedShaderModules, [&](auto& item) { vkDestroyShaderModule(device, item, nullptr); });
    destroy(destroyedPipelines, [&](auto& item) { vkDestroyPipeline(device, item, nullptr); });
    destroy(destroyedQueryPools, [&](auto& item) { vkDestroyQueryPool(device, item, nullptr); });
    destroy(destroyedDescriptorSets, [&](auto& item) { vkFreeDescriptorSets(device, item.first, 1u, &item.second); });

    destroyMutex.unlock();
}

/* VulkanBuffer */
VulkanBuffer::~VulkanBuffer()
{
    renderer->destroyMutex.lock();
    if (handle)
    {
        renderer->destroyedBuffers.push_back(std::make_pair(std::make_pair(handle, allocation), renderer->frameCount));
    }
    else if (allocation)
    {
        renderer->destroyedAllocations.push_back(std::make_pair(allocation, renderer->frameCount));
    }
    renderer->destroyMutex.unlock();
}

void VulkanBuffer::SetLabel(const char* label)
{
    renderer->SetObjectName(VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>(handle), label);
}

/* VulkanTexture */
VulkanTexture::~VulkanTexture()
{
    renderer->destroyMutex.lock();
    for (auto& it : viewCache)
    {
        renderer->destroyedImageViews.push_back(std::make_pair(it.second, renderer->frameCount));
    }
    viewCache.clear();
    if (allocation)
    {
        renderer->destroyedImages.push_back(std::make_pair(std::make_pair(handle, allocation), renderer->frameCount));
    }
    renderer->destroyMutex.unlock();
}

void VulkanTexture::SetLabel(const char* label)
{
    renderer->SetObjectName(VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64_t>(handle), label);
}

VkImageView VulkanTexture::GetView(uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount)
{
    size_t hash = 0;
    hash_combine(hash, baseMipLevel);
    hash_combine(hash, levelCount);
    hash_combine(hash, baseArrayLayer);
    hash_combine(hash, layerCount);

    auto it = viewCache.find(hash);
    if (it == viewCache.end())
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = handle;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vkFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = GetImageAspectFlags(viewInfo.format);
        viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
        viewInfo.subresourceRange.levelCount = levelCount;
        viewInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView newView;
        const VkResult result = vkCreateImageView(renderer->device, &viewInfo, nullptr, &newView);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create ImageView");
            return VK_NULL_HANDLE;
        }

        viewCache[hash] = newView;
        return newView;
    }

    return it->second;
}

VkImageView VulkanTexture::GetRTV(uint32_t level, uint32_t slice)
{
    return GetView(level, 1, slice, 1);
}

/* VulkanPipeline */
VulkanPipeline::~VulkanPipeline()
{
    pipelineLayout->Release();

    renderer->destroyMutex.lock();
    renderer->destroyedPipelines.push_back(std::make_pair(handle, renderer->frameCount));
    renderer->destroyMutex.unlock();
}

void VulkanPipeline::SetLabel(const char* label)
{
    renderer->SetObjectName(VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(handle), label);
}

/* VulkanDevice */
VulkanDevice::~VulkanDevice()
{
    VK_CHECK(vkDeviceWaitIdle(device));

    for (size_t i = 0; i < commandBuffersPool.size(); ++i)
    {
        VulkanCommandBuffer* commandBuffer = commandBuffersPool[i];
        delete commandBuffer;
    }
    commandBuffersPool.clear();

    for (uint8_t i = 0; i < _VGPUCommandQueue_Count; ++i)
    {
        if (queues[i].queue == VK_NULL_HANDLE)
            continue;

        for (uint32_t j = 0; j < VGPU_MAX_INFLIGHT_FRAMES; ++j)
        {
            vkDestroyFence(device, queues[i].frameFences[j], nullptr);
        }
    }

    // Destroy upload stuff
    vkQueueWaitIdle(queues[VGPUCommandQueue_Copy].queue);
    for (auto& context : uploadFreeList)
    {
        vkDestroyCommandPool(device, context.transferCommandPool, nullptr);
        vkDestroyCommandPool(device, context.transitionCommandPool, nullptr);
        vkDestroySemaphore(device, context.semaphores[0], nullptr);
        vkDestroySemaphore(device, context.semaphores[1], nullptr);
        vkDestroySemaphore(device, context.semaphores[2], nullptr);
        vkDestroyFence(device, context.fence, nullptr);

        uint32_t count = context.uploadBuffer->Release();
        VGPU_UNUSED(count);
        context.uploadBufferData = nullptr;
    }

    frameCount = UINT64_MAX;
    ProcessDeletionQueue();
    frameCount = 0;

    vmaDestroyBuffer(allocator, nullBuffer, nullBufferAllocation);
    vkDestroyBufferView(device, nullBufferView, nullptr);
    vmaDestroyImage(allocator, nullImage1D, nullImageAllocation1D);
    vmaDestroyImage(allocator, nullImage2D, nullImageAllocation2D);
    vmaDestroyImage(allocator, nullImage3D, nullImageAllocation3D);
    vkDestroyImageView(device, nullImageView1D, nullptr);
    vkDestroyImageView(device, nullImageView1DArray, nullptr);
    vkDestroyImageView(device, nullImageView2D, nullptr);
    vkDestroyImageView(device, nullImageView2DArray, nullptr);
    vkDestroyImageView(device, nullImageViewCube, nullptr);
    vkDestroyImageView(device, nullImageViewCubeArray, nullptr);
    vkDestroyImageView(device, nullImageView3D, nullptr);
    vkDestroySampler(device, nullSampler, nullptr);

    // Release caches
    {
        // Destroy Descriptor Pools
        for (VkDescriptorPool descriptorPool : descriptorSetPools)
        {
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        }
        descriptorSetPools.clear();
    }

    if (allocator != VK_NULL_HANDLE)
    {
#if defined(_DEBUG)
        VmaTotalStatistics stats;
        vmaCalculateStatistics(allocator, &stats);

        if (stats.total.statistics.allocationBytes > 0)
        {
            //    vgpuLogError("Total device memory leaked: {} bytes.", stats.total.usedBytes);
        }
#endif

        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }

    if (externalAllocator != VK_NULL_HANDLE)
    {
#if defined(_DEBUG)
        VmaTotalStatistics stats;
        vmaCalculateStatistics(externalAllocator, &stats);

        if (stats.total.statistics.allocationBytes > 0)
        {
            //    vgpuLogError("Total device memory leaked: {} bytes.", stats.total.usedBytes);
        }
#endif

        vmaDestroyAllocator(externalAllocator);
        externalAllocator = VK_NULL_HANDLE;
    }

    if (device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }

    if (debugUtilsMessenger != VK_NULL_HANDLE)
    {
        vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
        debugUtilsMessenger = VK_NULL_HANDLE;
    }

    if (instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }

#if defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
    if (x11xcb.handle)
    {
        dlclose(x11xcb.handle);
        x11xcb.handle = NULL;
    }
#endif
}

bool VulkanDevice::Init(const VGPUDeviceDesc* desc)
{
#if defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
#if defined(__CYGWIN__)
    x11xcb.handle = dlopen("libX11-xcb-1.so", RTLD_LAZY | RTLD_LOCAL);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
    x11xcb.handle = dlopen("libX11-xcb.so", RTLD_LAZY | RTLD_LOCAL);
#else
    x11xcb.handle = dlopen("libX11-xcb.so.1", RTLD_LAZY | RTLD_LOCAL);
#endif

    if (x11xcb.handle)
    {
        x11xcb.GetXCBConnection = (PFN_XGetXCBConnection)dlsym(x11xcb.handle, "XGetXCBConnection");
    }
#endif

    VkResult result = VK_SUCCESS;

    // Enumerate available layers and extensions:
    {
        uint32_t instanceLayerCount;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
        std::vector<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.data()));

        uint32_t extensionCount = 0;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> availableInstanceExtensions(extensionCount);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableInstanceExtensions.data()));

        std::vector<const char*> instanceLayers;
        std::vector<const char*> instanceExtensions;

        // MoltenVK
#if defined(__APPLE__)
        instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

        for (auto& availableExtension : availableInstanceExtensions)
        {
            if (strcmp(availableExtension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                debugUtils = true;
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            else if (strcmp(availableExtension.extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME) == 0)
            {
                instanceExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
            }
            else if (strcmp(availableExtension.extensionName, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME) == 0)
            {
                instanceExtensions.push_back(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
            }
            else if (strcmp(availableExtension.extensionName, "VK_KHR_xlib_surface") == 0)
            {
                xlib_surface = true;
            }
            else if (strcmp(availableExtension.extensionName, "VK_KHR_xcb_surface") == 0)
            {
                xcb_surface = true;
            }
            else if (strcmp(availableExtension.extensionName, "VK_KHR_wayland_surface") == 0)
            {
                wayland_surface = true;
            }
        }

        instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

        // Enable surface extensions depending on os
#if defined(_WIN32)
        instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
        instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
        instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#else
        if (xlib_surface)
        {
            instanceExtensions.push_back("VK_KHR_xlib_surface");
        }
        else if (xcb_surface)
        {
            instanceExtensions.push_back("VK_KHR_xcb_surface");
        }

        if (wayland_surface)
        {
            instanceExtensions.push_back("VK_KHR_wayland_surface");
        }
#endif

        if (desc->validationMode != VGPUValidationMode_Disabled)
        {
            // Determine the optimal validation layers to enable that are necessary for useful debugging
            std::vector<const char*> optimalValidationLyers = GetOptimalValidationLayers(availableInstanceLayers);
            instanceLayers.insert(instanceLayers.end(), optimalValidationLyers.begin(), optimalValidationLyers.end());
        }

#if defined(_DEBUG)
        bool validationFeatures = false;
        if (desc->validationMode == VGPUValidationMode_GPU)
        {
            uint32_t layerInstanceExtensionCount;
            VK_CHECK(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtensionCount, nullptr));
            std::vector<VkExtensionProperties> availableLayerInstanceExtensions(layerInstanceExtensionCount);
            VK_CHECK(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtensionCount, availableLayerInstanceExtensions.data()));

            for (auto& availableExtension : availableLayerInstanceExtensions)
            {
                if (strcmp(availableExtension.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0)
                {
                    validationFeatures = true;
                    instanceExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
                }
            }
        }
#endif 

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = desc->label;
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "vgpu";
        appInfo.engineVersion = VK_MAKE_VERSION(VGPU_VERSION_MAJOR, VGPU_VERSION_MINOR, VGPU_VERSION_PATCH);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
        createInfo.ppEnabledLayerNames = instanceLayers.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        createInfo.ppEnabledExtensionNames = instanceExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};

        if (desc->validationMode != VGPUValidationMode_Disabled && debugUtils)
        {
            debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugUtilsCreateInfo.pNext = nullptr;
            debugUtilsCreateInfo.flags = 0;

            debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            if (desc->validationMode == VGPUValidationMode_Verbose)
            {
                debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
            }

            debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugUtilsCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
            createInfo.pNext = &debugUtilsCreateInfo;
        }

#if defined(_DEBUG)
        VkValidationFeaturesEXT validationFeaturesInfo = {};
        validationFeaturesInfo.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        if (validationFeatures)
        {
            static const VkValidationFeatureEnableEXT enable_features[2] = {
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
            };
            validationFeaturesInfo.enabledValidationFeatureCount = 2;
            validationFeaturesInfo.pEnabledValidationFeatures = enable_features;
            validationFeaturesInfo.pNext = createInfo.pNext;
            createInfo.pNext = &validationFeaturesInfo;
        }
#endif

#if defined(__APPLE__)
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        result = vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create Vulkan instance.");
            return false;
        }

        GPU_FOREACH_INSTANCE(GPU_LOAD_INSTANCE);
        GPU_FOREACH_INSTANCE_PLATFORM(GPU_LOAD_INSTANCE);

        if (desc->validationMode != VGPUValidationMode_Disabled && debugUtils)
        {
            result = vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsCreateInfo, nullptr, &debugUtilsMessenger);
            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Could not create debug utils messenger");
            }
        }

#ifdef _DEBUG
        vgpuLogInfo("Created VkInstance with version: %d.%d.%d",
            VK_VERSION_MAJOR(appInfo.apiVersion),
            VK_VERSION_MINOR(appInfo.apiVersion),
            VK_VERSION_PATCH(appInfo.apiVersion)
        );

        if (createInfo.enabledLayerCount)
        {
            vgpuLogInfo("Enabled %d Validation Layers:", createInfo.enabledLayerCount);

            for (uint32_t i = 0; i < createInfo.enabledLayerCount; ++i)
            {
                vgpuLogInfo("	\t%s", createInfo.ppEnabledLayerNames[i]);
            }
        }

        vgpuLogInfo("Enabled %d Instance Extensions:", createInfo.enabledExtensionCount);
        for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
        {
            vgpuLogInfo("	\t%s", createInfo.ppEnabledExtensionNames[i]);
        }
#endif
    }

    // Enumerate physical device and create logical device.
    {
        uint32_t deviceCount = 0;
        VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

        if (deviceCount == 0)
        {
            vgpuLogError("Vulkan: Failed to find GPUs with Vulkan support");
            return false;
        }

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));

        std::vector<const char*> enabledDeviceExtensions;
        for (const VkPhysicalDevice& candidatePhysicalDevice : physicalDevices)
        {
            // We require minimum 1.2
            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(candidatePhysicalDevice, &physicalDeviceProperties);
            if (physicalDeviceProperties.apiVersion < VK_API_VERSION_1_2)
            {
                continue;
            }

            PhysicalDeviceExtensions physicalDeviceExt = QueryPhysicalDeviceExtensions(candidatePhysicalDevice);
            bool suitable = physicalDeviceExt.swapchain;

            if (!suitable)
            {
                continue;
            }

            bool priority = physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            if (desc->powerPreference == VGPUPowerPreference_LowPower)
            {
                priority = properties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
            }

            if (priority || physicalDevice == VK_NULL_HANDLE)
            {
                physicalDevice = candidatePhysicalDevice;

                if (priority)
                {
                    // If this is prioritized GPU type, look no further
                    break;
                }
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            vgpuLogError("Vulkan: Failed to find a suitable GPU");
            return false;
        }

        supportedExtensions = QueryPhysicalDeviceExtensions(physicalDevice);

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        // For debug purpose
        //gpuProps.apiVersion = VK_API_VERSION_1_2;

        // Features
        void** features_chain = nullptr;
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        features1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

        maintenance4Features = {};
        dynamicRenderingFeatures = {};
        synchronization2Features = {};
        extendedDynamicStateFeatures = {};
        extendedDynamicState2Features = {};

        astc_hdrFeatures = {};
        depthClipEnableFeatures = {};
        acceleration_structure_features = {};
        raytracing_features = {};
        rayQueryFeatures = {};
        fragmentShadingRateFeatures = {};
        meshShaderFeatures = {};
        conditionalRenderingFeatures = {};

        features2.pNext = &features1_1;
        if (physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_3)
        {
            features1_1.pNext = &features1_2;
            features1_2.pNext = &features1_3;
            features_chain = &features1_3.pNext;
        }
        else
        {
            // gpuProps.apiVersion >= VK_API_VERSION_1_2
            features1_1.pNext = &features1_2;
            features_chain = &features1_2.pNext;
        }

        // Properties
        void** propertiesChain = nullptr;
        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        properties_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
        properties_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
        properties_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
        driverProperties = {};
        depthStencilResolveProperties = {};
        accelerationStructureProperties = {};
        rayTracingPipelineProperties = {};
        fragmentShadingRateProperties = {};
        meshShaderProperties = {};
        conservativeRasterProps = {};

        properties2.pNext = &properties_1_1;
        if (physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_3)
        {
            properties_1_1.pNext = &properties_1_2;
            properties_1_2.pNext = &properties_1_3;
            propertiesChain = &properties_1_3.pNext;
        }
        else
        {
            // gpuProps.apiVersion >= VK_API_VERSION_1_2
            properties_1_1.pNext = &properties_1_2;
            propertiesChain = &properties_1_2.pNext;
        }

        samplerFilterMinmaxProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
        *propertiesChain = &samplerFilterMinmaxProperties;
        propertiesChain = &samplerFilterMinmaxProperties.pNext;

        depthStencilResolveProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;
        *propertiesChain = &depthStencilResolveProperties;
        propertiesChain = &depthStencilResolveProperties.pNext;


        enabledDeviceExtensions.clear();
        enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        if (supportedExtensions.memoryBudget)
        {
            enabledDeviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
        }

        if (supportedExtensions.AMD_device_coherent_memory)
        {
            enabledDeviceExtensions.push_back(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
        }

        if (supportedExtensions.memory_priority)
        {
            enabledDeviceExtensions.push_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
        }

        // Core in 1.3
        if (physicalDeviceProperties.apiVersion < VK_API_VERSION_1_3)
        {
            // Core in 1.3
            driverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
            *propertiesChain = &driverProperties;
            propertiesChain = &driverProperties.pNext;

            if (supportedExtensions.maintenance4)
            {
                enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

                maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
                *propertiesChain = &maintenance4Features;
                propertiesChain = &maintenance4Features.pNext;
            }

            if (supportedExtensions.dynamicRendering)
            {
                enabledDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

                dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
                *propertiesChain = &dynamicRenderingFeatures;
                propertiesChain = &dynamicRenderingFeatures.pNext;
            }

            if (supportedExtensions.synchronization2)
            {
                enabledDeviceExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

                synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
                *propertiesChain = &synchronization2Features;
                propertiesChain = &synchronization2Features.pNext;
            }

            if (supportedExtensions.extendedDynamicState)
            {
                enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);

                extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
                *features_chain = &extendedDynamicStateFeatures;
                features_chain = &extendedDynamicStateFeatures.pNext;
            }

            if (supportedExtensions.extendedDynamicState2)
            {
                enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);

                extendedDynamicState2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
                *features_chain = &extendedDynamicState2Features;
                features_chain = &extendedDynamicState2Features.pNext;
            }
        }

        if (supportedExtensions.textureCompressionAstcHdr)
        {
            enabledDeviceExtensions.push_back(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME);

            astc_hdrFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES;
            *features_chain = &astc_hdrFeatures;
            features_chain = &astc_hdrFeatures.pNext;
        }

        // For performance queries, we also use host query reset since queryPool resets cannot live in the same command buffer as beginQuery
        if (supportedExtensions.performanceQuery &&
            supportedExtensions.hostQueryReset)
        {
            enabledDeviceExtensions.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);

            perf_counter_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR;
            *features_chain = &perf_counter_features;
            features_chain = &perf_counter_features.pNext;

            host_query_reset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
            enabledDeviceExtensions.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
            *features_chain = &host_query_reset_features;
            features_chain = &host_query_reset_features.pNext;
        }

        if (supportedExtensions.depthClipEnable)
        {
            enabledDeviceExtensions.push_back(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);

            depthClipEnableFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT;
            *features_chain = &depthClipEnableFeatures;
            features_chain = &depthClipEnableFeatures.pNext;
        }

        if (supportedExtensions.conservativeRasterization)
        {
            enabledDeviceExtensions.push_back(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);

            conservativeRasterProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
            *propertiesChain = &conservativeRasterProps;
            propertiesChain = &conservativeRasterProps.pNext;
        }

        if (supportedExtensions.accelerationStructure)
        {
            // Required by VK_KHR_acceleration_structure
            VGPU_VERIFY(supportedExtensions.deferred_host_operations);

            enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            enabledDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

            acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
            *features_chain = &acceleration_structure_features;
            features_chain = &acceleration_structure_features.pNext;

            accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
            *propertiesChain = &accelerationStructureProperties;
            propertiesChain = &accelerationStructureProperties.pNext;

            if (supportedExtensions.raytracingPipeline)
            {
                enabledDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                raytracing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
                *features_chain = &raytracing_features;
                features_chain = &raytracing_features.pNext;

                rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
                *propertiesChain = &rayTracingPipelineProperties;
                propertiesChain = &rayTracingPipelineProperties.pNext;
            }

            if (supportedExtensions.rayQuery)
            {
                enabledDeviceExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);

                rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
                *features_chain = &rayQueryFeatures;
                features_chain = &rayQueryFeatures.pNext;
            }
        }

        if (supportedExtensions.fragment_shading_rate)
        {
            enabledDeviceExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

            fragmentShadingRateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
            *features_chain = &fragmentShadingRateFeatures;
            features_chain = &fragmentShadingRateFeatures.pNext;

            fragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
            *propertiesChain = &fragmentShadingRateProperties;
            propertiesChain = &fragmentShadingRateProperties.pNext;
        }

        if (supportedExtensions.meshShader)
        {
            enabledDeviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);

            meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
            *features_chain = &meshShaderFeatures;
            features_chain = &meshShaderFeatures.pNext;

            meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
            *propertiesChain = &meshShaderProperties;
            propertiesChain = &meshShaderProperties.pNext;
        }

        if (supportedExtensions.conditionalRendering)
        {
            enabledDeviceExtensions.push_back(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);

            conditionalRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT;
            *features_chain = &conditionalRenderingFeatures;
            features_chain = &conditionalRenderingFeatures.pNext;
        }

#if defined(_WIN32)
        if (supportedExtensions.externalMemory)
        {
            enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
        }

        if (supportedExtensions.externalSemaphore)
        {
            enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
        }

        if (supportedExtensions.externalFence)
        {
            enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
        }
#else
        if (supportedExtensions.externalMemory)
        {
            enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
        }

        if (supportedExtensions.externalSemaphore)
        {
            enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
        }

        if (supportedExtensions.externalFence)
        {
            enabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
        }
#endif

        vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
        vkGetPhysicalDeviceProperties2(physicalDevice, &properties2);

        // Memory properties
        memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &memoryProperties2);

        VGPU_VERIFY(features2.features.robustBufferAccess == VK_TRUE);
        VGPU_VERIFY(features2.features.depthBiasClamp == VK_TRUE);
        VGPU_VERIFY(features2.features.fragmentStoresAndAtomics == VK_TRUE);
        VGPU_VERIFY(features2.features.imageCubeArray == VK_TRUE);
        VGPU_VERIFY(features2.features.independentBlend == VK_TRUE);
        VGPU_VERIFY(features2.features.fullDrawIndexUint32 == VK_TRUE);
        VGPU_VERIFY(features2.features.sampleRateShading == VK_TRUE);
        VGPU_VERIFY(features2.features.shaderClipDistance == VK_TRUE);
        VGPU_VERIFY(features2.features.depthClamp == VK_TRUE);
        VGPU_VERIFY(features2.features.occlusionQueryPrecise == VK_TRUE);

        VGPU_ASSERT(features1_3.dynamicRendering == VK_TRUE || dynamicRenderingFeatures.dynamicRendering == VK_TRUE);

        synchronization2 = features1_3.synchronization2 == VK_TRUE || synchronization2Features.synchronization2 == VK_TRUE;
        dynamicRendering = features1_3.dynamicRendering == VK_TRUE || dynamicRenderingFeatures.dynamicRendering == VK_TRUE;

        if (!features2.features.textureCompressionBC &&
            !(features2.features.textureCompressionETC2 && features2.features.textureCompressionASTC_LDR))
        {
            vgpuLogError("Vulkan textureCompressionBC feature required or both textureCompressionETC2 and textureCompressionASTC required.");
            return false;
        }

        // Find queue families:
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount);
        std::vector<VkQueueFamilyVideoPropertiesKHR> queueFamiliesVideo(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            queueFamilies[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;

            if (supportedExtensions.video.queue)
            {
                queueFamilies[i].pNext = &queueFamiliesVideo[i];
                queueFamiliesVideo[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR;
            }
        }

        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, queueFamilies.data());

        queueFamilyIndices.queueFamilyCount = queueFamilyCount;
        queueFamilyIndices.queueOffsets.resize(queueFamilyCount);
        queueFamilyIndices.queuePriorities.resize(queueFamilyCount);

        const auto FindVacantQueue = [&](uint32_t& family, uint32_t& index,
            VkQueueFlags required, VkQueueFlags ignore_flags,
            float priority) -> bool
            {
                for (uint32_t familyIndex = 0; familyIndex < queueFamilyCount; familyIndex++)
                {
                    if ((queueFamilies[familyIndex].queueFamilyProperties.queueFlags & ignore_flags) != 0)
                        continue;

                    // A graphics queue candidate must support present for us to select it.
                    if ((required & VK_QUEUE_GRAPHICS_BIT) != 0)
                    {
                        VkBool32 supported = vulkan_queryPresentationSupport(physicalDevice, familyIndex);
                        if (!supported)
                            continue;
                        //if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, vk_surface, &supported) != VK_SUCCESS || !supported)
                        //    continue;
                    }

                    // A video decode queue candidate must support VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR or VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR
                    if ((required & VK_QUEUE_VIDEO_DECODE_BIT_KHR) != 0)
                    {
                        VkVideoCodecOperationFlagsKHR videoCodecOperations = queueFamiliesVideo[familyIndex].videoCodecOperations;

                        if ((videoCodecOperations & VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) == 0 &&
                            (videoCodecOperations & VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR) == 0)
                        {
                            continue;
                        }
                    }

                    if (queueFamilies[familyIndex].queueFamilyProperties.queueCount &&
                        (queueFamilies[familyIndex].queueFamilyProperties.queueFlags & required) == required)
                    {
                        family = familyIndex;
                        queueFamilies[familyIndex].queueFamilyProperties.queueCount--;
                        index = queueFamilyIndices.queueOffsets[familyIndex]++;
                        queueFamilyIndices.queuePriorities[familyIndex].push_back(priority);
                        return true;
                    }
                }

                return false;
            };

        if (!FindVacantQueue(queueFamilyIndices.familyIndices[VGPUCommandQueue_Graphics], queueFamilyIndices.queueIndices[VGPUCommandQueue_Graphics],
            VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 0.5f))
        {
            vgpuLogError("Vulkan: Could not find suitable graphics queue.");
            return false;
        }

        // XXX: This assumes timestamp valid bits is the same for all queue types.
        queueFamilyIndices.timestampValidBits = queueFamilies[queueFamilyIndices.familyIndices[VGPUCommandQueue_Graphics]].queueFamilyProperties.timestampValidBits;

        // Prefer another graphics queue since we can do async graphics that way.
        // The compute queue is to be treated as high priority since we also do async graphics on it.
        if (!FindVacantQueue(queueFamilyIndices.familyIndices[VGPUCommandQueue_Compute], queueFamilyIndices.queueIndices[VGPUCommandQueue_Compute], VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 1.0f) &&
            !FindVacantQueue(queueFamilyIndices.familyIndices[VGPUCommandQueue_Compute], queueFamilyIndices.queueIndices[VGPUCommandQueue_Compute], VK_QUEUE_COMPUTE_BIT, 0, 1.0f))
        {
            // Fallback to the graphics queue if we must.
            queueFamilyIndices.familyIndices[VGPUCommandQueue_Compute] = queueFamilyIndices.familyIndices[VGPUCommandQueue_Graphics];
            queueFamilyIndices.queueIndices[VGPUCommandQueue_Compute] = queueFamilyIndices.queueIndices[VGPUCommandQueue_Graphics];
        }

        // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
        // If not, fallback to a dedicated compute queue.
        // Finally, fallback to same queue as compute.
        if (!FindVacantQueue(queueFamilyIndices.familyIndices[VGPUCommandQueue_Copy], queueFamilyIndices.queueIndices[VGPUCommandQueue_Copy], VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0.5f) &&
            !FindVacantQueue(queueFamilyIndices.familyIndices[VGPUCommandQueue_Copy], queueFamilyIndices.queueIndices[VGPUCommandQueue_Copy], VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, 0.5f))
        {
            queueFamilyIndices.familyIndices[VGPUCommandQueue_Copy] = queueFamilyIndices.familyIndices[VGPUCommandQueue_Compute];
            queueFamilyIndices.queueIndices[VGPUCommandQueue_Copy] = queueFamilyIndices.queueIndices[VGPUCommandQueue_Compute];
        }

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        for (uint32_t familyIndex = 0; familyIndex < queueFamilyCount; familyIndex++)
        {
            if (queueFamilyIndices.queueOffsets[familyIndex] == 0)
                continue;

            VkDeviceQueueCreateInfo queueInfo = {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = familyIndex;
            queueInfo.queueCount = queueFamilyIndices.queueOffsets[familyIndex];
            queueInfo.pQueuePriorities = queueFamilyIndices.queuePriorities[familyIndex].data();
            queueInfos.push_back(queueInfo);
        }

        createInfo.pNext = &features2;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
        createInfo.pQueueCreateInfos = queueInfos.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.pEnabledFeatures = nullptr;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();

        result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Cannot create device");
            return false;
        }

        GPU_FOREACH_DEVICE(GPU_LOAD_DEVICE);
        GPU_LOAD_DEVICE(vkGetMemoryFdKHR);
        if (features1_3.synchronization2 == VK_TRUE)
        {
            GPU_LOAD_DEVICE(vkCmdPipelineBarrier2);
            GPU_LOAD_DEVICE(vkCmdWriteTimestamp2);
            GPU_LOAD_DEVICE(vkQueueSubmit2);
        }
        else if (synchronization2Features.synchronization2 == VK_TRUE)
        {
            vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier2KHR");
            vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)vkGetDeviceProcAddr(device, "vkCmdWriteTimestamp2KHR");
            vkQueueSubmit2 = (PFN_vkQueueSubmit2)vkGetDeviceProcAddr(device, "vkQueueSubmit2KHR");
        }

        if (meshShaderFeatures.meshShader == VK_TRUE && meshShaderFeatures.taskShader == VK_TRUE)
        {
            GPU_FOREACH_DEVICE_MESH_SHADER(GPU_LOAD_DEVICE);
        }

        // Queues
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        for (uint8_t i = 0; i < _VGPUCommandQueue_Count; i++)
        {
            if (queueFamilyIndices.familyIndices[i] != VK_QUEUE_FAMILY_IGNORED)
            {
                vkGetDeviceQueue(device, queueFamilyIndices.familyIndices[i], queueFamilyIndices.queueIndices[i], &queues[i].queue);

                queueFamilyIndices.counts[i] = queueFamilyIndices.queueOffsets[queueFamilyIndices.familyIndices[i]];

                for (uint32_t j = 0; j < VGPU_MAX_INFLIGHT_FRAMES; ++j)
                {
                    VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &queues[i].frameFences[j]));
                }
            }
            else
            {
                queues[i].queue = VK_NULL_HANDLE;
            }
        }

#ifdef _DEBUG
        vgpuLogInfo("Enabled %d Device Extensions:", createInfo.enabledExtensionCount);
        for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
        {
            vgpuLogInfo("	\t%s", createInfo.ppEnabledExtensionNames[i]);
    }
#endif
        if (desc->label)
        {
            SetObjectName(VK_OBJECT_TYPE_DEVICE, reinterpret_cast<uint64_t>(device), desc->label);
        }

        if (properties2.properties.apiVersion >= VK_API_VERSION_1_3)
        {
            driverDescription = properties_1_2.driverName;
            if (properties_1_2.driverInfo[0] != '\0')
            {
                driverDescription += std::string(": ") + properties_1_2.driverInfo;
            }
        }
        else
        {
            driverDescription = driverProperties.driverName;
            if (driverProperties.driverInfo[0] != '\0')
            {
                //driverDescription += std::to_string(": ") + driverProperties.driverInfo;
            }
        }
}

    // Create memory allocator
    {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;
        allocatorInfo.instance = instance;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

        // Core in 1.1
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT | VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
        if (supportedExtensions.memoryBudget)
        {
            allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        }

        if (supportedExtensions.AMD_device_coherent_memory)
        {
            allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
        }

        if (features1_2.bufferDeviceAddress == VK_TRUE)
        {
            allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        }

        if (supportedExtensions.memory_priority)
        {
            allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
        }

        // Core in 1.3
        if (properties2.properties.apiVersion < VK_API_VERSION_1_3)
        {
            if (supportedExtensions.maintenance4)
            {
                allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
            }
        }

        if (supportedExtensions.maintenance5)
        {
            allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
        }

#if VMA_DYNAMIC_VULKAN_FUNCTIONS
        static VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        allocatorInfo.pVulkanFunctions = &vulkanFunctions;
#endif

        result = vmaCreateAllocator(&allocatorInfo, &allocator);

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Cannot create allocator");
            return false;
        }

        if (supportedExtensions.externalMemory)
        {
            std::vector<VkExternalMemoryHandleTypeFlags> externalMemoryHandleTypes;
#if defined(_WIN32)
            externalMemoryHandleTypes.resize(memoryProperties2.memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT);
#else
            externalMemoryHandleTypes.resize(memoryProperties2.memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
#endif

            allocatorInfo.pTypeExternalMemoryHandleTypes = externalMemoryHandleTypes.data();
            result = vmaCreateAllocator(&allocatorInfo, &externalAllocator);
            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Failed to create Vulkan external memory allocator");
            }
        }

    }

    // Create default null descriptors.
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = 4;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.flags = 0;
        VmaAllocationCreateInfo bufferAllocInfo = {};
        bufferAllocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        result = vmaCreateBuffer(allocator, &bufferInfo, &bufferAllocInfo, &nullBuffer, &nullBufferAllocation, nullptr);
        VGPU_ASSERT(result == VK_SUCCESS);

        VkBufferViewCreateInfo bufferViewInfo = {};
        bufferViewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        bufferViewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        bufferViewInfo.range = VK_WHOLE_SIZE;
        bufferViewInfo.buffer = nullBuffer;
        result = vkCreateBufferView(device, &bufferViewInfo, nullptr, &nullBufferView);
        VGPU_ASSERT(result == VK_SUCCESS);
    }
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.extent.width = 1;
        imageInfo.extent.height = 1;
        imageInfo.extent.depth = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.arrayLayers = 1;
        imageInfo.mipLevels = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        imageInfo.flags = 0;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        imageInfo.imageType = VK_IMAGE_TYPE_1D;
        result = vmaCreateImage(allocator, &imageInfo, &allocInfo, &nullImage1D, &nullImageAllocation1D, nullptr);
        VGPU_ASSERT(result == VK_SUCCESS);

        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageInfo.arrayLayers = 6;
        result = vmaCreateImage(allocator, &imageInfo, &allocInfo, &nullImage2D, &nullImageAllocation2D, nullptr);
        VGPU_ASSERT(result == VK_SUCCESS);

        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        imageInfo.flags = 0;
        imageInfo.arrayLayers = 1;
        result = vmaCreateImage(allocator, &imageInfo, &allocInfo, &nullImage3D, &nullImageAllocation3D, nullptr);
        VGPU_ASSERT(result == VK_SUCCESS);

        // Transitions
        {
            VulkanUploadContext  uploadContext = Allocate(0);
            if (synchronization2)
            {
                VkImageMemoryBarrier2 barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                barrier.oldLayout = imageInfo.initialLayout;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = nullImage1D;
                barrier.subresourceRange.layerCount = 1;

                VkDependencyInfo dependencyInfo = {};
                dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                dependencyInfo.imageMemoryBarrierCount = 1;
                dependencyInfo.pImageMemoryBarriers = &barrier;
                vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);

                barrier.image = nullImage2D;
                barrier.subresourceRange.layerCount = 6;
                vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);
                barrier.image = nullImage3D;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);
            }
            else
            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = imageInfo.initialLayout;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = nullImage1D;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;

                vkCmdPipelineBarrier(uploadContext.transitionCommandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                barrier.image = nullImage2D;
                barrier.subresourceRange.layerCount = 6;
                vkCmdPipelineBarrier(uploadContext.transitionCommandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                barrier.image = nullImage3D;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(uploadContext.transitionCommandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);
            }

            UploadSubmit(uploadContext);
        }

        VkImageViewCreateInfo imageViewInfo = {};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
        imageViewInfo.image = nullImage1D;
        imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        result = vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView1D);
        VGPU_ASSERT(result == VK_SUCCESS);

        imageViewInfo.image = nullImage1D;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        result = vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView1DArray);
        VGPU_ASSERT(result == VK_SUCCESS);

        imageViewInfo.image = nullImage2D;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        result = vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView2D);
        VGPU_ASSERT(result == VK_SUCCESS);

        imageViewInfo.image = nullImage2D;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        result = vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView2DArray);
        VGPU_ASSERT(result == VK_SUCCESS);

        imageViewInfo.image = nullImage2D;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        imageViewInfo.subresourceRange.layerCount = 6;
        result = vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageViewCube);
        VGPU_ASSERT(result == VK_SUCCESS);

        imageViewInfo.image = nullImage2D;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        imageViewInfo.subresourceRange.layerCount = 6;
        result = vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageViewCubeArray);
        VGPU_ASSERT(result == VK_SUCCESS);

        imageViewInfo.image = nullImage3D;
        imageViewInfo.subresourceRange.layerCount = 1;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        result = vkCreateImageView(device, &imageViewInfo, nullptr, &nullImageView3D);
        VGPU_ASSERT(result == VK_SUCCESS);
    }
    {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        result = vkCreateSampler(device, &samplerInfo, nullptr, &nullSampler);
        VGPU_ASSERT(result == VK_SUCCESS);
    }

    // Allocate at least one descriptor pool.
    descriptorSetPools.emplace_back(CreateDescriptorSetPool());

    // Dynamic PSO states:
    psoDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    psoDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    psoDynamicStates.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    psoDynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
    if (features2.features.depthBounds == VK_TRUE)
    {
        psoDynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
    }
    if (fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
    {
        psoDynamicStates.push_back(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
    }

    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = (uint32_t)psoDynamicStates.size();
    dynamicStateInfo.pDynamicStates = psoDynamicStates.data();

    // Init caps
    timestampFrequency = uint64_t(1.0 / double(properties2.properties.limits.timestampPeriod) * 1000 * 1000 * 1000);

    // Log some info
    vgpuLogInfo("VGPU Driver: Vulkan");
    vgpuLogInfo("Vulkan Adapter: %s", properties2.properties.deviceName);

    return true;
}

void VulkanDevice::SetLabel(const char* label)
{
    SetObjectName(VK_OBJECT_TYPE_DEVICE, reinterpret_cast<uint64_t>(device), label);
}

void VulkanDevice::WaitIdle()
{
    VK_CHECK(vkDeviceWaitIdle(device));
}

VGPUBool32 VulkanDevice::QueryFeatureSupport(VGPUFeature feature) const
{
    switch (feature)
    {
        case VGPUFeature_Depth32FloatStencil8:
            return IsDepthStencilFormatSupported(physicalDevice, VK_FORMAT_D32_SFLOAT_S8_UINT);

        case VGPUFeature_TimestampQuery:
            return properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE;

        case VGPUFeature_PipelineStatisticsQuery:
            return features2.features.pipelineStatisticsQuery == VK_TRUE;

        case VGPUFeature_TextureCompressionBC:
            return features2.features.textureCompressionBC;

        case VGPUFeature_TextureCompressionETC2:
            return features2.features.textureCompressionETC2;

        case VGPUFeature_TextureCompressionASTC:
            return features2.features.textureCompressionASTC_LDR;

        case VGPUFeature_IndirectFirstInstance:
            return features2.features.drawIndirectFirstInstance == VK_TRUE;

        case VGPUFeature_ShaderFloat16:
            // VK_KHR_16bit_storage core in 1.1
            // VK_KHR_shader_float16_int8 core in 1.2
            return features1_2.shaderFloat16 == VK_TRUE;

        case VGPUFeature_CacheCoherentUMA:
            if (memoryProperties2.memoryProperties.memoryHeapCount == 1 &&
                memoryProperties2.memoryProperties.memoryHeaps[0].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                return true;
            }

            return false;

        case VGPUFeature_GeometryShader:
            return features2.features.geometryShader == VK_TRUE;

        case VGPUFeature_TessellationShader:
            return features2.features.tessellationShader == VK_TRUE;

        case VGPUFeature_DepthBoundsTest:
            return features2.features.depthBounds == VK_TRUE;

        case VGPUFeature_SamplerClampToBorder:
            return true;

        case VGPUFeature_SamplerMirrorClampToEdge:
            return features1_2.samplerMirrorClampToEdge == VK_TRUE;

        case VGPUFeature_SamplerMinMax:
            return features1_2.samplerFilterMinmax == VK_TRUE;

        case VGPUFeature_DepthResolveMinMax:
            return
                (depthStencilResolveProperties.supportedDepthResolveModes & VK_RESOLVE_MODE_MIN_BIT) &&
                (depthStencilResolveProperties.supportedDepthResolveModes & VK_RESOLVE_MODE_MAX_BIT);

        case VGPUFeature_StencilResolveMinMax:
            return
                (depthStencilResolveProperties.supportedStencilResolveModes & VK_RESOLVE_MODE_MIN_BIT) &&
                (depthStencilResolveProperties.supportedStencilResolveModes & VK_RESOLVE_MODE_MAX_BIT);

        case VGPUFeature_ShaderOutputViewportIndex:
            return features1_2.shaderOutputLayer == VK_TRUE && features1_2.shaderOutputViewportIndex == VK_TRUE;

        case VGPUFeature_ConservativeRasterization:
            return supportedExtensions.conservativeRasterization;

        case VGPUFeature_DescriptorIndexing:
            //VGPU_ASSERT(features1_2.descriptorIndexing == VK_TRUE);
            //VGPU_ASSERT(features1_2.runtimeDescriptorArray == VK_TRUE);
            //VGPU_ASSERT(features1_2.descriptorBindingPartiallyBound == VK_TRUE);
            //VGPU_ASSERT(features1_2.descriptorBindingVariableDescriptorCount == VK_TRUE);
            //VGPU_ASSERT(features1_2.shaderSampledImageArrayNonUniformIndexing == VK_TRUE);
            return features1_2.descriptorIndexing == VK_TRUE;

        case VGPUFeature_Predication:
            return conditionalRenderingFeatures.conditionalRendering == VK_TRUE;

        case VGPUFeature_VariableRateShading:
            return (fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE);

        case VGPUFeature_VariableRateShadingTier2:
            return (fragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE);

        case VGPUFeature_RayTracing:
            if (features1_2.bufferDeviceAddress == VK_TRUE &&
                acceleration_structure_features.accelerationStructure == VK_TRUE &&
                raytracing_features.rayTracingPipeline == VK_TRUE
                )
            {
                return true;
            }

            return false;

        case VGPUFeature_RayTracingTier2:
            return QueryFeatureSupport(VGPUFeature_RayTracing) & (rayQueryFeatures.rayQuery == VK_TRUE);


        case VGPUFeature_MeshShader:
            return meshShaderFeatures.meshShader == VK_TRUE && meshShaderFeatures.taskShader == VK_TRUE;


        default:
            return false;
    }
}

void VulkanDevice::GetAdapterProperties(VGPUAdapterProperties* properties) const
{
    properties->vendorId = properties2.properties.vendorID;
    properties->deviceId = properties2.properties.deviceID;
    strncpy(properties->name, properties2.properties.deviceName, _VGPU_MIN(VGPU_ADAPTER_NAME_MAX_LENGTH, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE));
    properties->driverDescription = driverDescription.c_str();

    switch (properties2.properties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            properties->type = VGPUAdapterType_IntegratedGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            properties->type = VGPUAdapterType_DiscreteGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            properties->type = VGPUAdapterType_Unknown;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            properties->type = VGPUAdapterType_CPU;
            break;
        default:
            properties->type = VGPUAdapterType_Unknown;
            break;
    }
}

void VulkanDevice::GetLimits(VGPULimits* limits) const
{
#define SET_LIMIT_FROM_VULKAN(vulkanName, name) limits->name = properties2.properties.limits.vulkanName
    SET_LIMIT_FROM_VULKAN(maxImageDimension1D, maxTextureDimension1D);
    SET_LIMIT_FROM_VULKAN(maxImageDimension2D, maxTextureDimension2D);
    SET_LIMIT_FROM_VULKAN(maxImageDimension3D, maxTextureDimension3D);
    SET_LIMIT_FROM_VULKAN(maxImageDimensionCube, maxTextureDimensionCube);
    SET_LIMIT_FROM_VULKAN(maxImageArrayLayers, maxTextureArrayLayers);
    //SET_LIMIT_FROM_VULKAN(maxBoundDescriptorSets, maxBindGroups);
    //SET_LIMIT_FROM_VULKAN(maxDescriptorSetUniformBuffersDynamic, maxDynamicUniformBuffersPerPipelineLayout);
    //SET_LIMIT_FROM_VULKAN(maxDescriptorSetStorageBuffersDynamic, maxDynamicStorageBuffersPerPipelineLayout);
    //SET_LIMIT_FROM_VULKAN(maxPerStageDescriptorSampledImages, maxSampledTexturesPerShaderStage);
    //SET_LIMIT_FROM_VULKAN(maxPerStageDescriptorSamplers, maxSamplersPerShaderStage);
    //SET_LIMIT_FROM_VULKAN(maxPerStageDescriptorStorageBuffers, maxStorageBuffersPerShaderStage);
    //SET_LIMIT_FROM_VULKAN(maxPerStageDescriptorStorageImages, maxStorageTexturesPerShaderStage);
    //SET_LIMIT_FROM_VULKAN(maxPerStageDescriptorUniformBuffers, maxUniformBuffersPerShaderStage);

    limits->maxConstantBufferBindingSize = properties2.properties.limits.maxUniformBufferRange;
    limits->maxStorageBufferBindingSize = properties2.properties.limits.maxStorageBufferRange;
    limits->minUniformBufferOffsetAlignment = (uint32_t)properties2.properties.limits.minUniformBufferOffsetAlignment;
    limits->minStorageBufferOffsetAlignment = (uint32_t)properties2.properties.limits.minStorageBufferOffsetAlignment;

    limits->maxVertexBuffers = properties2.properties.limits.maxVertexInputBindings;
    limits->maxVertexAttributes = properties2.properties.limits.maxVertexInputAttributes;

    limits->maxVertexBufferArrayStride = _VGPU_MIN(
        properties2.properties.limits.maxVertexInputBindingStride,
        properties2.properties.limits.maxVertexInputAttributeOffset + 1);

    SET_LIMIT_FROM_VULKAN(maxComputeSharedMemorySize, maxComputeWorkgroupStorageSize);
    SET_LIMIT_FROM_VULKAN(maxComputeWorkGroupInvocations, maxComputeInvocationsPerWorkGroup);
    SET_LIMIT_FROM_VULKAN(maxComputeWorkGroupSize[0], maxComputeWorkGroupSizeX);
    SET_LIMIT_FROM_VULKAN(maxComputeWorkGroupSize[1], maxComputeWorkGroupSizeY);
    SET_LIMIT_FROM_VULKAN(maxComputeWorkGroupSize[2], maxComputeWorkGroupSizeZ);
    SET_LIMIT_FROM_VULKAN(maxComputeWorkGroupSize[2], maxComputeWorkGroupsPerDimension);

    limits->maxComputeWorkGroupsPerDimension = _VGPU_MIN(
        _VGPU_MIN(
            properties2.properties.limits.maxComputeWorkGroupCount[0],
            properties2.properties.limits.maxComputeWorkGroupCount[1]),
        properties2.properties.limits.maxComputeWorkGroupCount[2]
    );

    limits->maxViewports = properties2.properties.limits.maxViewports;
    limits->maxViewportDimensions[0] = properties2.properties.limits.maxViewportDimensions[0];
    limits->maxViewportDimensions[1] = properties2.properties.limits.maxViewportDimensions[1];
    limits->maxColorAttachments = properties2.properties.limits.maxColorAttachments;

    if (QueryFeatureSupport(VGPUFeature_RayTracing))
    {
        limits->rayTracingShaderGroupIdentifierSize = rayTracingPipelineProperties.shaderGroupHandleSize;
        limits->rayTracingShaderTableAligment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
        limits->rayTracingShaderTableMaxStride = rayTracingPipelineProperties.maxShaderGroupStride;
        limits->rayTracingShaderRecursionMaxDepth = rayTracingPipelineProperties.maxRayRecursionDepth;
        limits->rayTracingMaxGeometryCount = (uint32_t)accelerationStructureProperties.maxGeometryCount;
    }

#undef SET_LIMIT_FROM_VULKAN
}

static void AddUniqueFamily(uint32_t* sharing_indices, uint32_t& count, uint32_t family)
{
    if (family == VK_QUEUE_FAMILY_IGNORED)
        return;

    for (uint32_t i = 0; i < count; i++)
    {
        if (sharing_indices[i] == family)
            return;
    }

    sharing_indices[count++] = family;
}

/* Buffer */
VGPUBuffer VulkanDevice::CreateBuffer(const VGPUBufferDesc* desc, const void* pInitialData)
{
    if (desc->existingHandle)
    {
        VulkanBuffer* buffer = new VulkanBuffer();
        buffer->renderer = this;
        buffer->size = desc->size;
        buffer->usage = desc->usage;
        buffer->handle = reinterpret_cast<VkBuffer>(desc->existingHandle);
        buffer->allocation = VK_NULL_HANDLE;
        buffer->allocatedSize = 0u;
        buffer->gpuAddress = 0;

        if (desc->label)
        {
            SetObjectName(VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>(buffer->handle), desc->label);
        }

        return buffer;
    }

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc->size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    bool needBufferDeviceAddress = false;
    if (desc->usage & VGPUBufferUsage_Vertex)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        needBufferDeviceAddress = true;
    }
    if (desc->usage & VGPUBufferUsage_Index)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        needBufferDeviceAddress = true;
    }

    if (desc->usage & VGPUBufferUsage_Constant)
    {
        bufferInfo.size = VmaAlignUp(bufferInfo.size, properties2.properties.limits.minUniformBufferOffsetAlignment);
        bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if (desc->usage & VGPUBufferUsage_ShaderRead)
    {
        // ReadOnly ByteAddressBuffer is also storage buffer
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }

    if (desc->usage & VGPUBufferUsage_ShaderWrite)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }

    if (desc->usage & VGPUBufferUsage_Indirect)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        needBufferDeviceAddress = true;
    }

    // We check for feature in vgpuCreateBuffer
    if (desc->usage & VGPUBufferUsage_Predication)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
    }

    if (desc->usage & VGPUBufferUsage_RayTracing)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
        bufferInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
        bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
        needBufferDeviceAddress = true;
    }

    if (features1_2.bufferDeviceAddress == VK_TRUE && needBufferDeviceAddress)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    uint32_t sharingIndices[3] = {};
    for (auto& i : queueFamilyIndices.familyIndices)
    {
        AddUniqueFamily(sharingIndices, bufferInfo.queueFamilyIndexCount, i);
    }

    if (bufferInfo.queueFamilyIndexCount > 1)
    {
        // For buffers, always just use CONCURRENT access modes,
        // so we don't have to deal with acquire/release barriers in async compute.
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

        bufferInfo.pQueueFamilyIndices = sharingIndices;
    }
    else
    {
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 0;
        bufferInfo.pQueueFamilyIndices = nullptr;
    }

    VmaAllocationCreateInfo memoryInfo = {};
    memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
    if (desc->cpuAccess == VGPUCpuAccessMode_Read)
    {
        memoryInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }
    else if (desc->cpuAccess == VGPUCpuAccessMode_Write)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        memoryInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VmaAllocationInfo allocationInfo{};
    VulkanBuffer* buffer = new VulkanBuffer();
    buffer->renderer = this;
    VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &memoryInfo,
        &buffer->handle,
        &buffer->allocation,
        &allocationInfo);

    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Failed to create buffer.");
        return nullptr;
    }

    buffer->size = desc->size;
    buffer->usage = desc->usage;

    if (desc->label)
    {
        buffer->SetLabel(desc->label);
    }

    if (memoryInfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)
    {
        buffer->pMappedData = allocationInfo.pMappedData;
    }

    if (bufferInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        VkBufferDeviceAddressInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.buffer = buffer->handle;
        buffer->gpuAddress = vkGetBufferDeviceAddress(device, &info);
    }

    // Issue data copy.
    if (pInitialData != nullptr)
    {
        VulkanUploadContext uploadContext;
        void* pMappedData = nullptr;
        if (desc->cpuAccess == VGPUCpuAccessMode_Write)
        {
            pMappedData = buffer->pMappedData;
        }
        else
        {
            uploadContext = Allocate(desc->size);
            pMappedData = uploadContext.uploadBufferData;
        }

        memcpy(pMappedData, pInitialData, desc->size);

        if (uploadContext.IsValid())
        {
            VkBufferCopy copyRegion = {};
            copyRegion.size = buffer->size;
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;

            vkCmdCopyBuffer(
                uploadContext.transferCommandBuffer,
                uploadContext.uploadBuffer->handle,
                buffer->handle,
                1,
                &copyRegion
            );

            if (synchronization2)
            {
                VkBufferMemoryBarrier2 barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
                barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.buffer = buffer->handle;
                barrier.size = VK_WHOLE_SIZE;

                if (desc->usage & VGPUBufferUsage_Vertex)
                {
                    barrier.dstStageMask |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
                    barrier.dstAccessMask |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
                }

                if (desc->usage & VGPUBufferUsage_Index)
                {
                    barrier.dstStageMask |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
                    barrier.dstAccessMask |= VK_ACCESS_2_INDEX_READ_BIT;
                }

                if (desc->usage & VGPUBufferUsage_Constant)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_UNIFORM_READ_BIT;
                }

                if (desc->usage & VGPUBufferUsage_ShaderRead)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT;
                }

                if (desc->usage & VGPUBufferUsage_ShaderWrite)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT;
                    barrier.dstAccessMask |= VK_ACCESS_2_SHADER_WRITE_BIT;
                }

                if (desc->usage & VGPUBufferUsage_Indirect)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
                }

                if (desc->usage & VGPUBufferUsage_Predication)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;
                }

                if (desc->usage & VGPUBufferUsage_RayTracing)
                {
                    barrier.dstAccessMask |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
                }

                //if (CheckBitsAny(desc.usage, BufferUsage::VideoDecode))
                //{
                //    barrier.dstAccessMask |= VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR;
                //}


                VkDependencyInfo dependencyInfo = {};
                dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                dependencyInfo.bufferMemoryBarrierCount = 1;
                dependencyInfo.pBufferMemoryBarriers = &barrier;

                vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);
            }
            else
            {
                VkBufferMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                barrier.buffer = buffer->handle;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.offset = 0;
                barrier.size = VK_WHOLE_SIZE;

                vkCmdPipelineBarrier(
                    uploadContext.transferCommandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    1, &barrier,
                    0, nullptr
                );

                std::swap(barrier.srcAccessMask, barrier.dstAccessMask);

                if (desc->usage & VGPUBufferUsage_Vertex)
                {
                    barrier.dstAccessMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                }
                if (desc->usage & VGPUBufferUsage_Index)
                {
                    barrier.dstAccessMask |= VK_ACCESS_INDEX_READ_BIT;
                }
                if (desc->usage & VGPUBufferUsage_Constant)
                {
                    barrier.dstAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
                }
                if (desc->usage & VGPUBufferUsage_ShaderRead)
                {
                    barrier.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
                }
                if (desc->usage & VGPUBufferUsage_ShaderWrite)
                {
                    barrier.dstAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
                }
                if (desc->usage & VGPUBufferUsage_Indirect)
                {
                    barrier.dstAccessMask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
                }
                if (desc->usage & VGPUBufferUsage_Predication)
                {
                    barrier.dstAccessMask |= VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT;
                }
                if (desc->usage & VGPUBufferUsage_RayTracing)
                {
                    barrier.dstAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
                }

                vkCmdPipelineBarrier(
                    uploadContext.transitionCommandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    1, &barrier,
                    0, nullptr
                );
            }

            UploadSubmit(uploadContext);
        }
    }

    return buffer;
}

/* Texture */
VGPUTexture VulkanDevice::CreateTexture(const VGPUTextureDesc* desc, const VGPUTextureData* pInitialData)
{
    const bool isDepthStencilFormat = vgpuIsDepthStencilFormat(desc->format);

    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.format = ToVkFormat(desc->format);
    createInfo.extent.width = desc->width;

    switch (desc->dimension)
    {
        case VGPUTextureDimension_1D:
            createInfo.imageType = VK_IMAGE_TYPE_1D;
            createInfo.extent.height = 1;
            createInfo.extent.depth = 1;
            createInfo.arrayLayers = desc->depthOrArrayLayers;
            break;

        case VGPUTextureDimension_2D:
            createInfo.imageType = VK_IMAGE_TYPE_2D;
            createInfo.extent.height = desc->height;
            createInfo.extent.depth = 1;
            createInfo.arrayLayers = desc->depthOrArrayLayers;
            createInfo.samples = (VkSampleCountFlagBits)desc->sampleCount;

            if (createInfo.extent.width == createInfo.extent.height &&
                createInfo.arrayLayers >= 6)
            {
                createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            }
            break;

        case VGPUTextureDimension_3D:
            createInfo.imageType = VK_IMAGE_TYPE_3D;
            createInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
            createInfo.extent.height = desc->height;
            createInfo.extent.depth = desc->depthOrArrayLayers;
            createInfo.arrayLayers = 1;
            createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            break;

        default:
            return nullptr;
    }

    createInfo.mipLevels = desc->mipLevelCount;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    if (desc->usage & VGPUTextureUsage_Transient)
    {
        createInfo.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    }
    else
    {
        createInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        createInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (desc->usage & VGPUTextureUsage_ShaderRead)
    {
        createInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (desc->usage & VGPUTextureUsage_ShaderWrite)
    {
        createInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if (desc->usage & VGPUTextureUsage_RenderTarget)
    {
        if (isDepthStencilFormat)
        {
            createInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        else
        {
            createInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }

    if (desc->usage & VGPUTextureUsage_ShadingRate)
    {
        createInfo.usage |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    }

    // If ShaderRead and RenderTarget add input attachment
    if (!isDepthStencilFormat &&
        (desc->usage & (VGPUTextureUsage_RenderTarget | VGPUTextureUsage_ShaderRead)))
    {
        createInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    uint32_t sharingIndices[3] = {};
    for (auto& i : queueFamilyIndices.familyIndices)
    {
        AddUniqueFamily(sharingIndices, createInfo.queueFamilyIndexCount, i);
    }

    if (createInfo.queueFamilyIndexCount > 1)
    {
        // For buffers, always just use CONCURRENT access modes,
        // so we don't have to deal with acquire/release barriers in async compute.
        createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

        createInfo.pQueueFamilyIndices = sharingIndices;
    }
    else
    {
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    VmaAllocationCreateInfo memoryInfo = {};
    memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
    bool isShared = false;
    VkExternalMemoryImageCreateInfo externalInfo = {};
    if (desc->usage & VGPUTextureUsage_Shared)
    {
        // Ensure that the handle type is supported.
        VkImageFormatProperties2 props2 = {};
        props2.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
        VkExternalImageFormatProperties externalProps = {};
        externalProps.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;

        VkPhysicalDeviceExternalImageFormatInfo externalFormatInfo = {};
        externalFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
#if defined(_WIN32)
        externalFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
        externalFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

        props2.pNext = &externalProps;
        if (!GetImageFormatProperties(createInfo, &externalFormatInfo, &props2))
        {
            vgpuLogError("Image format is not supported for external memory type %u.", (uint32_t)externalFormatInfo.handleType);
            return nullptr;
        }

        externalInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
#if defined(_WIN32)
        externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
        externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

        const bool supportsImport = (externalProps.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0;
        const bool supportsExport = (externalProps.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) != 0;

        VGPU_UNUSED(supportsImport);
        VGPU_UNUSED(supportsExport);

        createInfo.pNext = &externalInfo;

        // We have to use a dedicated allocator for external handles that has been created with VkExportMemoryAllocateInfo
        allocator = externalAllocator;

        // Dedicated memory
        memoryInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        isShared = true;
    }

    VmaAllocationInfo allocationInfo{};

    if (desc->cpuAccess == VGPUCpuAccessMode_Write
        || desc->cpuAccess == VGPUCpuAccessMode_Read)
    {
        // TODO: Handle readback texture
    }

    VulkanTexture* texture = new VulkanTexture();
    texture->renderer = this;
    texture->dimension = desc->dimension;
    texture->format = desc->format;
    texture->width = createInfo.extent.width;
    texture->height = createInfo.extent.height;
    texture->vkFormat = createInfo.format;

    VkResult result = vmaCreateImage(allocator,
        &createInfo, &memoryInfo,
        &texture->handle,
        &texture->allocation,
        &allocationInfo);

    if (result != VK_SUCCESS)
    {
        vgpuLogError("Vulkan: Failed to create texture");
        delete texture;
        return nullptr;
    }

    if (desc->label)
    {
        texture->SetLabel(desc->label);
    }

    if (isShared)
    {
#if defined(_WIN32)
        VkMemoryGetWin32HandleInfoKHR getWin32HandleInfoKHR = {};
        getWin32HandleInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
        getWin32HandleInfoKHR.pNext = nullptr;
        getWin32HandleInfoKHR.memory = allocationInfo.deviceMemory;
        getWin32HandleInfoKHR.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
        VK_CHECK(vkGetMemoryWin32HandleKHR(device, &getWin32HandleInfoKHR, &texture->sharedHandle));
#else
        VkMemoryGetFdInfoKHR memoryGetFdInfoKHR = {};
        memoryGetFdInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
        memoryGetFdInfoKHR.pNext = nullptr;
        memoryGetFdInfoKHR.memory = allocationInfo.deviceMemory;
        memoryGetFdInfoKHR.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
        int sharedHandle;
        VK_CHECK(vkGetMemoryFdKHR(device, &memoryGetFdInfoKHR, &sharedHandle));
        texture->sharedHandle = (void*)(size_t)sharedHandle;
#endif
    }

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = GetImageAspectFlags(createInfo.format);
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = createInfo.mipLevels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = createInfo.arrayLayers;

    if (pInitialData != nullptr)
    {
        VulkanUploadContext uploadContext;
        void* pMappedData = nullptr;
        if (desc->cpuAccess == VGPUCpuAccessMode_Write)
        {
            //pMappedData = texture->pMappedData;
        }
        else
        {
            uploadContext = Allocate(allocationInfo.size);
            pMappedData = uploadContext.uploadBuffer->pMappedData;
        }
        VGPU_UNUSED(pMappedData);

        std::vector<VkBufferImageCopy> copyRegions;

        VGPUPixelFormatInfo formatInfo;
        vgpuGetPixelFormatInfo(desc->format, &formatInfo);
        const uint32_t blockSize = formatInfo.blockWidth;

        VkDeviceSize copyOffset = 0;
        uint32_t initDataIndex = 0;
        for (uint32_t arrayIndex = 0; arrayIndex < createInfo.arrayLayers; ++arrayIndex)
        {
            uint32_t levelWidth = createInfo.extent.width;
            uint32_t levelHeight = createInfo.extent.height;
            uint32_t levelDepth = createInfo.extent.depth;

            for (uint32_t mipIndex = 0; mipIndex < createInfo.mipLevels; ++mipIndex)
            {
                const VGPUTextureData& subresourceData = pInitialData[initDataIndex++];
                const uint32_t numBlocksX = _VGPU_MAX(1u, levelWidth / blockSize);
                const uint32_t numBlocksY = _VGPU_MAX(1u, levelHeight / blockSize);
                const uint32_t dstRowPitch = numBlocksX * formatInfo.bytesPerBlock;
                const uint32_t dstSlicePitch = dstRowPitch * numBlocksY;

                uint32_t srcRowPitch = subresourceData.rowPitch;
                uint32_t srcSlicePitch = subresourceData.slicePitch;
                //GetSurfaceInfo(desc.format, levelWidth, levelHeight, &srcRowPitch, &srcSlicePitch);

                for (uint32_t z = 0; z < levelDepth; ++z)
                {
                    uint8_t* dstSlice = (uint8_t*)uploadContext.uploadBufferData + copyOffset + dstSlicePitch * z;
                    uint8_t* srcSlice = (uint8_t*)subresourceData.pData + srcSlicePitch * z;
                    for (uint32_t y = 0; y < numBlocksY; ++y)
                    {
                        std::memcpy(
                            dstSlice + dstRowPitch * y,
                            srcSlice + srcRowPitch * y,
                            dstRowPitch
                        );
                    }
                }

                if (uploadContext.IsValid())
                {
                    VkBufferImageCopy copyRegion = {};
                    copyRegion.bufferOffset = copyOffset;
                    copyRegion.bufferRowLength = 0;
                    copyRegion.bufferImageHeight = 0;

                    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    copyRegion.imageSubresource.mipLevel = mipIndex;
                    copyRegion.imageSubresource.baseArrayLayer = arrayIndex;
                    copyRegion.imageSubresource.layerCount = 1;

                    copyRegion.imageOffset = { 0, 0, 0 };
                    copyRegion.imageExtent.width = levelWidth;
                    copyRegion.imageExtent.height = levelHeight;
                    copyRegion.imageExtent.depth = levelDepth;

                    copyRegions.push_back(copyRegion);
                }

                copyOffset += dstSlicePitch * levelDepth;

                levelWidth = _VGPU_MAX(1u, levelWidth / 2);
                levelHeight = _VGPU_MAX(1u, levelHeight / 2);
                levelDepth = _VGPU_MAX(1u, levelDepth / 2);
            }
        }

        if (uploadContext.IsValid())
        {
            if (synchronization2)
            {
                VkImageMemoryBarrier2 barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                barrier.srcAccessMask = 0;
                barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = texture->handle;
                barrier.subresourceRange = subresourceRange;

                VkDependencyInfo dependencyInfo = {};
                dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                dependencyInfo.imageMemoryBarrierCount = 1;
                dependencyInfo.pImageMemoryBarriers = &barrier;
                vkCmdPipelineBarrier2(uploadContext.transferCommandBuffer, &dependencyInfo);

                vkCmdCopyBufferToImage(
                    uploadContext.transferCommandBuffer,
                    uploadContext.uploadBuffer->handle,
                    texture->handle,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    (uint32_t)copyRegions.size(),
                    copyRegions.data()
                );

                //const ResourceStateMapping2 mappingAfter = ConvertResourceState2(initialState);
                //ALIMER_ASSERT(mappingAfter.imageLayout != VK_IMAGE_LAYOUT_UNDEFINED);
                //
                //std::swap(barrier.srcStageMask, barrier.dstStageMask);
                //barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                //barrier.newLayout = mappingAfter.imageLayout;
                //barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                //barrier.dstAccessMask = mappingAfter.accessMask;
                //
                //vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);
            }
            else
            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.oldLayout = createInfo.initialLayout;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = texture->handle;
                barrier.subresourceRange = subresourceRange;

                vkCmdPipelineBarrier(uploadContext.transferCommandBuffer,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );

                vkCmdCopyBufferToImage(
                    uploadContext.transferCommandBuffer,
                    uploadContext.uploadBuffer->handle,
                    texture->handle,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    (uint32_t)copyRegions.size(),
                    copyRegions.data()
                );

                //const ResourceStateMapping mappingAfter = ConvertResourceState(initialState);
                //ALIMER_ASSERT(mappingAfter.imageLayout != VK_IMAGE_LAYOUT_UNDEFINED);
                //
                //barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                //barrier.dstAccessMask = mappingAfter.accessMask;
                //barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                //barrier.newLayout = mappingAfter.imageLayout;
                //
                //vkCmdPipelineBarrier(uploadContext.transitionCommandBuffer,
                //    VK_PIPELINE_STAGE_TRANSFER_BIT,
                //    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                //    0,
                //    0, nullptr,
                //    0, nullptr,
                //    1, &barrier
                //);
            }

            UploadSubmit(uploadContext);
        }
    }
    else
    {
        VulkanUploadContext uploadContext = Allocate(0);

        // Barrier
        if (synchronization2)
        {
            VkImageMemoryBarrier2 barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            barrier.srcAccessMask = 0;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            barrier.oldLayout = createInfo.initialLayout;
            barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = texture->handle;
            barrier.subresourceRange = subresourceRange;

            VkDependencyInfo dependencyInfo = {};
            dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &barrier;

            vkCmdPipelineBarrier2(uploadContext.transitionCommandBuffer, &dependencyInfo);
        }
        else
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = 0u;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            barrier.oldLayout = createInfo.initialLayout;
            barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = texture->handle;
            barrier.subresourceRange = subresourceRange;

            vkCmdPipelineBarrier(uploadContext.transitionCommandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
        }

        UploadSubmit(uploadContext);
    }

    return texture;
}

/* VulkanSampler */
VulkanSampler::~VulkanSampler()
{
    renderer->destroyMutex.lock();
    renderer->destroyedSamplers.push_back(std::make_pair(handle, renderer->frameCount));
    renderer->destroyMutex.unlock();
}

void VulkanSampler::SetLabel(const char* label)
{
    renderer->SetObjectName(VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(handle), label);
}

VGPUSampler VulkanDevice::CreateSampler(const VGPUSamplerDesc* desc)
{
    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = ToVkFilter(desc->magFilter);
    createInfo.minFilter = ToVkFilter(desc->minFilter);
    createInfo.mipmapMode = ToVkMipmapMode(desc->mipFilter);
    createInfo.addressModeU = ToVkSamplerAddressMode(desc->addressU);
    createInfo.addressModeV = ToVkSamplerAddressMode(desc->addressV);
    createInfo.addressModeW = ToVkSamplerAddressMode(desc->addressW);
    createInfo.mipLodBias = desc->mipLodBias;
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSamplerCreateInfo.html
    if (features2.features.samplerAnisotropy == VK_TRUE && desc->maxAnisotropy > 1)
    {
        createInfo.anisotropyEnable = VK_TRUE;
        createInfo.maxAnisotropy = _VGPU_MIN(float(desc->maxAnisotropy), properties2.properties.limits.maxSamplerAnisotropy);
    }
    else
    {
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.maxAnisotropy = 1;
    }

    if (desc->compareFunction != VGPUCompareFunction_Never)
    {
        createInfo.compareOp = ToVk(desc->compareFunction);
        createInfo.compareEnable = VK_TRUE;
    }
    else
    {
        createInfo.compareOp = VK_COMPARE_OP_NEVER;
        createInfo.compareEnable = VK_FALSE;
    }

    createInfo.minLod = desc->lodMinClamp;
    createInfo.maxLod = desc->lodMaxClamp;
    createInfo.borderColor = ToVkBorderColor(desc->borderColor);
    createInfo.unnormalizedCoordinates = VK_FALSE;

    VulkanSampler* sampler = new VulkanSampler();
    sampler->renderer = this;
    VkResult result = vkCreateSampler(device, &createInfo, nullptr, &sampler->handle);

    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Failed to create sampler.");
        delete sampler;
        return nullptr;
    }

    if (desc->label)
    {
        sampler->SetLabel(desc->label);
    }

    return sampler;
}

/* BindGroupLayout */
VulkanBindGroupLayout::~VulkanBindGroupLayout()
{
    device->destroyMutex.lock();
    device->destroyedDescriptorSetLayouts.push_back(std::make_pair(handle, device->frameCount));
    device->destroyMutex.unlock();
}

void VulkanBindGroupLayout::SetLabel(const char* label)
{
    device->SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, reinterpret_cast<uint64_t>(handle), label);
}

VGPUBindGroupLayout VulkanDevice::CreateBindGroupLayout(const VGPUBindGroupLayoutDesc* desc)
{
    const size_t bindingLayoutCount = desc->entryCount;

    VulkanBindGroupLayout* layout = new VulkanBindGroupLayout();
    layout->device = this;

    layout->layoutBindings.reserve(bindingLayoutCount);
    layout->layoutBindingsOriginal.reserve(bindingLayoutCount);

    std::vector<VkDescriptorBindingFlags> vkBindingFlags;
    vkBindingFlags.reserve(bindingLayoutCount);

    constexpr VkDescriptorBindingFlags bindlessBindingFlags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
        | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT
        | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
        //| VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
        ;

    for (size_t i = 0; i < bindingLayoutCount; ++i)
    {
        const VGPUBindGroupLayoutEntry& entry = desc->entries[i];

        VkDescriptorSetLayoutBinding layoutBinding = {};
        uint32_t registerOffset = 0;

        //if (entry.sampler.staticSampler != nullptr)
        //{
        //    registerOffset = kVulkanBindingShiftSampler;
        //    VkSampler sampler = GetOrCreateVulkanSampler(entry.sampler.staticSampler);
        //
        //    layoutBinding.binding = entry.binding + registerOffset;
        //    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        //    layoutBinding.descriptorCount = 1;
        //    layoutBinding.stageFlags = ToVkShaderStageFlags(entry.visibility);
        //    layoutBinding.pImmutableSamplers = &sampler;
        //    layout->layoutBindings.push_back(layoutBinding);
        //    layout->layoutBindingsOriginal.push_back(entry.binding);
        //    vkBindingFlags.push_back(bindlessBindingFlags);
        //    continue;
        //}

        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        switch (entry.descriptorType)
        {
            case VGPUDescriptorType_Sampler:
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                registerOffset = kVulkanBindingShiftSampler;
                break;

            case VGPUDescriptorType_SampledTexture:
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                registerOffset = kVulkanBindingShiftSRV;
                break;

            case VGPUDescriptorType_StorageTexture:
            case VGPUDescriptorType_ReadOnlyStorageTexture:
                registerOffset = kVulkanBindingShiftUAV;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;

            case VGPUDescriptorType_ConstantBuffer:
                registerOffset = kVulkanBindingShiftBuffer;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;

            case VGPUDescriptorType_DynamicConstantBuffer:
                registerOffset = kVulkanBindingShiftBuffer;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                break;

            case VGPUDescriptorType_StorageBuffer:
            case VGPUDescriptorType_ReadOnlyStorageBuffer:
                registerOffset = (entry.descriptorType == VGPUDescriptorType_ReadOnlyStorageBuffer) ? kVulkanBindingShiftSRV : kVulkanBindingShiftUAV;
                // UniformTexelBuffer, StorageTexelBuffer ?
                //if (entry.buffer.hasDynamicOffset)
                //{
                //    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                //}
                //else
                {
                    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                }
                break;

            default:
                VGPU_UNREACHABLE();
                break;
        }
        layoutBinding.binding = registerOffset + entry.binding;
        layoutBinding.descriptorCount = entry.count;
        layoutBinding.stageFlags = ToVkShaderStageFlags(entry.visibility);

        layout->layoutBindings.push_back(layoutBinding);
        layout->layoutBindingsOriginal.push_back(entry.binding);

        vkBindingFlags.push_back(bindlessBindingFlags);
    }

    //layout->isBindless = true;
    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = (uint32_t)layout->layoutBindings.size();
    createInfo.pBindings = layout->layoutBindings.data();

    // Bindless
    VkDescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlags = {};
    if (layout->isBindless)
    {
        setLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        setLayoutBindingFlags.bindingCount = static_cast<uint32_t>(vkBindingFlags.size());
        setLayoutBindingFlags.pBindingFlags = vkBindingFlags.data();

        createInfo.pNext = &setLayoutBindingFlags;
        createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    }

    VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout->handle);
    if (result != VK_SUCCESS)
    {
        delete layout;
        return nullptr;
    }

    if (desc->label)
    {
        layout->SetLabel(desc->label);
    }

    return layout;
}

/* PipelineLayout */
VulkanPipelineLayout::~VulkanPipelineLayout()
{
    device->destroyMutex.lock();
    device->destroyedPipelineLayouts.push_back(std::make_pair(handle, device->frameCount));
    device->destroyMutex.unlock();
}

void VulkanPipelineLayout::SetLabel(const char* label)
{
    device->SetObjectName(VK_OBJECT_TYPE_PIPELINE_LAYOUT, reinterpret_cast<uint64_t>(handle), label);
}

VGPUPipelineLayout VulkanDevice::CreatePipelineLayout(const VGPUPipelineLayoutDesc* descriptor)
{
    VulkanPipelineLayout* layout = new VulkanPipelineLayout();
    layout->device = this;

    layout->bindGroupLayoutCount = (uint32_t)descriptor->bindGroupLayoutCount;

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(descriptor->bindGroupLayoutCount);
    for (uint32_t i = 0; i < descriptor->bindGroupLayoutCount; i++)
    {
        descriptorSetLayouts[i] = static_cast<VulkanBindGroupLayout*>(descriptor->bindGroupLayouts[i])->handle;
    }

    // Push constants
    if (descriptor->pushConstantRangeCount > 0)
    {
        uint32_t offset = 0;
        layout->pushConstantRanges.resize(descriptor->pushConstantRangeCount);

        for (uint32_t i = 0; i < descriptor->pushConstantRangeCount; i++)
        {
            const VGPUPushConstantRange& pushConstantRange = descriptor->pushConstantRanges[i];

            VkPushConstantRange& range = layout->pushConstantRanges[i];
            range = {};
            range.stageFlags = ToVkShaderStageFlags(pushConstantRange.visibility);
            range.offset = offset;
            range.size = pushConstantRange.size;

            offset += pushConstantRange.size;
        }
    }

    // Create pipeline layout
    VkPipelineLayoutCreateInfo createInfo = { };
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
    createInfo.pSetLayouts = descriptorSetLayouts.data();
    createInfo.pushConstantRangeCount = descriptor->pushConstantRangeCount;
    createInfo.pPushConstantRanges = layout->pushConstantRanges.data();

    VkResult result = vkCreatePipelineLayout(device, &createInfo, nullptr, &layout->handle);
    if (result != VK_SUCCESS)
    {
        delete layout;
        return nullptr;
    }

    if (descriptor->label)
    {
        layout->SetLabel(descriptor->label);
    }

    return layout;
}

/* BindGroup */
VulkanBindGroup::~VulkanBindGroup()
{
    bindGroupLayout->Release();

    const uint64_t frameCount = device->frameCount;
    device->destroyMutex.lock();
    device->destroyedDescriptorSets.push_back(std::make_pair(std::make_pair(descriptorPool, descriptorSet), frameCount));
    device->destroyMutex.unlock();
}

void VulkanBindGroup::SetLabel(const char* label)
{
    device->SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, reinterpret_cast<uint64_t>(descriptorSet), label);
}

void VulkanBindGroup::Update(size_t entryCount, const VGPUBindGroupEntry* entries)
{
    // collect all of the descriptor write data
    const size_t layoutBindingCount = bindGroupLayout->layoutBindings.size();
    uint32_t descriptorWriteCount = 0;
    std::vector<VkWriteDescriptorSet> descriptorWrites(layoutBindingCount);
    std::vector<VkDescriptorImageInfo> descriptorImageInfo;
    std::vector<VkDescriptorBufferInfo> descriptorBufferInfo;
    //std::vector<VkWriteDescriptorSetAccelerationStructureKHR> accelStructWriteInfo;

    descriptorImageInfo.reserve(layoutBindingCount);
    descriptorBufferInfo.reserve(layoutBindingCount);

    // Generates a VkWriteDescriptorSet in descriptorWriteInfo
    auto generateWriteDescriptorData =
        [&](uint32_t bindingLocation,
            VkDescriptorType descriptorType,
            VkDescriptorImageInfo* imageInfo,
            VkDescriptorBufferInfo* bufferInfo,
            VkBufferView* bufferView,
            const void* pNext = nullptr)
        {
            descriptorWrites[descriptorWriteCount].pNext = pNext;
            descriptorWrites[descriptorWriteCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[descriptorWriteCount].dstSet = descriptorSet;
            descriptorWrites[descriptorWriteCount].dstBinding = bindingLocation;
            descriptorWrites[descriptorWriteCount].dstArrayElement = 0;
            descriptorWrites[descriptorWriteCount].descriptorCount = 1;
            descriptorWrites[descriptorWriteCount].descriptorType = descriptorType;
            descriptorWrites[descriptorWriteCount].pImageInfo = imageInfo;
            descriptorWrites[descriptorWriteCount].pBufferInfo = bufferInfo;
            descriptorWrites[descriptorWriteCount].pTexelBufferView = bufferView;

            descriptorWriteCount++;
        };

    // TODO: Handle null descriptors
    for (size_t bindingIndex = 0; bindingIndex < layoutBindingCount; ++bindingIndex)
    {
        const VkDescriptorSetLayoutBinding& layoutBinding = bindGroupLayout->layoutBindings[bindingIndex];

        if (layoutBinding.pImmutableSamplers != nullptr)
            continue;


        const VGPUBindGroupEntry& entry = entries[bindingIndex];

        //uint32_t registerOffset = VkGetRegisterOffset(layoutBinding.descriptorType);
        uint32_t originalBinding = bindGroupLayout->layoutBindingsOriginal[entry.binding]; // layoutBinding.binding - registerOffset;

        if (entry.binding != originalBinding)
            return;

        VkDescriptorType descriptorType = layoutBinding.descriptorType;

        switch (descriptorType)
        {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            {
                auto& samplerImageInfo = descriptorImageInfo.emplace_back();
                //samplerImageInfo.sampler = foundEntry != nullptr ? backendSampler->handle : nullSampler;

                VGPU_ASSERT(samplerImageInfo.sampler != VK_NULL_HANDLE);
                generateWriteDescriptorData(layoutBinding.binding,
                    layoutBinding.descriptorType,
                    &samplerImageInfo, nullptr, nullptr);
            }
            break;

            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            {
                VkDescriptorBufferInfo& bufferInfo = descriptorBufferInfo.emplace_back();
                if (entry.buffer != nullptr)
                {
                    bufferInfo.buffer = static_cast<VulkanBuffer*>(entry.buffer)->handle; // VkBuffer
                    bufferInfo.offset = _VGPU_MIN(entry.offset, entry.buffer->GetSize());
                    if (entry.size == VGPU_WHOLE_SIZE)
                    {
                        //bufferInfo.range = backendBuffer->GetSize() - bufferInfo.offset;
                        bufferInfo.range = VK_WHOLE_SIZE;
                    }
                    else
                    {
                        bufferInfo.range = _VGPU_MIN(entry.size, entry.buffer->GetSize() - bufferInfo.offset);
                    }
                }
                else
                {
                    bufferInfo.buffer = device->nullBuffer;
                    bufferInfo.range = VK_WHOLE_SIZE;
                }

                VGPU_ASSERT(bufferInfo.buffer != VK_NULL_HANDLE);
                generateWriteDescriptorData(layoutBinding.binding,
                    layoutBinding.descriptorType,
                    nullptr, &bufferInfo, nullptr);
                break;
            }

            default:
                VGPU_UNREACHABLE();
                break;
        }
    }

    vkUpdateDescriptorSets(
        device->device,
        descriptorWriteCount,
        descriptorWrites.data(),
        0,
        nullptr
    );
}

VGPUBindGroup VulkanDevice::CreateBindGroup(const VGPUBindGroupLayout layout, const VGPUBindGroupDesc* desc)
{
    VulkanBindGroupLayout* vulkanLayout = static_cast<VulkanBindGroupLayout*>(layout);

    auto AllocateDescriptorSet = [](VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout setLayout, VkDescriptorSet& descriptorSet, uint32_t maxVariableDescriptorCounts)
        {
            // For variable length descriptor arrays, this specify the maximum count we expect them to be.
            // Note that this value will apply to all bindings defined as variable arrays in the BindGroupLayout
            // used to allocate this BindGroup
            VkDescriptorSetVariableDescriptorCountAllocateInfo variableLengthInfo{};
            variableLengthInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
            variableLengthInfo.descriptorSetCount = 1;
            variableLengthInfo.pDescriptorCounts = &maxVariableDescriptorCounts;

            VkDescriptorSetAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &setLayout;
            allocInfo.pNext = &variableLengthInfo;

            return vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
        };

    // Have we create a DescriptorSet pool already?
    if (descriptorSetPools.empty())
        descriptorSetPools.emplace_back(CreateDescriptorSetPool());

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    //  Create DescriptorSet
    const uint32_t maxVariableArrayLength = 0;
    VkResult result = AllocateDescriptorSet(device, descriptorSetPools.back(), vulkanLayout->handle, descriptorSet, maxVariableArrayLength);
    // If we have run out of pool memory
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
    {
        // We need to allocate a new DescriptorPool and retry
        descriptorSetPools.emplace_back(CreateDescriptorSetPool());
        result = AllocateDescriptorSet(device, descriptorSetPools.back(), vulkanLayout->handle, descriptorSet, maxVariableArrayLength);
    }
    if (result != VK_SUCCESS)
    {
        return nullptr;
    }

    if (desc->label)
    {
        SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, reinterpret_cast<uint64_t>(descriptorSet), desc->label);
    }

    VulkanBindGroup* bindGroup = new VulkanBindGroup();
    bindGroup->device = this;
    bindGroup->bindGroupLayout = vulkanLayout;
    bindGroup->bindGroupLayout->AddRef();
    bindGroup->descriptorPool = descriptorSetPools.back();
    bindGroup->descriptorSet = descriptorSet;

    // Set up the initial bindings
    bindGroup->Update(desc->entryCount, desc->entries);

    return bindGroup;
}

/* Pipeline */
VkResult SetupShaderStage(VkDevice device,
    VkPipelineShaderStageCreateInfo& pipelineStage,
    std::string& entryPoint,
    const VGPUShaderStageDesc& shaderDesc)
{
    VkShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = shaderDesc.size;
    moduleInfo.pCode = (const uint32_t*)shaderDesc.bytecode;

    const VkResult vkResult = vkCreateShaderModule(device, &moduleInfo, nullptr, &pipelineStage.module);

    if (vkResult != VK_SUCCESS)
    {
        VK_LOG_ERROR(vkResult, "Failed to create a pipeline shader module");
        return vkResult;
    }

    entryPoint = shaderDesc.entryPointName ? shaderDesc.entryPointName : "main";

    pipelineStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineStage.stage = (VkShaderStageFlagBits)ToVkShaderStageFlags(shaderDesc.stage);
    pipelineStage.pName = entryPoint.c_str();

    return VK_SUCCESS;
}

VGPUPipeline VulkanDevice::CreateRenderPipeline(const VGPURenderPipelineDesc* desc)
{
    VulkanPipelineLayout* layout = (VulkanPipelineLayout*)desc->layout;

    // ShaderStages
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages(desc->shaderStageCount);
    std::vector<std::string> stageEntryPoints(desc->shaderStageCount);

    for (uint32_t i = 0; i < desc->shaderStageCount; i++)
    {
        const VGPUShaderStageDesc& shaderDesc = desc->shaderStages[i];
        VkResult res = SetupShaderStage(device, shaderStages[i], stageEntryPoints[i], shaderDesc);
        if (res != VK_SUCCESS)
        {
            return nullptr;
        }
    }

    // RenderingInfo
    VkFormat colorAttachmentFormats[VGPU_MAX_COLOR_ATTACHMENTS];
    VkPipelineRenderingCreateInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

    for (uint32_t i = 0; i < desc->colorFormatCount; ++i)
    {
        VGPU_ASSERT(desc->colorFormats[i] != VGPUTextureFormat_Undefined);

        colorAttachmentFormats[renderingInfo.colorAttachmentCount] = ToVkFormat(desc->colorFormats[i]);
        renderingInfo.colorAttachmentCount++;
    }
    renderingInfo.pColorAttachmentFormats = colorAttachmentFormats;

    // VertexInputState
    std::vector<VkVertexInputBindingDescription> vertexInputBindings;
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;

    vertexInputBindings.resize(desc->vertex.layoutCount);
    for (uint32_t binding = 0; binding < desc->vertex.layoutCount; ++binding)
    {
        const VGPUVertexBufferLayout& bufferLayout = desc->vertex.layouts[binding];

        vertexInputBindings[binding].binding = binding;
        vertexInputBindings[binding].stride = bufferLayout.stride;
        vertexInputBindings[binding].inputRate = ToVkVertexInputRate(bufferLayout.stepMode);

        for (uint32_t attributeIndex = 0; attributeIndex < bufferLayout.attributeCount; ++attributeIndex)
        {
            const VGPUVertexAttribute& attribute = bufferLayout.attributes[attributeIndex];

            VkVertexInputAttributeDescription vertexInputAttribute;
            vertexInputAttribute.location = attribute.shaderLocation;
            vertexInputAttribute.binding = binding;
            vertexInputAttribute.format = ToVkFormat(attribute.format);
            vertexInputAttribute.offset = attribute.offset;
            vertexInputAttributes.push_back(vertexInputAttribute);
        }
    }

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = (uint32_t)vertexInputBindings.size();
    vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
    vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributes.size();
    vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

    // InputAssemblyState
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = ToVk(desc->primitiveTopology);
    switch (desc->primitiveTopology)
    {
        case VGPUPrimitiveTopology_LineStrip:
        case VGPUPrimitiveTopology_TriangleStrip:
            inputAssemblyState.primitiveRestartEnable = VK_TRUE;
            break;
        default:
            inputAssemblyState.primitiveRestartEnable = VK_FALSE;
            break;
    }

    // TessellationState
    VkPipelineTessellationStateCreateInfo tessellationState = {};
    tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    if (inputAssemblyState.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
    {
        tessellationState.patchControlPoints = desc->patchControlPoints;
    }
    else
    {
        tessellationState.patchControlPoints = 0;
    }

    // ViewportState
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // RasterizationState
    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = (desc->depthStencilState.depthClipMode == VGPUDepthClipMode_Clamp) ? VK_TRUE : VK_FALSE;

    VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClipStateInfo = {};
    depthClipStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT;

    const void** tail = &rasterizationState.pNext;
    if (depthClipEnableFeatures.depthClipEnable == VK_TRUE)
    {
        depthClipStateInfo.depthClipEnable = (desc->depthStencilState.depthClipMode == VGPUDepthClipMode_Clip) ? VK_TRUE : VK_FALSE;

        APPEND_EXT(depthClipStateInfo);
    }
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = ToVk(desc->rasterizerState.fillMode, features2.features.fillModeNonSolid);
    rasterizationState.cullMode = ToVk(desc->rasterizerState.cullMode);
    rasterizationState.frontFace = ToVk(desc->rasterizerState.frontFace);
    // Can be managed by command buffer
    rasterizationState.depthBiasEnable = desc->depthStencilState.depthBias != 0.0f || desc->depthStencilState.depthBiasSlopeScale != 0.0f;
    rasterizationState.depthBiasConstantFactor = desc->depthStencilState.depthBias;
    rasterizationState.depthBiasClamp = desc->depthStencilState.depthBiasClamp;
    rasterizationState.depthBiasSlopeFactor = desc->depthStencilState.depthBiasSlopeScale;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineRasterizationConservativeStateCreateInfoEXT rasterizationConservativeState = {};
    if (desc->rasterizerState.conservativeRaster)
    {
        rasterizationConservativeState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
        rasterizationConservativeState.conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
        rasterizationConservativeState.extraPrimitiveOverestimationSize = 0.0f;

        APPEND_EXT(rasterizationConservativeState);
    }

    // Multi sampling state
    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = static_cast<VkSampleCountFlagBits>(desc->sampleCount);

    VGPU_ASSERT(multisampleState.rasterizationSamples <= 32);
    if (multisampleState.rasterizationSamples > VK_SAMPLE_COUNT_1_BIT)
    {
        //multisampleState.alphaToCoverageEnable = desc.blendState.alphaToCoverageEnable ? VK_TRUE : VK_FALSE;
        multisampleState.alphaToOneEnable = VK_FALSE;
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.minSampleShading = 1.0f;
    }
    const VkSampleMask sampleMask = UINT32_MAX;
    multisampleState.pSampleMask = &sampleMask;

    // DepthStencilState
    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    if (desc->depthStencilFormat != VGPUTextureFormat_Undefined)
    {
        renderingInfo.depthAttachmentFormat = ToVkFormat(desc->depthStencilFormat);
        if (!vgpuIsDepthOnlyFormat(desc->depthStencilFormat))
        {
            renderingInfo.stencilAttachmentFormat = ToVkFormat(desc->depthStencilFormat);
        }

        depthStencilState.depthTestEnable = (desc->depthStencilState.depthCompareFunction != VGPUCompareFunction_Always || desc->depthStencilState.depthWriteEnabled) ? VK_TRUE : VK_FALSE;
        depthStencilState.depthWriteEnable = desc->depthStencilState.depthWriteEnabled ? VK_TRUE : VK_FALSE;
        depthStencilState.depthCompareOp = ToVk(desc->depthStencilState.depthCompareFunction);
        if (features2.features.depthBounds == VK_TRUE)
        {
            depthStencilState.depthBoundsTestEnable = desc->depthStencilState.depthBoundsTestEnable ? VK_TRUE : VK_FALSE;
        }
        else
        {
            depthStencilState.depthBoundsTestEnable = false;
        }
        depthStencilState.minDepthBounds = 0.0f;
        depthStencilState.maxDepthBounds = 1.0f;

        depthStencilState.stencilTestEnable = vgpuStencilTestEnabled(&desc->depthStencilState) ? VK_TRUE : VK_FALSE;
        depthStencilState.front.failOp = ToVk(desc->depthStencilState.stencilFront.failOperation);
        depthStencilState.front.passOp = ToVk(desc->depthStencilState.stencilFront.passOperation);
        depthStencilState.front.depthFailOp = ToVk(desc->depthStencilState.stencilFront.depthFailOperation);
        depthStencilState.front.compareOp = ToVk(desc->depthStencilState.stencilFront.compareFunction);
        depthStencilState.front.compareMask = desc->depthStencilState.stencilReadMask;
        depthStencilState.front.writeMask = desc->depthStencilState.stencilWriteMask;
        depthStencilState.front.reference = 0;

        depthStencilState.back.failOp = ToVk(desc->depthStencilState.stencilBack.failOperation);
        depthStencilState.back.passOp = ToVk(desc->depthStencilState.stencilBack.passOperation);
        depthStencilState.back.depthFailOp = ToVk(desc->depthStencilState.stencilBack.depthFailOperation);
        depthStencilState.back.compareOp = ToVk(desc->depthStencilState.stencilBack.compareFunction);
        depthStencilState.back.compareMask = desc->depthStencilState.stencilReadMask;
        depthStencilState.back.writeMask = desc->depthStencilState.stencilWriteMask;
        depthStencilState.back.reference = 0;
    }
    else
    {
        depthStencilState.depthTestEnable = VK_FALSE;
        depthStencilState.depthWriteEnable = VK_FALSE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.minDepthBounds = 0.0f;
        depthStencilState.maxDepthBounds = 1.0f;

        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front.failOp = VK_STENCIL_OP_KEEP;
        depthStencilState.front.passOp = VK_STENCIL_OP_KEEP;
        depthStencilState.front.depthFailOp = VK_STENCIL_OP_KEEP;
        depthStencilState.front.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencilState.front.compareMask = desc->depthStencilState.stencilReadMask;
        depthStencilState.front.writeMask = desc->depthStencilState.stencilWriteMask;
        depthStencilState.front.reference = 0;

        depthStencilState.back = depthStencilState.front;

        renderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
    }

    // Color blend state
    VkPipelineColorBlendStateCreateInfo blendState = {};
    blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendState.attachmentCount = 0;

    VkPipelineColorBlendAttachmentState blendAttachmentStates[VGPU_MAX_COLOR_ATTACHMENTS] = {};

    for (uint32_t i = 0; i < desc->colorFormatCount; ++i)
    {
        VGPU_ASSERT(desc->colorFormats[i] != VGPUTextureFormat_Undefined);

        uint32_t attachmentIndex = 0;
        if (desc->blendState.independentBlendEnable)
            attachmentIndex = i;

        const VGPURenderTargetBlendState& attachment = desc->blendState.renderTargets[attachmentIndex];
        blendAttachmentStates[blendState.attachmentCount].blendEnable = attachment.blendEnabled ? VK_TRUE : VK_FALSE; // BlendEnabled(&attachment) ? VK_TRUE : VK_FALSE;
        blendAttachmentStates[blendState.attachmentCount].srcColorBlendFactor = ToVk(attachment.srcColorBlendFactor);
        blendAttachmentStates[blendState.attachmentCount].dstColorBlendFactor = ToVk(attachment.dstColorBlendFactor);
        blendAttachmentStates[blendState.attachmentCount].colorBlendOp = ToVk(attachment.colorBlendOperation);
        blendAttachmentStates[blendState.attachmentCount].srcAlphaBlendFactor = ToVk(attachment.srcAlphaBlendFactor);
        blendAttachmentStates[blendState.attachmentCount].dstAlphaBlendFactor = ToVk(attachment.dstAlphaBlendFactor);
        blendAttachmentStates[blendState.attachmentCount].alphaBlendOp = ToVk(attachment.alphaBlendOperation);
        blendAttachmentStates[blendState.attachmentCount].colorWriteMask = ToVk(attachment.colorWriteMask);
        blendState.attachmentCount++;
    }
    blendState.logicOpEnable = VK_FALSE;
    blendState.logicOp = VK_LOGIC_OP_CLEAR;
    blendState.pAttachments = blendAttachmentStates;
    blendState.blendConstants[0] = 0.0f;
    blendState.blendConstants[1] = 0.0f;
    blendState.blendConstants[2] = 0.0f;
    blendState.blendConstants[3] = 0.0f;

    // 
    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = &renderingInfo;
    createInfo.stageCount = desc->shaderStageCount;
    createInfo.pStages = shaderStages.data();
    createInfo.pVertexInputState = &vertexInputState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pTessellationState = (inputAssemblyState.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) ? &tessellationState : nullptr;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pDepthStencilState = &depthStencilState;
    createInfo.pColorBlendState = &blendState;
    createInfo.pDynamicState = &dynamicStateInfo;
    createInfo.layout = layout->handle;
    createInfo.renderPass = VK_NULL_HANDLE;

    VkPipeline handle = VK_NULL_HANDLE;
    const VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle);

    if (result != VK_SUCCESS)
    {
        return nullptr;
    }

    for (size_t i = 0; i < shaderStages.size(); i++)
    {
        vkDestroyShaderModule(device, shaderStages[i].module, nullptr);
    }

    VulkanPipeline* pipeline = new VulkanPipeline();
    pipeline->renderer = this;
    pipeline->type = VGPUPipelineType_Render;
    pipeline->pipelineLayout = layout;
    pipeline->pipelineLayout->AddRef();
    pipeline->handle = handle;

    if (desc->label)
    {
        pipeline->SetLabel(desc->label);
    }

    return pipeline;
}

VGPUPipeline VulkanDevice::CreateComputePipeline(const VGPUComputePipelineDesc* desc)
{
    VulkanPipeline* pipeline = new VulkanPipeline();
    pipeline->renderer = this;
    pipeline->type = VGPUPipelineType_Compute;
    pipeline->pipelineLayout = (VulkanPipelineLayout*)desc->layout;
    pipeline->pipelineLayout->AddRef();

    VkPipelineShaderStageCreateInfo stage;
    std::string entryPoint;

    VkResult result = SetupShaderStage(device, stage, entryPoint, desc->shader);
    if (result != VK_SUCCESS)
    {
        return nullptr;
    }

    VkComputePipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.stage = stage;
    createInfo.layout = pipeline->pipelineLayout->handle;

    result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline->handle);

    // Delete shader module.
    vkDestroyShaderModule(device, stage.module, nullptr);

    if (result != VK_SUCCESS)
    {
        delete pipeline;
        return nullptr;
    }

    if (desc->label)
    {
        pipeline->SetLabel(desc->label);
    }

    return pipeline;
}

VGPUPipeline VulkanDevice::CreateRayTracingPipeline(const VGPURayTracingPipelineDesc* desc)
{
    VulkanPipelineLayout* layout = (VulkanPipelineLayout*)desc->layout;
    VGPU_UNUSED(desc);

    VulkanPipeline* pipeline = new VulkanPipeline();
    pipeline->renderer = this;
    pipeline->type = VGPUPipelineType_RayTracing;
    pipeline->pipelineLayout = layout;
    pipeline->pipelineLayout->AddRef();

    if (desc->label)
    {
        pipeline->SetLabel(desc->label);
    }

    return pipeline;
}

/* VulkanQueryHeap */
VulkanQueryHeap::~VulkanQueryHeap()
{
}

void VulkanQueryHeap::SetLabel(const char* label)
{
    renderer->SetObjectName(VK_OBJECT_TYPE_QUERY_POOL, reinterpret_cast<uint64_t>(handle), label);
}

VGPUQueryHeap VulkanDevice::CreateQueryHeap(const VGPUQueryHeapDesc* desc)
{
    VkQueryPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    createInfo.queryType = ToVk(desc->type);
    createInfo.queryCount = desc->count;

    if (desc->type == VGPUQueryType_PipelineStatistics)
    {
        createInfo.pipelineStatistics =
            VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
            VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
            VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
            VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
            VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
            VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
            VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
            ;

        //if (StageMask & VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT)
        //{
        //    createInfo.pipelineStatistics |=
        //        VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT |
        //        VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT;
        //}

        //if (StageMask & VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT)
        //    createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT;

        //if (StageMask & VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT)
        //    createInfo.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT;
    }

    VkQueryPool handle = VK_NULL_HANDLE;
    VkResult result = vkCreateQueryPool(device, &createInfo, nullptr, &handle);
    if (result != VK_SUCCESS)
    {
        return nullptr;
    }

    VulkanQueryHeap* heap = new VulkanQueryHeap();
    heap->renderer = this;
    heap->type = desc->type;
    heap->count = desc->count;
    heap->handle = handle;
    heap->resultSize = GetQueryResultSize(desc->type);

    if (desc->label)
    {
        heap->SetLabel(desc->label);
    }

    return heap;
}

/* VulkanSwapChain */
VulkanSwapChain::~VulkanSwapChain()
{
    for (size_t i = 0, count = backbufferTextures.size(); i < count; ++i)
    {
        backbufferTextures[i]->Release();
    }

    if (acquireSemaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(renderer->device, acquireSemaphore, nullptr);
        acquireSemaphore = VK_NULL_HANDLE;
    }

    if (releaseSemaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(renderer->device, releaseSemaphore, nullptr);
        releaseSemaphore = VK_NULL_HANDLE;
    }

    if (handle != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(renderer->device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    if (surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(renderer->instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }
}

bool VulkanSwapChain::Update()
{
    VkSurfaceCapabilitiesKHR caps;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer->physicalDevice, surface, &caps));

    uint32_t formatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->physicalDevice, surface, &formatCount, nullptr));

    std::vector<VkSurfaceFormatKHR> swapchainFormats(formatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->physicalDevice, surface, &formatCount, swapchainFormats.data()));

    uint32_t presentModeCount;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->physicalDevice, surface, &presentModeCount, nullptr));

    std::vector<VkPresentModeKHR> swapchainPresentModes(presentModeCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->physicalDevice, surface, &presentModeCount, swapchainPresentModes.data()));

    VkSurfaceFormatKHR surfaceFormat = {};
    surfaceFormat.format = ToVkFormat(colorFormat);
    surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    bool valid = false;

    for (const auto& format : swapchainFormats)
    {
        if (!allowHDR && format.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            continue;

        if (format.format == surfaceFormat.format)
        {
            surfaceFormat = format;
            valid = true;
            break;
        }
    }
    if (!valid)
    {
        surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    if (caps.currentExtent.width != 0xFFFFFFFF
        && caps.currentExtent.height != 0xFFFFFFFF)
    {
        extent = caps.currentExtent;
    }
    else
    {
        extent.width = _VGPU_MAX(caps.minImageExtent.width, _VGPU_MIN(caps.maxImageExtent.width, extent.width));
        extent.height = _VGPU_MAX(caps.minImageExtent.height, _VGPU_MIN(caps.maxImageExtent.height, extent.height));
    }

    // Determine the number of images
    uint32_t imageCount = caps.minImageCount + 1;
    if ((caps.maxImageCount > 0) && (imageCount > caps.maxImageCount))
    {
        imageCount = caps.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent.width = extent.width;
    createInfo.imageExtent.height = extent.height;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Enable transfer source on swap chain images if supported
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Enable transfer destination on swap chain images if supported
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.preTransform = caps.currentTransform;

    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // The only one that is always supported
    if (!vsync)
    {
        // The mailbox/immediate present mode is not necessarily supported:
        for (auto& presentMode : swapchainPresentModes)
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = handle;

    VkResult result = vkCreateSwapchainKHR(renderer->device, &createInfo, nullptr, &handle);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "vkCreateSwapchainKHR failed");
        return false;
    }

    if (createInfo.oldSwapchain != VK_NULL_HANDLE)
    {
        for (size_t i = 0, count = backbufferTextures.size(); i < count; ++i)
        {
            delete backbufferTextures[i];
        }
        backbufferTextures.clear();

        vkDestroySwapchainKHR(renderer->device, createInfo.oldSwapchain, nullptr);
    }

    VK_CHECK(vkGetSwapchainImagesKHR(renderer->device, handle, &imageCount, nullptr));
    std::vector<VkImage> swapchainImages(imageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(renderer->device, handle, &imageCount, swapchainImages.data()));

    colorFormat = VGPUTextureFormat_BGRA8Unorm;
    if (createInfo.imageFormat == VK_FORMAT_B8G8R8A8_UNORM)
    {
        colorFormat = VGPUTextureFormat_BGRA8Unorm;
    }
    else if (createInfo.imageFormat == VK_FORMAT_B8G8R8A8_SRGB)
    {
        colorFormat = VGPUTextureFormat_BGRA8UnormSrgb;
    }
    else if (createInfo.imageFormat == VK_FORMAT_R8G8B8A8_UNORM)
    {
        colorFormat = VGPUTextureFormat_RGBA8Unorm;
    }
    else if (createInfo.imageFormat == VK_FORMAT_R8G8B8A8_SRGB)
    {
        colorFormat = VGPUTextureFormat_RGBA8UnormSrgb;
    }

    imageIndex = 0;
    backbufferTextures.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        VulkanTexture* texture = new VulkanTexture();
        texture->renderer = renderer;
        texture->dimension = VGPUTextureDimension_2D;
        texture->format = colorFormat;
        texture->handle = swapchainImages[i];
        texture->width = createInfo.imageExtent.width;
        texture->height = createInfo.imageExtent.height;
        texture->vkFormat = createInfo.imageFormat;
        backbufferTextures[i] = texture;
    }

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (acquireSemaphore == VK_NULL_HANDLE)
    {
        VK_CHECK(vkCreateSemaphore(renderer->device, &semaphoreInfo, nullptr, &acquireSemaphore));
    }

    if (releaseSemaphore == VK_NULL_HANDLE)
    {
        VK_CHECK(vkCreateSemaphore(renderer->device, &semaphoreInfo, nullptr, &releaseSemaphore));
    }

    extent = createInfo.imageExtent;
    return true;
}

void VulkanSwapChain::SetLabel(const char* label)
{
    renderer->SetObjectName(VK_OBJECT_TYPE_SWAPCHAIN_KHR, reinterpret_cast<uint64_t>(handle), label);
}

VGPUSwapChain VulkanDevice::CreateSwapChain(const VGPUSwapChainDesc* desc)
{
    VkSurfaceKHR vk_surface = CreateSurface(desc);
    if (vk_surface == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    VkBool32 supported = VK_FALSE;
    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndices.familyIndices[VGPUCommandQueue_Graphics], vk_surface, &supported);
    if (result != VK_SUCCESS || !supported)
    {
        return nullptr;
    }

    VulkanSwapChain* swapChain = new VulkanSwapChain();
    swapChain->renderer = this;
    swapChain->surface = vk_surface;
    swapChain->extent.width = desc->width;
    swapChain->extent.height = desc->height;
    swapChain->colorFormat = desc->format;
    swapChain->vsync = desc->presentMode == VGPUPresentMode_Fifo;
    if (!swapChain->Update())
    {
        delete swapChain;
        return nullptr;
    }

    if (desc->label)
    {
        swapChain->SetLabel(desc->label);
    }

    return swapChain;
}

/* CommandBuffer */
VulkanCommandBuffer::~VulkanCommandBuffer()
{
    Reset();

    for (uint32_t j = 0; j < VGPU_MAX_INFLIGHT_FRAMES; ++j)
    {
        //vkFreeCommandBuffers(device, commandPools[j], 1, &commandBuffers[j]);
        vkDestroyCommandPool(renderer->device, commandPools[j], nullptr);
    }

    vkDestroySemaphore(renderer->device, semaphore, nullptr);
}

void VulkanCommandBuffer::Reset()
{
    hasLabel = false;
    hasRenderPassLabel = false;
    clearValueCount = 0u;
    insideRenderPass = false;

    presentSwapChains.clear();

    bindGroupsDirty = false;
    numBoundBindGroups = 0;
    for (uint32_t i = 0; i < VGPU_MAX_BIND_GROUPS; ++i)
    {
        if (boundBindGroups[i])
        {
            boundBindGroups[i]->Release();
            boundBindGroups[i] = nullptr;
        }

        descriptorSets[i] = VK_NULL_HANDLE;
    }

    if (currentPipeline)
    {
        currentPipeline->Release();
        currentPipeline = nullptr;
    }
}

void VulkanCommandBuffer::Begin(uint32_t frameIndex, const char* label)
{
    Reset();

    VK_CHECK(vkResetCommandPool(renderer->device, commandPools[frameIndex], 0));

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional
    VK_CHECK(vkBeginCommandBuffer(commandBuffers[frameIndex], &beginInfo));

    commandBuffer = commandBuffers[frameIndex];

    if (queueType == VGPUCommandQueue_Graphics)
    {
        VkRect2D scissors[16];
        for (uint32_t i = 0; i < _VGPU_COUNT_OF(scissors); ++i)
        {
            scissors[i].offset.x = 0;
            scissors[i].offset.y = 0;
            scissors[i].extent.width = 65535;
            scissors[i].extent.height = 65535;
        }
        vkCmdSetScissor(commandBuffer, 0, _VGPU_COUNT_OF(scissors), scissors);

        const float blendConstants[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        vkCmdSetBlendConstants(commandBuffer, blendConstants);
        vkCmdSetStencilReference(commandBuffer, VK_STENCIL_FRONT_AND_BACK, ~0u);

        if (renderer->features2.features.depthBounds == VK_TRUE)
        {
            vkCmdSetDepthBounds(commandBuffer, 0.0f, 1.0f);
        }
    }

    if (label)
    {
        PushDebugGroup(label);
        hasLabel = true;
    }
}

void VulkanCommandBuffer::PushDebugGroup(const char* groupLabel)
{
    if (!renderer->debugUtils)
        return;

    VkDebugUtilsLabelEXT label{};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pNext = nullptr;
    label.pLabelName = groupLabel;
    label.color[0] = 0.0f;
    label.color[1] = 0.0f;
    label.color[2] = 0.0f;
    label.color[3] = 1.0f;
    vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &label);
}

void VulkanCommandBuffer::PopDebugGroup()
{
    if (!renderer->debugUtils)
        return;

    vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}

void VulkanCommandBuffer::InsertDebugMarker(const char* markerLabel)
{
    if (!renderer->debugUtils)
        return;

    VkDebugUtilsLabelEXT label{};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pNext = nullptr;
    label.pLabelName = markerLabel;
    label.color[0] = 0.0f;
    label.color[1] = 0.0f;
    label.color[2] = 0.0f;
    label.color[3] = 1.0f;
    vkCmdInsertDebugUtilsLabelEXT(commandBuffer, &label);
}

void VulkanCommandBuffer::ClearBuffer(VGPUBuffer buffer, uint64_t offset, uint64_t size)
{
    VulkanBuffer* backendBuffer = (VulkanBuffer*)buffer;

    //InsertBufferMemoryBarrier(buffer, BufferUsage::CopyDst);
    VkDeviceSize commandSize = (size == VGPU_WHOLE_SIZE) ? VK_WHOLE_SIZE : size;
    vkCmdFillBuffer(commandBuffer, backendBuffer->handle, offset, commandSize, 0u);
}

void VulkanCommandBuffer::SetPipeline(VGPUPipeline pipeline)
{
    VulkanPipeline* backendPipeline = (VulkanPipeline*)pipeline;
    if (currentPipeline == backendPipeline)
        return;

    currentPipeline = backendPipeline;
    currentPipeline->AddRef();

    vkCmdBindPipeline(commandBuffer, currentPipeline->bindPoint, currentPipeline->handle);
}


void VulkanCommandBuffer::SetBindGroup(uint32_t groupIndex, VGPUBindGroup bindGroup)
{
    VGPU_VERIFY(bindGroup != nullptr);
    VGPU_VERIFY(groupIndex < VGPU_MAX_BIND_GROUPS);

    if (boundBindGroups[groupIndex] != bindGroup)
    {
        bindGroupsDirty = true;
        boundBindGroups[groupIndex] = static_cast<VulkanBindGroup*>(bindGroup);
        boundBindGroups[groupIndex]->AddRef();
        descriptorSets[groupIndex] = boundBindGroups[groupIndex]->descriptorSet;
        numBoundBindGroups = _VGPU_MAX(groupIndex + 1, numBoundBindGroups);
    }
}

void VulkanCommandBuffer::SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size)
{
    VGPU_ASSERT(size <= renderer->properties2.properties.limits.maxPushConstantsSize);
    VGPU_ASSERT(currentPipeline);

    const VkPushConstantRange& range = currentPipeline->pipelineLayout->pushConstantRanges[pushConstantIndex];
    vkCmdPushConstants(commandBuffer, currentPipeline->pipelineLayout->handle, range.stageFlags, range.offset, size, data);
}

void VulkanCommandBuffer::FlushBindGroups()
{
    if (!currentPipeline->pipelineLayout)
        return;

    VGPU_ASSERT(currentPipeline);
    VGPU_ASSERT(currentPipeline->pipelineLayout);

    if (!bindGroupsDirty)
        return;

    vkCmdBindDescriptorSets(
        commandBuffer,
        currentPipeline->bindPoint,
        currentPipeline->pipelineLayout->handle,
        0u,
        currentPipeline->pipelineLayout->bindGroupLayoutCount,
        descriptorSets,
        0, nullptr
    );
    bindGroupsDirty = false;
}

void VulkanCommandBuffer::PrepareDispatch()
{
    FlushBindGroups();
}

void VulkanCommandBuffer::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    VGPU_ASSERT(!insideRenderPass);

    PrepareDispatch();
    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void VulkanCommandBuffer::DispatchIndirect(VGPUBuffer buffer, uint64_t offset)
{
    VGPU_ASSERT(!insideRenderPass);
    VulkanBuffer* vulkanBuffer = (VulkanBuffer*)buffer;

    PrepareDispatch();
    vkCmdDispatchIndirect(commandBuffer, vulkanBuffer->handle, offset);
}

VGPUTexture VulkanCommandBuffer::AcquireSwapchainTexture(VGPUSwapChain swapChain)
{
    VulkanSwapChain* vulkanSwapChain = (VulkanSwapChain*)swapChain;

    // Check if window is minimized
    VkSurfaceCapabilitiesKHR surfaceProperties;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        renderer->physicalDevice,
        vulkanSwapChain->surface,
        &surfaceProperties));

    if (surfaceProperties.currentExtent.width == 0 ||
        surfaceProperties.currentExtent.width == 0xFFFFFFFF)
    {
        return nullptr;
    }

    if (vulkanSwapChain->extent.width != surfaceProperties.currentExtent.width ||
        vulkanSwapChain->extent.height != surfaceProperties.currentExtent.height)
    {
        renderer->WaitIdle();
        vulkanSwapChain->Update();
    }

    VkResult result = vkAcquireNextImageKHR(renderer->device,
        vulkanSwapChain->handle,
        UINT64_MAX,
        vulkanSwapChain->acquireSemaphore, VK_NULL_HANDLE,
        &vulkanSwapChain->imageIndex);

    if (result != VK_SUCCESS)
    {
        // Handle outdated error in acquire
        if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            renderer->WaitIdle();
            vulkanSwapChain->Update();
            return AcquireSwapchainTexture(swapChain);
        }
    }

    VulkanTexture* swapchainTexture = vulkanSwapChain->backbufferTextures[vulkanSwapChain->imageIndex];

    // Transition from undefined -> render target
    VkImageSubresourceRange range{};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = VK_REMAINING_MIP_LEVELS;
    range.baseArrayLayer = 0;
    range.layerCount = VK_REMAINING_ARRAY_LAYERS;
    InsertImageMemoryBarrier(
        swapchainTexture->handle,
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        range
    );

    presentSwapChains.push_back(vulkanSwapChain);

    return swapchainTexture;
}

void VulkanCommandBuffer::BeginRenderPass(const VGPURenderPassDesc* desc)
{
    uint32_t width = renderer->properties2.properties.limits.maxFramebufferWidth;
    uint32_t height = renderer->properties2.properties.limits.maxFramebufferHeight;

    if (desc->label)
    {
        PushDebugGroup(desc->label);
        hasRenderPassLabel = true;
    }

    if (renderer->dynamicRendering)
    {
        VkRenderingInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = VK_NULL_HANDLE;
        renderingInfo.flags = 0u;
        renderingInfo.renderArea.extent = { UINT32_MAX, UINT32_MAX };
        renderingInfo.layerCount = 1;
        renderingInfo.viewMask = 0;

        VkRenderingAttachmentInfo colorAttachments[VGPU_MAX_COLOR_ATTACHMENTS] = {};
        for (uint32_t i = 0; i < desc->colorAttachmentCount; ++i)
        {
            const VGPURenderPassColorAttachment* attachment = &desc->colorAttachments[i];
            VulkanTexture* texture = (VulkanTexture*)attachment->texture;
            const uint32_t level = attachment->level;
            const uint32_t slice = attachment->slice;

            width = _VGPU_MIN(width, _VGPU_MAX(1U, texture->width >> level));
            height = _VGPU_MIN(height, _VGPU_MAX(1U, texture->height >> level));

            VkRenderingAttachmentInfo& attachmentInfo = colorAttachments[renderingInfo.colorAttachmentCount++];
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.pNext = nullptr;
            attachmentInfo.imageView = texture->GetRTV(level, slice);
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
            attachmentInfo.loadOp = ToVkAttachmentLoadOp(attachment->loadAction);
            attachmentInfo.storeOp = ToVkAttachmentStoreOp(attachment->storeAction);

            attachmentInfo.clearValue.color.float32[0] = attachment->clearColor.r;
            attachmentInfo.clearValue.color.float32[1] = attachment->clearColor.g;
            attachmentInfo.clearValue.color.float32[2] = attachment->clearColor.b;
            attachmentInfo.clearValue.color.float32[3] = attachment->clearColor.a;
        }

        VGPUTextureFormat depthStencilFormat = VGPUTextureFormat_Undefined;
        VkRenderingAttachmentInfo depthAttachment = {};
        VkRenderingAttachmentInfo stencilAttachment = {};
        const bool hasDepthOrStencil = desc->depthStencilAttachment != nullptr && desc->depthStencilAttachment->texture != nullptr;
        bool hasStencil = false;
        if (hasDepthOrStencil)
        {
            const VGPURenderPassDepthStencilAttachment* attachment = desc->depthStencilAttachment;

            depthStencilFormat = attachment->texture->GetFormat();
            VulkanTexture* texture = (VulkanTexture*)attachment->texture;
            const uint32_t level = attachment->level;
            const uint32_t slice = attachment->slice;

            width = _VGPU_MIN(width, _VGPU_MAX(1U, texture->width >> level));
            height = _VGPU_MIN(height, _VGPU_MAX(1U, texture->height >> level));

            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            depthAttachment.pNext = VK_NULL_HANDLE;
            depthAttachment.imageView = texture->GetRTV(level, slice);
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
            depthAttachment.loadOp = ToVkAttachmentLoadOp(attachment->depthLoadAction);
            depthAttachment.storeOp = ToVkAttachmentStoreOp(attachment->depthStoreAction);
            depthAttachment.clearValue.depthStencil.depth = attachment->depthClearValue;

            if (!vgpuIsDepthOnlyFormat(depthStencilFormat))
            {
                hasStencil = true;

                stencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
                stencilAttachment.pNext = VK_NULL_HANDLE;
                stencilAttachment.imageView = texture->GetRTV(level, slice);
                stencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                stencilAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
                stencilAttachment.loadOp = ToVkAttachmentLoadOp(attachment->stencilLoadAction);
                stencilAttachment.storeOp = ToVkAttachmentStoreOp(attachment->stencilStoreAction);
                stencilAttachment.clearValue.depthStencil.stencil = attachment->stencilClearValue;
            }

            // Barrier
            VkImageSubresourceRange depthTextureRange{};
            depthTextureRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            depthTextureRange.baseMipLevel = 0;
            depthTextureRange.levelCount = VK_REMAINING_MIP_LEVELS;
            depthTextureRange.baseArrayLayer = 0;
            depthTextureRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

            InsertImageMemoryBarrier(
                texture->handle,
                0,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                depthTextureRange
            );
        }

        renderingInfo.renderArea.offset.x = 0;
        renderingInfo.renderArea.offset.y = 0;
        renderingInfo.renderArea.extent.width = width;
        renderingInfo.renderArea.extent.height = height;
        renderingInfo.pColorAttachments = colorAttachments;
        renderingInfo.pDepthAttachment = hasDepthOrStencil ? &depthAttachment : nullptr;
        renderingInfo.pStencilAttachment = hasStencil ? &stencilAttachment : nullptr;

        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }
    else
    {
        //VkRenderPassBeginInfo renderPassBeginInfo{};
        //renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        //renderPassBeginInfo.renderPass = render_pass;
        //renderPassBeginInfo.framebuffer = framebuffers[i];
        //renderPassBeginInfo.renderArea.extent.width = width;
        //renderPassBeginInfo.renderArea.extent.height = height;
        //renderPassBeginInfo.clearValueCount = 3;
        //renderPassBeginInfo.pClearValues = clear_values.data();

        //vkCmdBeginRenderPass2(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    // The viewport and scissor default to cover all of the attachments
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(height);
    viewport.width = static_cast<float>(width);
    viewport.height = -static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissorRect{};
    scissorRect.offset.x = 0;
    scissorRect.offset.y = 0;
    scissorRect.extent.width = width;
    scissorRect.extent.height = height;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);

    insideRenderPass = true;
}

void VulkanCommandBuffer::EndRenderPass()
{
    if (renderer->dynamicRendering)
    {
        vkCmdEndRendering(commandBuffer);
    }
    else
    {
        //vkCmdEndRenderPass2(commandBuffer);
    }

    if (hasRenderPassLabel)
    {
        PopDebugGroup();
    }

    insideRenderPass = false;
}

void VulkanCommandBuffer::SetViewport(const VGPUViewport* viewport)
{
    VkViewport vkViewport;
    vkViewport.x = viewport->x;
    vkViewport.y = viewport->height - viewport->y;
    vkViewport.width = viewport->width;
    vkViewport.height = -viewport->height;
    vkViewport.minDepth = viewport->minDepth;
    vkViewport.maxDepth = viewport->maxDepth;

    vkCmdSetViewport(commandBuffer, 0, 1, &vkViewport);
}

void VulkanCommandBuffer::SetViewports(uint32_t count, const VGPUViewport* viewports)
{
    VGPU_ASSERT(count < renderer->properties2.properties.limits.maxViewports);

    // Flip viewport to match DirectX coordinate system
    VkViewport vkViewports[16];
    for (uint32_t i = 0; i < count; i++)
    {
        const VkViewport& viewport = *(const VkViewport*)&viewports[i];
        VkViewport& vkViewport = vkViewports[i];
        vkViewport = viewport;
        vkViewport.y = viewport.height - viewport.y;
        vkViewport.height = -viewport.height;
    }

    vkCmdSetViewport(commandBuffer, 0, count, vkViewports);
}

void VulkanCommandBuffer::SetScissorRect(const VGPURect* rect)
{
    vkCmdSetScissor(commandBuffer, 0, 1, (VkRect2D*)&rect);
}

void VulkanCommandBuffer::SetScissorRects(uint32_t count, const VGPURect* rects)
{
    VGPU_ASSERT(count < renderer->properties2.properties.limits.maxViewports);

    vkCmdSetScissor(commandBuffer, 0, count, (const VkRect2D*)rects);
}

void VulkanCommandBuffer::SetVertexBuffer(uint32_t index, VGPUBuffer buffer, uint64_t offset)
{
    VulkanBuffer* vulkanBuffer = (VulkanBuffer*)buffer;

    vkCmdBindVertexBuffers(commandBuffer, index, 1, &vulkanBuffer->handle, &offset);
}

void VulkanCommandBuffer::SetIndexBuffer(VGPUBuffer buffer, VGPUIndexType type, uint64_t offset)
{
    VulkanBuffer* vulkanBuffer = (VulkanBuffer*)buffer;

    const VkIndexType vkIndexType = (type == VGPUIndexType_Uint16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    vkCmdBindIndexBuffer(commandBuffer, vulkanBuffer->handle, offset, vkIndexType);
}

void VulkanCommandBuffer::SetStencilReference(uint32_t reference)
{
    vkCmdSetStencilReference(commandBuffer, VK_STENCIL_FRONT_AND_BACK, reference);
}

void VulkanCommandBuffer::BeginQuery(VGPUQueryHeap heap, uint32_t index)
{
    VulkanQueryHeap* vulkanHeap = static_cast<VulkanQueryHeap*>(heap);

    switch (vulkanHeap->type)
    {
        case VGPUQueryType_Occlusion:
            vkCmdBeginQuery(commandBuffer, vulkanHeap->handle, index, VK_QUERY_CONTROL_PRECISE_BIT);
            break;

        case VGPUQueryType_BinaryOcclusion:
            vkCmdBeginQuery(commandBuffer, vulkanHeap->handle, index, 0);
            break;

        case VGPUQueryType_PipelineStatistics:
            vkCmdBeginQuery(commandBuffer, vulkanHeap->handle, index, 0);
            break;

        default:
        case VGPUQueryType_Timestamp:
            break;
    }
}

void VulkanCommandBuffer::EndQuery(VGPUQueryHeap heap, uint32_t index)
{
    VulkanQueryHeap* vulkanHeap = static_cast<VulkanQueryHeap*>(heap);

    switch (vulkanHeap->type)
    {
        case VGPUQueryType_Timestamp:
            if (renderer->synchronization2)
            {
                vkCmdWriteTimestamp2(commandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, vulkanHeap->handle, index);
            }
            else
            {
                vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, vulkanHeap->handle, index);
            }
            break;
        case VGPUQueryType_Occlusion:
        case VGPUQueryType_BinaryOcclusion:
            vkCmdEndQuery(commandBuffer, vulkanHeap->handle, index);
            break;

        case VGPUQueryType_PipelineStatistics:
            vkCmdEndQuery(commandBuffer, vulkanHeap->handle, index);
            break;

        default:
            break;
    }
}

void VulkanCommandBuffer::ResolveQuery(VGPUQueryHeap heap, uint32_t index, uint32_t count, VGPUBuffer destinationBuffer, uint64_t destinationOffset)
{
    VulkanQueryHeap* vulkanHeap = static_cast<VulkanQueryHeap*>(heap);
    VulkanBuffer* vulkanDestBuffer = static_cast<VulkanBuffer*>(destinationBuffer);

    VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT;

    switch (vulkanHeap->type)
    {
        case VGPUQueryType_BinaryOcclusion:
            flags |= VK_QUERY_RESULT_PARTIAL_BIT;
            break;
        default:
            break;
    }

    vkCmdCopyQueryPoolResults(
        commandBuffer,
        vulkanHeap->handle,
        index,
        count,
        vulkanDestBuffer->handle,
        destinationOffset,
        vulkanHeap->resultSize,
        flags
    );
}

void VulkanCommandBuffer::ResetQuery(VGPUQueryHeap heap, uint32_t index, uint32_t count)
{
    VulkanQueryHeap* vulkanHeap = static_cast<VulkanQueryHeap*>(heap);

    vkCmdResetQueryPool(commandBuffer, vulkanHeap->handle, index, count);
}

void VulkanCommandBuffer::PrepareDraw()
{
    VGPU_ASSERT(insideRenderPass);

    FlushBindGroups();
}

void VulkanCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    PrepareDraw();

    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
{
    PrepareDraw();

    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}

void VulkanCommandBuffer::DrawIndirect(VGPUBuffer indirectBuffer, uint64_t indirectBufferOffset)
{
    VGPU_ASSERT(indirectBuffer);
    PrepareDraw();

    VulkanBuffer* backendBuffer = static_cast<VulkanBuffer*>(indirectBuffer);
    vkCmdDrawIndirect(commandBuffer, backendBuffer->handle, indirectBufferOffset, 1, (uint32_t)sizeof(VkDrawIndirectCommand));
}

void VulkanCommandBuffer::DrawIndexedIndirect(VGPUBuffer indirectBuffer, uint64_t indirectBufferOffset)
{
    VGPU_ASSERT(indirectBuffer);
    PrepareDraw();

    VulkanBuffer* backendBuffer = static_cast<VulkanBuffer*>(indirectBuffer);
    vkCmdDrawIndexedIndirect(commandBuffer, backendBuffer->handle, indirectBufferOffset, 1, sizeof(VkDrawIndexedIndirectCommand));
}


void VulkanCommandBuffer::DispatchMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
    PrepareDraw();

    vkCmdDrawMeshTasksEXT(commandBuffer, threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void VulkanCommandBuffer::DispatchMeshIndirect(VGPUBuffer indirectBuffer, uint64_t indirectBufferOffset)
{
    VGPU_ASSERT(indirectBuffer);
    PrepareDraw();

    VulkanBuffer* backendBuffer = static_cast<VulkanBuffer*>(indirectBuffer);
    vkCmdDrawMeshTasksIndirectEXT(commandBuffer, backendBuffer->handle, indirectBufferOffset, 1, sizeof(VGPUDispatchIndirectCommand));
}

void VulkanCommandBuffer::DispatchMeshIndirectCount(VGPUBuffer indirectBuffer, uint64_t indirectBufferOffset, VGPUBuffer countBuffer, uint64_t countBufferOffset, uint32_t maxCount)
{
    VGPU_ASSERT(indirectBuffer);
    VGPU_ASSERT(countBuffer);

    VulkanBuffer* vulkanIndirectBuffer = static_cast<VulkanBuffer*>(indirectBuffer);
    VulkanBuffer* vulkanCountBuffer = static_cast<VulkanBuffer*>(countBuffer);

    PrepareDraw();
    vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer,
        vulkanIndirectBuffer->handle, indirectBufferOffset,
        vulkanCountBuffer->handle, countBufferOffset,
        maxCount, sizeof(VGPUDispatchIndirectCommand));
}

/* VulkanRenderer */
VGPUCommandBuffer VulkanDevice::BeginCommandBuffer(VGPUCommandQueue queueType, const char* label)
{
    VulkanCommandBuffer* commandBuffer = nullptr;

    cmdBuffersLocker.lock();
    uint32_t cmd_current = cmdBuffersCount++;
    if (cmd_current >= commandBuffersPool.size())
    {
        commandBuffer = new VulkanCommandBuffer();
        commandBuffer->renderer = this;
        commandBuffer->queueType = queueType;

        for (uint32_t i = 0; i < VGPU_MAX_INFLIGHT_FRAMES; ++i)
        {
            VkCommandPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndices.familyIndices[queueType];

            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            //poolInfo.queueFamilyIndex = computeQueueFamily;
            VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandBuffer->commandPools[i]));

            VkCommandBufferAllocateInfo commandBufferInfo = {};
            commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferInfo.commandBufferCount = 1;
            commandBufferInfo.commandPool = commandBuffer->commandPools[i];
            commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferInfo, &commandBuffer->commandBuffers[i]));
        }

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &commandBuffer->semaphore));

        commandBuffersPool.push_back(commandBuffer);
    }
    else
    {
        commandBuffer = commandBuffersPool.back();
    }

    cmdBuffersLocker.unlock();

    // Begin recording
    commandBuffer->Begin(frameIndex, label);

    return commandBuffersPool.back();
}

void VulkanQueue::Submit(VulkanDevice* device, VkFence fence)
{
    if (queue == VK_NULL_HANDLE)
        return;

    std::scoped_lock lock(locker);

    if (device->synchronization2)
    {
        VGPU_ASSERT(submitSignalSemaphores.size() == submitSignalSemaphoreInfos.size());
        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = (uint32_t)submitWaitSemaphoreInfos.size();
        submitInfo.pWaitSemaphoreInfos = submitWaitSemaphoreInfos.data();
        submitInfo.commandBufferInfoCount = (uint32_t)submitCommandBufferInfos.size();
        submitInfo.pCommandBufferInfos = submitCommandBufferInfos.data();
        submitInfo.signalSemaphoreInfoCount = (uint32_t)submitSignalSemaphoreInfos.size();
        submitInfo.pSignalSemaphoreInfos = submitSignalSemaphoreInfos.data();

        VK_CHECK(vkQueueSubmit2(queue, 1, &submitInfo, fence));
    }
    else
    {
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = (uint32_t)submitWaitSemaphores.size();
        submitInfo.pWaitSemaphores = submitWaitSemaphores.data();
        submitInfo.pWaitDstStageMask = submitWaitStages.data();
        submitInfo.commandBufferCount = (uint32_t)submitCommandBuffers.size();
        submitInfo.pCommandBuffers = submitCommandBuffers.data();
        submitInfo.signalSemaphoreCount = (uint32_t)submitSignalSemaphores.size();
        submitInfo.pSignalSemaphores = submitSignalSemaphores.data();

        VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence));
    }

    if (!submitSwapchains.empty())
    {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = (uint32_t)submitSignalSemaphores.size();
        presentInfo.pWaitSemaphores = submitSignalSemaphores.data();
        presentInfo.swapchainCount = (uint32_t)submitSwapchains.size();
        presentInfo.pSwapchains = submitSwapchains.data();
        presentInfo.pImageIndices = submitSwapchainImageIndices.data();

        VkResult result = vkQueuePresentKHR(queue, &presentInfo);
        if (result != VK_SUCCESS)
        {
            // Handle outdated error in present
            if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                for (auto& swapchain : swapchainUpdates)
                {
                    swapchain->Update();
                }
            }
            else
            {
                VGPU_UNREACHABLE();
            }
        }
    }

    swapchainUpdates.clear();
    submitSwapchains.clear();
    submitSwapchainImageIndices.clear();
    submitWaitSemaphores.clear();
    submitWaitStages.clear();
    submitCommandBuffers.clear();
    submitSignalSemaphores.clear();
    // KHR_synchronization2
    submitWaitSemaphoreInfos.clear();
    submitSignalSemaphoreInfos.clear();
    submitCommandBufferInfos.clear();
}

uint64_t VulkanDevice::Submit(VGPUCommandBuffer* commandBuffers, uint32_t count)
{
    cmdBuffersCount = 0;

    // Submit current frame.
    {
        for (uint32_t i = 0; i < count; i += 1)
        {
            VulkanCommandBuffer* commandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffers[i]);
            VulkanQueue& queue = queues[commandBuffer->queueType];

            VkCommandBufferSubmitInfo& commandBufferSubmitInfo = queue.submitCommandBufferInfos.emplace_back();
            commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            commandBufferSubmitInfo.commandBuffer = commandBuffer->commandBuffer;

            queue.swapchainUpdates = commandBuffer->presentSwapChains;
            for (size_t j = 0; j < commandBuffer->presentSwapChains.size(); ++j)
            {
                VulkanSwapChain* swapChain = commandBuffer->presentSwapChains[j];

                queue.submitSwapchains.push_back(swapChain->handle);
                queue.submitSwapchainImageIndices.push_back(swapChain->imageIndex);

                if (synchronization2)
                {
                    VkSemaphoreSubmitInfo& waitSemaphore = queue.submitWaitSemaphoreInfos.emplace_back();
                    waitSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    waitSemaphore.semaphore = swapChain->acquireSemaphore;
                    waitSemaphore.value = 0; // not a timeline semaphore
                    waitSemaphore.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

                    VkSemaphoreSubmitInfo& signalSemaphore = queue.submitSignalSemaphoreInfos.emplace_back();
                    signalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    signalSemaphore.semaphore = swapChain->releaseSemaphore;
                    signalSemaphore.value = 0; // not a timeline semaphore

                    queue.submitSignalSemaphores.push_back(swapChain->releaseSemaphore);
                }
                else
                {
                    queue.submitWaitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    queue.submitWaitSemaphores.push_back(swapChain->acquireSemaphore);
                    queue.submitSignalSemaphores.push_back(swapChain->releaseSemaphore);
                }

                VkImageSubresourceRange range{};
                range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                range.baseMipLevel = 0;
                range.levelCount = VK_REMAINING_MIP_LEVELS;
                range.baseArrayLayer = 0;
                range.layerCount = VK_REMAINING_ARRAY_LAYERS;
                commandBuffer->InsertImageMemoryBarrier(
                    swapChain->backbufferTextures[swapChain->imageIndex]->handle,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    0,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    range);
            }

            if (commandBuffer->hasLabel)
            {
                commandBuffer->PopDebugGroup();
            }

            VK_CHECK(vkEndCommandBuffer(commandBuffer->commandBuffer));
            queue.submitCommandBuffers.push_back(commandBuffer->commandBuffer);
        }

        // Final submits with fences.
        for (uint8_t i = 0; i < _VGPUCommandQueue_Count; ++i)
        {
            queues[i].Submit(this, queues[i].frameFences[frameIndex]);
        }
    }

    frameCount++;
    frameIndex = frameCount % VGPU_MAX_INFLIGHT_FRAMES;

    // Begin new frame
    // Initiate stalling CPU when GPU is not yet finished with next frame
    if (frameCount >= VGPU_MAX_INFLIGHT_FRAMES)
    {
        for (uint8_t i = 0; i < _VGPUCommandQueue_Count; ++i)
        {
            if (queues[i].queue == VK_NULL_HANDLE)
                continue;

            VK_CHECK(vkWaitForFences(device, 1, &queues[i].frameFences[frameIndex], true, 0xFFFFFFFFFFFFFFFF));
            VK_CHECK(vkResetFences(device, 1, &queues[i].frameFences[frameIndex]));
        }
    }

    // Safe delete deferred destroys
    ProcessDeletionQueue();

    // Return current frame
    return frameCount - 1;
}

static bool vulkan_isSupported(void)
{
    static bool available_initialized = false;
    static bool available = false;

    if (available_initialized) {
        return available;
    }

    available_initialized = true;

#if defined(_WIN32)
    HMODULE module = LoadLibraryA("vulkan-1.dll");
    if (!module)
        return false;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(module, "vkGetInstanceProcAddr");
#elif defined(__APPLE__)
    void* module = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!module)
        module = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!module)
        module = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
    // Add support for using Vulkan and MoltenVK in a Framework. App store rules for iOS
    // strictly enforce no .dylib's. If they aren't found it just falls through
    if (!module)
        module = dlopen("vulkan.framework/vulkan", RTLD_NOW | RTLD_LOCAL);
    if (!module)
        module = dlopen("MoltenVK.framework/MoltenVK", RTLD_NOW | RTLD_LOCAL);
    // modern versions of macOS don't search /usr/local/lib automatically contrary to what man dlopen says
    // Vulkan SDK uses this as the system-wide installation location, so we're going to fallback to this if all else fails
    if (!module && getenv("DYLD_FALLBACK_LIBRARY_PATH") == NULL)
        module = dlopen("/usr/local/lib/libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!module)
        return false;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(module, "vkGetInstanceProcAddr");
#else
    void* module = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!module) {
        module = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    }
    if (!module) {
        return false;
    }
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(module, "vkGetInstanceProcAddr");
#endif

    GPU_FOREACH_ANONYMOUS(GPU_LOAD_ANONYMOUS);

    // We require vulkan 1.2
    uint32_t apiVersion;
    if (vkEnumerateInstanceVersion(&apiVersion) != VK_SUCCESS)
        return false;

    if (apiVersion < VK_API_VERSION_1_2)
    {
        return false;
    }

    available = true;
    return true;
}

static VGPUInstanceImpl* vulkan_CreateInstance(const VGPUInstanceDesc* desc)
{
    return nullptr;
}

static VGPUDeviceImpl* vulkan_createDevice(const VGPUDeviceDesc* desc)
{
    VulkanDevice* device = new VulkanDevice();

    if (!device->Init(desc))
    {
        delete device;
        return nullptr;
    }

    return device;
}

VGPUDriver Vulkan_Driver = {
    VGPUBackend_Vulkan,
    vulkan_isSupported,
    vulkan_CreateInstance,
    vulkan_createDevice
};

uint32_t vgpuToVkFormat(VGPUTextureFormat format)
{
    return ToVkFormat(format);
}

#else

#include "vgpu_driver.h"

uint32_t vgpuToVkFormat(VGPUTextureFormat format)
{
    VGPU_UNUSED(format);
    return 0;
}

#endif /* VGPU_VULKAN_DRIVER */
