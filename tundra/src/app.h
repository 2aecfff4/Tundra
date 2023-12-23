#pragma once
#include "core/std/unique_ptr.h"
#include "core/typedefs.h"
#include "math/math_utils.h"
#include "math/quat.h"
#include "math/transform.h"
#include "math/vector2.h"
#include "rhi/resources/handle.h"
#include "rhi/rhi_context.h"
#include "rhi/rhi_module.h"

struct GLFWwindow;

namespace tundra {

struct YawPitch {
    f32 yaw_degrees = 0.f;
    f32 pitch_degrees = 0.f;

    void rotate_yaw_pitch(const f32 yaw_degrees, const f32 pitch_degrees) noexcept
    {
        this->yaw_degrees = std::fmod(this->yaw_degrees + yaw_degrees, 720.f);
        this->pitch_degrees = math::clamp(
            this->pitch_degrees + pitch_degrees, -90.0f, 90.0f);
    }

    [[nodiscard]] math::Quat rotation() const noexcept
    {
        return math::Quat::from_angle(
            math::to_radians(math::Vec3 { pitch_degrees, yaw_degrees, 0 }));
    }
};

///
struct Camera {
    math::Mat4 view = math::Mat4 { 1.f };
    math::Mat4 projection;
    f32 near_plane;

    YawPitch yaw_pitch;
    math::Vec3 position = math::Vec3 {};

    void translate(const math::Vec3 move_vec) noexcept
    {
        position += move_vec;
    }

    void rotate_yaw_pitch(const f32 yaw_degrees, const f32 pitch_degrees) noexcept
    {
        yaw_pitch.rotate_yaw_pitch(yaw_degrees, pitch_degrees);
    }

    void update() noexcept
    {
        const math::Quat rotation = yaw_pitch.rotation();

        const auto transform = math::Transform {
            .rotation = rotation,
            .position = position,
            .scale = 1.f,
        };

        view = math::Mat4::look_at(
            transform.position, transform.position + transform.forward(), transform.up());
    }

    [[nodiscard]] math::Transform transform() const noexcept
    {
        const math::Quat rotation = yaw_pitch.rotation();
        return math::Transform {
            .rotation = rotation,
            .position = position,
            .scale = 1.f,
        };
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
    math::Vec2 m_mouse_previous_pos = {};

protected:
    Camera m_camera;

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
    virtual void on_key_press([[maybe_unused]] int key, [[maybe_unused]] int action)
    {
    }

protected:
    void set_window_name(const std::string& window_name) noexcept;

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
