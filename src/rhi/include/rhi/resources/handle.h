#pragma once
#include "core/core.h"
#include "core/std/assert.h"
#include "core/std/hash.h"

namespace tundra::rhi {

///
/// | type   |                 index                  |   generation   |
/// |--------|----------------------------------------|----------------|
/// |   8    |                  40                    |       16       |
template <u64 handle_type>
class Handle {
public:
    static constexpr u64 INDEX_MASK = 0x00FF'FFFF'FFFF'0000;
    static constexpr u64 INDEX_SHIFT = 16;
    static constexpr u64 GENERATION_MASK = 0x0000'0000'0000'FFFF;
    static constexpr u64 GENERATION_SHIFT = 0;
    static constexpr u64 MAX_GENERATION = GENERATION_MASK;
    static constexpr u64 HANDLE_TYPE_MASK = 0xFF00'0000'0000'0000;
    static constexpr u64 HANDLE_TYPE_SHIFT = 56;
    static constexpr u64 NULL_HANDLE = INDEX_MASK | GENERATION_MASK;

private:
    u64 m_id = NULL_HANDLE;

public:
    constexpr Handle() noexcept = default;

    constexpr Handle(const u64 index, const u64 generation) noexcept
    {
        tndr_debug_assert(index < (INDEX_MASK >> INDEX_SHIFT), "");
        tndr_debug_assert(generation < (GENERATION_MASK >> GENERATION_SHIFT), "");

        m_id = (handle_type << HANDLE_TYPE_SHIFT) | (index << INDEX_SHIFT) |
               (generation << GENERATION_SHIFT);
    }

    explicit constexpr Handle(const u64 id) noexcept
        : m_id(id)
    {
    }

public:
    [[nodiscard]] constexpr bool is_null() const noexcept
    {
        return (m_id & (~HANDLE_TYPE_MASK)) == NULL_HANDLE;
    }

    [[nodiscard]] constexpr u64 get_id() const noexcept
    {
        return m_id;
    }

    [[nodiscard]] constexpr u64 get_generation() const noexcept
    {
        return (m_id & GENERATION_MASK) >> GENERATION_SHIFT;
    }

    [[nodiscard]] constexpr u64 get_index() const noexcept
    {
        return (m_id & INDEX_MASK) >> INDEX_SHIFT;
    }

    [[nodiscard]] constexpr u64 get_handle_type() const noexcept
    {
        return (m_id & HANDLE_TYPE_MASK) >> HANDLE_TYPE_SHIFT;
    }

public:
    constexpr bool operator==(const Handle&) const noexcept = default;
    constexpr bool operator!=(const Handle&) const noexcept = default;
};

///
struct BindableResource {
    static constexpr u32 INVALID_INDEX = (u64(1) << 32) - 1;
    u32 bindless_srv = INVALID_INDEX;
    u32 bindless_uav = INVALID_INDEX;

public:
    constexpr bool operator==(const BindableResource&) const noexcept = default;
    constexpr bool operator!=(const BindableResource&) const noexcept = default;
};

///
enum class HandleType : u64 {
    Buffer,
    Shader,
    ComputePipeline,
    GraphicsPipeline,
    Swapchain,
    Texture,
    TextureView,
    Sampler,
};

namespace handle_impl {

///
template <typename T>
class BindableHandleImpl {
private:
    T m_handle;
    BindableResource m_bindings;

public:
    constexpr BindableHandleImpl() noexcept = default;
    constexpr BindableHandleImpl(const T handle, const BindableResource bindings) noexcept
        : m_handle(handle)
        , m_bindings(bindings)
    {
    }

public:
    [[nodiscard]] constexpr T get_handle() const noexcept
    {
        return m_handle;
    }

    [[nodiscard]] constexpr BindableResource get_bindings() const noexcept
    {
        return m_bindings;
    }

    [[nodiscard]] constexpr u32 get_srv() const noexcept
    {
        return m_bindings.bindless_srv;
    }

    [[nodiscard]] constexpr u32 get_uav() const noexcept
    {
        return m_bindings.bindless_uav;
    }

    [[nodiscard]] constexpr bool is_null() const noexcept
    {
        return m_handle.is_null();
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return !m_handle.is_null();
    }

public:
    constexpr bool operator==(const BindableHandleImpl&) const noexcept = default;
    constexpr bool operator!=(const BindableHandleImpl&) const noexcept = default;
};

///
template <typename T>
class HandleImpl {
private:
    T m_handle;

public:
    constexpr HandleImpl() noexcept = default;
    constexpr HandleImpl(const T handle) noexcept
        : m_handle(handle)
    {
    }

public:
    [[nodiscard]] constexpr T get_handle() const noexcept
    {
        return m_handle;
    }

    [[nodiscard]] constexpr bool is_null() const noexcept
    {
        return m_handle.is_null();
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return !m_handle.is_null();
    }

public:
    constexpr bool operator==(const HandleImpl&) const noexcept = default;
    constexpr bool operator!=(const HandleImpl&) const noexcept = default;
};

} // namespace handle_impl

#define TNDR_BINDABLE_HANDLE(id, handle_type_name, type)                                 \
    using handle_type_name = Handle<static_cast<u64>(type)>;                             \
    using id = handle_impl::BindableHandleImpl<handle_type_name>;

#define TNDR_HANDLE(id, handle_type_name, type)                                          \
    using handle_type_name = Handle<static_cast<u64>(type)>;                             \
    using id = handle_impl::HandleImpl<handle_type_name>;

TNDR_BINDABLE_HANDLE(BufferHandle, BufferHandleType, HandleType::Buffer)
TNDR_BINDABLE_HANDLE(TextureHandle, TextureHandleType, HandleType::Texture)
TNDR_BINDABLE_HANDLE(TextureViewHandle, TextureViewHandleType, HandleType::TextureView)
TNDR_BINDABLE_HANDLE(SamplerHandle, SamplerHandleType, HandleType::Sampler)

TNDR_HANDLE(ShaderHandle, ShaderHandleType, HandleType::Shader)
TNDR_HANDLE(ComputePipelineHandle, ComputePipelineHandleType, HandleType::ComputePipeline)
TNDR_HANDLE(
    GraphicsPipelineHandle, GraphicsPipelineHandleType, HandleType::GraphicsPipeline)
TNDR_HANDLE(SwapchainHandle, SwapchainHandleType, HandleType::Swapchain)

#undef TNDR_BINDABLE_HANDLE
#undef TNDR_HANDLE

} // namespace tundra::rhi

namespace tundra::core {

template <u64 handle_type>
struct Hash<rhi::Handle<handle_type>> {
    [[nodiscard]] usize operator()(const rhi::Handle<handle_type>& handle) const noexcept
    {
        using T = decltype(handle.get_id());
        return core::Hash<T> {}(handle.get_id());
    }
};

} // namespace tundra::core
