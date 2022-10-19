#pragma once
#include "core/std/unique_ptr.h"
#include "math/transform.h"
#include "math/vector2.h"
#include "rhi/resources/handle.h"
#include "rhi/rhi_context.h"
#include "rhi/rhi_module.h"

struct GLFWwindow;

namespace tundra {

///
struct Camera {
    math::Mat4 view = math::Mat4 { 1.f };
    math::Mat4 projection;
    f32 near_plane;

    void update(const math::Transform& transform) noexcept
    {
        view = math::Mat4::look_at(
            transform.position, transform.position + transform.forward(), transform.up());
    }
};

///
class App {
private:
    static constexpr math::IVec2 DEFAULT_WINDOW_SIZE = math::IVec2 { 1337, 768 };

private:
    GLFWwindow* m_window = nullptr;
    rhi::SwapchainHandle m_swapchain;
    math::IVec2 m_window_surface_size = {};
    math::Vec2 m_mouse_delta = {};

protected:
    Camera m_camera;
    math::Transform m_camera_transform;

private:
    u64 m_frame_counter = 0;

public:
    App() noexcept;
    virtual ~App() noexcept;

private:
    void create_window(const math::IVec2& window_size) noexcept;
    void destroy_window() noexcept;

public:
    void loop() noexcept;

protected:
    virtual void tick(const f32 delta_time) noexcept = 0;

protected:
    [[nodiscard]] rhi::SwapchainHandle get_swapchain() const noexcept;
    [[nodiscard]] math::IVec2 get_window_surface_size() const noexcept;

private:
    static void window_size_callback(GLFWwindow* const window, int, int);
    static void key_callback(
        GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
};

} // namespace tundra
