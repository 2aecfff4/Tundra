#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/hash.h"
#include "core/std/option.h"
#include "core/std/variant.h"
#include "core/utils/enum_flags.h"
#include "math/math_hash.h"
#include "math/vector4.h"
#include "rhi/resources/access_flags.h"
#include "rhi/resources/texture.h"

namespace tundra::rhi {

/// Specify how contents of an attachment are treated at the beginning of a subpass.
enum class AttachmentLoadOp : u8 {
    /// Preserve existing content in the attachment.
    Load,
    /// Clear the attachment.
    Clear,
    /// The previous contents within the area need not be preserved.
    /// The contents of the attachment will be undefined inside the render area.
    DontCare,
};

/// Specify how contents of an attachment are treated at the end of a subpass.
enum class AttachmentStoreOp : u8 {
    /// Content written to the attachment will be preserved.
    Store,
    /// The contents within the render area are not needed after rendering,
    /// and **may** be discarded.
    DontCare,
};

///
struct RHI_API AttachmentOps {
    /// Specify how contents of an attachment are treated at the beginning of a subpass.
    AttachmentLoadOp load;
    /// Specify how contents of an attachment are treated at the end of a subpass.
    AttachmentStoreOp store;

    /// Specifies `AttachmentLoadOp::DontCare` for load op and `AttachmentStoreOp::DontCare` for store op.
    static const AttachmentOps DONT_CARE;
    /// Specifies `AttachmentLoadOp::Clear` for load op and `AttachmentStoreOp::Store`
    /// for store op.
    static const AttachmentOps INIT;
    /// Specifies `AttachmentLoadOp::Load` for load op and `AttachmentStoreOp::Store`
    /// for store op.
    static const AttachmentOps PRESERVE;

    [[nodiscard]] bool operator==(const AttachmentOps&) const noexcept = default;
    [[nodiscard]] bool operator!=(const AttachmentOps&) const noexcept = default;
};

///
using AttachmentTexture = core::Variant<TextureHandleType, TextureViewHandleType>;

///
using ClearValue = core::Variant<math::Vec4, math::IVec4, math::UVec4>;

///
struct RHI_API ClearDepthStencil {
    /// Depth value
    f32 depth = 0;
    /// Stencil value
    u32 stencil = 0;

    [[nodiscard]] bool operator==(const ClearDepthStencil&) const noexcept = default;
    [[nodiscard]] bool operator!=(const ClearDepthStencil&) const noexcept = default;
};

///
struct RHI_API ResolveTexture {
    /// Current texture access.
    /// `AccessFlags` are used to determine the correct layouts for a texture.
    AccessFlags texture_access;
    /// The texture used to write resolved multisample data at the end of rendering.
    AttachmentTexture resolve_texture;

    [[nodiscard]] bool operator==(const ResolveTexture&) const noexcept = default;
    [[nodiscard]] bool operator!=(const ResolveTexture&) const noexcept = default;
};

///
struct RHI_API ColorAttachment {
    /// Load and store operations of the attachment.
    AttachmentOps ops;
    /// Current texture access.
    /// `AccessFlags` are used to determine the correct layouts for a texture.
    /// Must be set to `AccessFlags::GENERAL`, `AccessFlags::COLOR_ATTACHMENT` or
    /// `AccessFlags::COLOR_ATTACHMENT_WRITE`.
    AccessFlags texture_access;
    /// The texture that will be used for rendering.
    AttachmentTexture texture;
    /// The texture used to write resolved multisample data at the end of rendering.
    core::Option<ResolveTexture> resolve_texture;
    /// Defines values used to clear texture when `AttachmentOps::load` is equal
    /// to `AttachmentLoadOp::Clear`.
    ClearValue clear_value;

    [[nodiscard]] bool operator==(const ColorAttachment&) const noexcept = default;
    [[nodiscard]] bool operator!=(const ColorAttachment&) const noexcept = default;
};

///
struct RHI_API DepthStencilAttachment {
    /// Load and store operations of the attachment.
    AttachmentOps ops;
    /// Load and store operations of the stencil.
    AttachmentOps stencil_ops;
    /// Current texture access.
    /// `AccessFlags` are used to determine the correct layouts for a texture.
    /// Must be set to `AccessFlags::GENERAL`, `AccessFlags::DEPTH_STENCIL_ATTACHMENT` or
    /// `AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE`.
    AccessFlags texture_access;
    /// The texture that will be used for rendering.
    AttachmentTexture texture;
    /// The texture used to write resolved multisample data at the end of rendering.
    core::Option<ResolveTexture> resolve_texture;
    /// Defines values used to clear texture when `AttachmentOps::load` is equal
    /// to `AttachmentLoadOp::Clear`.
    ClearDepthStencil clear_value;

    [[nodiscard]] bool operator==(const DepthStencilAttachment&) const noexcept = default;
    [[nodiscard]] bool operator!=(const DepthStencilAttachment&) const noexcept = default;
};

///
struct RHI_API RenderPass {
    /// Number of attachments must be less or equal to `MAX_NUM_COLOR_ATTACHMENTS`.
    core::Array<ColorAttachment> color_attachments;
    core::Option<DepthStencilAttachment> depth_stencil_attachment;

    [[nodiscard]] bool operator==(const RenderPass&) const noexcept = default;
    [[nodiscard]] bool operator!=(const RenderPass&) const noexcept = default;
};

} // namespace tundra::rhi

namespace tundra::core {

template <>
struct Hash<rhi::AttachmentOps> {
    [[nodiscard]] usize operator()(const rhi::AttachmentOps& ops) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, ops.load);
        core::hash_and_combine(seed, ops.store);
        return seed;
    }
};

template <>
struct Hash<rhi::ClearDepthStencil> {
    [[nodiscard]] usize operator()(
        const rhi::ClearDepthStencil& clear_depth_stencil) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, clear_depth_stencil.depth);
        core::hash_and_combine(seed, clear_depth_stencil.stencil);
        return seed;
    }
};

template <>
struct Hash<rhi::DepthStencilAttachment> {
    [[nodiscard]] usize operator()(
        const rhi::DepthStencilAttachment& attachment) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, attachment.ops);
        core::hash_and_combine(seed, attachment.stencil_ops);
        core::hash_and_combine(seed, attachment.texture_access);
        core::hash_and_combine(seed, attachment.texture);
        core::hash_and_combine(seed, attachment.resolve_texture);
        core::hash_and_combine(seed, attachment.clear_value);
        return seed;
    }
};

template <>
struct Hash<rhi::ResolveTexture> {
    [[nodiscard]] usize operator()(
        const rhi::ResolveTexture& resolve_texture) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, resolve_texture.texture_access);
        core::hash_and_combine(seed, resolve_texture.resolve_texture);
        return seed;
    }
};

template <>
struct Hash<rhi::ColorAttachment> {
    [[nodiscard]] usize operator()(const rhi::ColorAttachment& attachment) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, attachment.ops);
        core::hash_and_combine(seed, attachment.texture_access);
        core::hash_and_combine(seed, attachment.texture);
        core::hash_and_combine(seed, attachment.resolve_texture);
        core::hash_and_combine(seed, attachment.clear_value);
        return seed;
    }
};

template <>
struct Hash<rhi::RenderPass> {
    [[nodiscard]] usize operator()(const rhi::RenderPass& render_pass) const noexcept
    {
        usize seed = core::hash_range(
            render_pass.color_attachments.begin(), render_pass.color_attachments.end());
        core::hash_and_combine(seed, render_pass.depth_stencil_attachment);
        return seed;
    }
};

} // namespace tundra::core
