#include "vulkan_backend.h"

#include "vulkan_types.h"
#include "vulkan_platform.h"
#include "vulkan_device.h"

#include "core/logger.h"
#include "core/kstring.h"

#include "containers/darray.h"


// Static Vulkan Context
static vulkan_context context;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data
);


b8 vulkan_renderer_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state)
{
    // TODO: Custom Allocator
    context.allocator = 0;

    // Setup Vulkan instance.
    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO}; // initializer list  but only the first field sType is supplied.
    app_info.apiVersion = VK_API_VERSION_1_2;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "KoEngine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;

    // Obtain a list of required extensions
    const char** required_extensions = darray_create(const char*);
    darray_push(required_extensions, &VK_KHR_SURFACE_EXTENSION_NAME); // Generic surface extension
    platform_get_required_extension_names(&required_extensions); // Platform-specific extension(s)
#if defined(_DEBUG)
    darray_push(required_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // debug utilities

    KDEBUG("Required extensions:");
    u32 length = darray_length(required_extensions);
    for(u32 i = 0; i < length; ++i)
    {
        KDEBUG(required_extensions[i]);
    }
#endif

    create_info.enabledExtensionCount = darray_length(required_extensions);
    create_info.ppEnabledExtensionNames = required_extensions;

    // Validation Layers
    const char** required_validation_layer_names = 0;
    u32 required_validation_layer_count = 0;

 // If validation should be done, get a list of the required validation layert names
// and make sure they exist. Validation layers should only be enabled on non-release builds.
#if defined(_DEBUG)
    KINFO("Validation layers enabled. Enumerating...");

    // The list of validation layers required.
    required_validation_layer_names = darray_create(const char*);
    darray_push(required_validation_layer_names, &"VK_LAYER_KHRONOS_validation");
    required_validation_layer_count = darray_length(required_validation_layer_names);

    // Obtain a list of available validation layers
    u32 available_layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
    VkLayerProperties* available_layers = darray_reserve(VkLayerProperties, available_layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

    // Verify all required layers are available.
    for (u32 i = 0; i < required_validation_layer_count; ++i) 
    {
        KINFO("Searching for layer: %s...", required_validation_layer_names[i]);
        b8 found = FALSE;
        for (u32 j = 0; j < available_layer_count; ++j) 
        {
            if (strings_equal(required_validation_layer_names[i], available_layers[j].layerName)) 
            {
                found = TRUE;
                KINFO("Found.");
                break;
            }
        }

        if (!found) 
        {
            KFATAL("Required validation layer is missing: %s", required_validation_layer_names[i]);
            return FALSE;
        }
    }
    KINFO("All required validation layers are present.");
    // Clean-up
    darray_destroy(available_layers);
#endif

    create_info.enabledLayerCount = required_validation_layer_count;
    create_info.ppEnabledLayerNames = required_validation_layer_names;

    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
    KINFO("Vulkan Instance created.");

    // Debugger
#if defined(_DEBUG)
    KDEBUG("Creating Vulkan debugger...");
    // What severity levels to log
    u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
                                                                      //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = log_severity; 
    // Allows us to filter what message types we want
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    // Handler for any debug message events
    debug_create_info.pfnUserCallback = vk_debug_callback;

    // As this is an extension, it is not loaded automatically by Vulkan. So, we need to load a function pointer to it.
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    KASSERT_MSG(func, "Failed to create debug messenger!");
    VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
    KDEBUG("Vulkan debugger created.");
#endif

    // Surface
    KDEBUG("Creating Vulkan Surface...");
    if(!platform_create_vulkan_surface(plat_state, &context))
    {
        KERROR("Failed to create platform surface!");
        return FALSE;
    }
    KDEBUG("Vulkan Surface created.");

    // Device creation
    if(!vulkan_device_create(&context))
    {
        KERROR("Failed to create device!");
        return FALSE;
    }

    KINFO("Vulkan renderer initialized successfully");
    // Clean-up
    darray_destroy(required_extensions);
    darray_destroy(required_validation_layer_names);

    return TRUE;
}

void vulkan_renderer_backend_shutdown(renderer_backend* backend)
{
    // Destroying resources in the opposite order that we created them.
    KDEBUG("Destroying Vulkan debugger...");
    if (context.debug_messenger) 
    {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debug_messenger, context.allocator);
    }

    KDEBUG("Destroying Vulkan Instance...");
    vkDestroyInstance(context.instance, context.allocator);
}

void vulkan_renderer_backend_on_resized(renderer_backend* backend, u16 width, u16 height)
{

}

b8 vulkan_renderer_backend_begin_frame(renderer_backend* backend, f32 delta_time)
{
    return TRUE;
}

b8 vulkan_renderer_backend_end_frame(renderer_backend* backend, f32 delta_time)
{
    return TRUE;
}


VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data
)
{
    switch(message_severity) 
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            KERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            KWARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            KINFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            KTRACE(callback_data->pMessage);
            break;
        default:
            break;
    }

    // Per the spec, we always need to return VK_FALSE for this callback. Idk why.
    return VK_FALSE;
}