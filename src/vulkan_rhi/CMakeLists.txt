cmake_minimum_required(VERSION 3.20)
project(vulkan_rhi VERSION 1.0.0 LANGUAGES CXX)

# ######################################################
# Files
set(PUBLIC_HDRS
)

set(PRIVATE_HDRS
    src/commands/vulkan_barrier.h
    src/commands/vulkan_command_decoder.h
    src/commands/vulkan_submit_work_scheduler.h

    src/loader/extensions/ext/debug_utils.h
    src/loader/extensions/khr/android_surface.h
    src/loader/extensions/khr/surface.h
    src/loader/extensions/khr/swapchain.h
    src/loader/extensions/khr/wayland_surface.h
    src/loader/extensions/khr/win32_surface.h
    src/loader/extensions/khr/xcb_surface.h
    src/loader/extensions/khr/xlib_surface.h
    src/loader/device.h
    src/loader/entry.h
    src/loader/instance.h

    src/managers/managers.h
    src/managers/vulkan_command_buffer_manager.h
    src/managers/vulkan_descriptor_bindless_manager.h
    src/managers/vulkan_pipeline_cache_manager.h
    src/managers/vulkan_pipeline_layout_manager.h

    src/resources/vulkan_buffer.h
    src/resources/vulkan_compute_pipeline.h
    src/resources/vulkan_graphics_pipeline.h
    src/resources/vulkan_sampler.h
    src/resources/vulkan_shader.h
    src/resources/vulkan_swapchain.h
    src/resources/vulkan_texture_view.h
    src/resources/vulkan_texture.h

    src/vulkan_allocator.h
    src/vulkan_config.h
    src/vulkan_context.h
    src/vulkan_device.h
    src/vulkan_instance.h
    src/vulkan_helpers.h
    src/vulkan_rhi_context.h
    src/vulkan_utils.h
)

set(SRC
    src/commands/vulkan_barrier.cpp
    src/commands/vulkan_command_decoder.cpp
    src/commands/vulkan_submit_work_scheduler.cpp

    src/loader/extensions/ext/debug_utils.cpp
    src/loader/extensions/khr/android_surface.cpp
    src/loader/extensions/khr/surface.cpp
    src/loader/extensions/khr/swapchain.cpp
    src/loader/extensions/khr/wayland_surface.cpp
    src/loader/extensions/khr/win32_surface.cpp
    src/loader/extensions/khr/xcb_surface.cpp
    src/loader/extensions/khr/xlib_surface.cpp
    src/loader/device.cpp
    src/loader/entry.cpp
    src/loader/instance.cpp

    src/managers/managers.cpp
    src/managers/vulkan_command_buffer_manager.cpp
    src/managers/vulkan_descriptor_bindless_manager.cpp
    src/managers/vulkan_pipeline_cache_manager.cpp
    src/managers/vulkan_pipeline_layout_manager.cpp

    src/resources/vulkan_buffer.cpp
    src/resources/vulkan_compute_pipeline.cpp
    src/resources/vulkan_graphics_pipeline.cpp
    src/resources/vulkan_sampler.cpp
    src/resources/vulkan_shader.cpp
    src/resources/vulkan_swapchain.cpp
    src/resources/vulkan_texture_view.cpp
    src/resources/vulkan_texture.cpp

    src/vulkan_allocator.cpp
    src/vulkan_context.cpp
    src/vulkan_rhi_module.cpp
    src/vulkan_device.cpp
    src/vulkan_helpers.cpp
    src/vulkan_instance.cpp
    src/vulkan_rhi_context.cpp
    src/vulkan_utils.cpp
)

# ######################################################
# Dependencies
set(PUBLIC_DEPENDENCIES
)

set(PRIVATE_DEPENDENCIES
    core
    rhi
    vk_mem_alloc
    vulkan_headers
)

tndr_add_module(vulkan_rhi
    SOURCES ${SRC} ${PUBLIC_HDRS} ${PRIVATE_HDRS}
    PRIVATE_DEPENDENCIES ${PRIVATE_DEPENDENCIES}
    PUBLIC_DEPENDENCIES ${PUBLIC_DEPENDENCIES}
)
