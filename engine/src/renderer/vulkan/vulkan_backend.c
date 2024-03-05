#include "vulkan_backend.h"

#include "vulkan_types.h"

#include "core/logger.h"

// Static Vulkan Context
static vulkan_context context;

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
    create_info.enabledExtensionCount = 0;
    create_info.ppEnabledExtensionNames = 0;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = 0;

    VkResult result = vkCreateInstance(&create_info, context.allocator, &context.instance);
    if(result != VK_SUCCESS)
    {
        KERROR("vkCreateInstance failed with the result: %u", result);
        return FALSE;
    }

    KINFO("Vulkan renderer initialized successfully");
    return TRUE;
}

void vulkan_renderer_backend_shutdown(renderer_backend* backend)
{

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