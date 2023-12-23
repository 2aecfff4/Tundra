#include "app.h"
#include "core/module/module_manager.h"
#include "core/profiler.h"
#include "core/std/timer.h"
#include "core/std/unique_ptr.h"
#include "globals/globals.h"
#include "math/math_utils.h"
#include "math/matrix4.h"
#include "math/transform.h"
#include "math/vector2.h"
#include "rhi/resources/buffer.h"
#include "rhi/validation_layers.h"
#include <iostream>

#if TNDR_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace tundra {

//////////////////////////////////////////////////////////////////////////
// App

static constexpr f32 Z_NEAR = 0.0001;
static constexpr f32 FOV = 70.f;

App::App() noexcept
{
    this->create_window(DEFAULT_WINDOW_SIZE);
    m_camera.projection = math::Mat4::perspective_infinite(
        math::to_radians(FOV),
        DEFAULT_WINDOW_SIZE.x / float(DEFAULT_WINDOW_SIZE.y),
        Z_NEAR);

    m_camera.near_plane = Z_NEAR;
}

App::~App() noexcept
{
    this->destroy_window();
}

void App::create_window(const math::IVec2& window_size) noexcept
{
    m_window = glfwCreateWindow(window_size.x, window_size.y, "tundra", nullptr, nullptr);
    glfwSetWindowSizeLimits(m_window, 480, 320, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetWindowUserPointer(m_window, this);
    App::window_size_callback(m_window, window_size.x, window_size.y);
    glfwSetWindowSizeCallback(m_window, App::window_size_callback);
    glfwSetKeyCallback(m_window, App::key_callback);
    glfwSetCursorPosCallback(m_window, App::cursor_pos_callback);
}

void App::destroy_window() noexcept
{
    glfwDestroyWindow(m_window);
    m_window = nullptr;
    globals::g_rhi_context->destroy_swapchain(m_swapchain);
    m_swapchain = rhi::SwapchainHandle {};
}

void App::loop() noexcept
{
    core::Timer timer;
    while (!glfwWindowShouldClose(m_window)) {
        TNDR_PROFILER_TRACE("App::loop::tick");

        glfwPollEvents();

        const f32 delta_time = timer.get_delta_time();
        timer.tick();
        {
            TNDR_PROFILER_TRACE("App::loop::tick");
            m_camera.update();
            this->tick(delta_time);
        }

        m_frame_counter += 1;
    }
}

void App::set_window_name(const std::string& window_name) noexcept
{
    glfwSetWindowTitle(m_window, window_name.c_str());
}

rhi::SwapchainHandle App::get_swapchain() const noexcept
{
    return m_swapchain;
}

math::IVec2 App::get_window_surface_size() const noexcept
{
    return m_window_surface_size;
}

void App::window_size_callback(GLFWwindow* const window, int, int)
{
    App* const app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (app->m_swapchain.is_valid()) {
        globals::g_rhi_context->destroy_swapchain(app->m_swapchain);

        const HWND window_native = glfwGetWin32Window(window);
        app->m_swapchain = globals::g_rhi_context->create_swapchain(
            rhi::SwapchainCreateInfo {
                .window_handle =
                    platform::Win32Handle {
                        .hwnd = window_native,
                        .hinstance = nullptr,
                    },
            });
    } else {
        const HWND window_native = glfwGetWin32Window(window);
        app->m_swapchain = globals::g_rhi_context->create_swapchain(
            rhi::SwapchainCreateInfo {
                .window_handle =
                    platform::Win32Handle {
                        .hwnd = window_native,
                        .hinstance = nullptr,
                    },
            });
    }

    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);
    app->m_window_surface_size = math::IVec2 { width, height };

    app->m_camera.projection = math::Mat4::perspective_infinite(
        math::to_radians(FOV),
        app->m_window_surface_size.x / float(app->m_window_surface_size.y),
        Z_NEAR);

    // app->m_camera.projection = perspective(
    //     math::to_radians(120.f),
    //     DEFAULT_WINDOW_SIZE.x / float(DEFAULT_WINDOW_SIZE.y),
    //     Z_NEAR,
    //     10000.f);
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    App* const app = static_cast<App*>(glfwGetWindowUserPointer(window));

    const f32 speed = 0.2f;
    if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
        const auto transform = app->m_camera.transform();
        switch (key) {
            case GLFW_KEY_W: {
                app->m_camera.translate(transform.forward() * math::Vec3 { speed });
                break;
            }
            case GLFW_KEY_S: {
                app->m_camera.translate(transform.forward() * math::Vec3 { -speed });
                break;
            }
            case GLFW_KEY_A: {
                app->m_camera.translate(transform.right() * math::Vec3 { -speed });
                break;
            }
            case GLFW_KEY_D: {
                app->m_camera.translate(transform.right() * math::Vec3 { speed });
                break;
            }
            case GLFW_KEY_Q: {
                app->m_camera.translate(math::Transform::UP * math::Vec3 { -speed });
                break;
            }
            case GLFW_KEY_E: {
                app->m_camera.translate(math::Transform::UP * math::Vec3 { speed });
                break;
            }
            default: {
                break;
            }
        }
    }

    app->on_key_press(key, action);
}

void App::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    App* const app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->m_mouse_delta = app->m_mouse_previous_pos - math::Vec2 { xpos, ypos };
    app->m_mouse_previous_pos = math::Vec2 { xpos, ypos };

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        const math::Vec2 delta = app->m_mouse_delta;
        app->m_camera.rotate_yaw_pitch(0.1f * delta.x, 0.1f * delta.y);
    }
}

} // namespace tundra
