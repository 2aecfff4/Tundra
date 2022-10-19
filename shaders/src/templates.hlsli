#ifndef TNDR_SHADERS_TEMPLATES_H
#define TNDR_SHADERS_TEMPLATES_H

#include "bindings.hlsli"

namespace tundra {

///
template <typename T>
T load_ubo()
{
    return g_buffers[g_push_constant.buffer_index].Load<T>(g_push_constant.buffer_offset);
}

///
template <bool non_uniform, typename T>
void buffer_store(
    const uint buffer_index, const uint offset, const uint index, const T value)
{
    if (non_uniform) {
        g_rw_buffers[NonUniformResourceIndex(buffer_index)].Store<T>(
            offset + (index * sizeof(T)), value);
    } else {
        g_rw_buffers[buffer_index].Store<T>(offset + (index * sizeof(T)), value);
    }
}

///
template <bool non_uniform, typename T>
void buffer_store_raw(const uint buffer_index, const uint offset, const T value)
{
    if (non_uniform) {
        g_rw_buffers[NonUniformResourceIndex(buffer_index)].Store<T>(offset, value);
    } else {
        g_rw_buffers[buffer_index].Store<T>(offset, value);
    }
}

///
template <bool non_uniform, typename T>
T buffer_load(const uint buffer_index, const uint offset, const uint index)
{
    if (non_uniform) {
        return g_buffers[NonUniformResourceIndex(buffer_index)].Load<T>(
            offset + (index * sizeof(T)));
    } else {
        return g_buffers[buffer_index].Load<T>(offset + (index * sizeof(T)));
    }
}

///
template <bool non_uniform, typename T>
T buffer_load_raw(const uint buffer_index, const uint offset)
{
    if (non_uniform) {
        return g_buffers[NonUniformResourceIndex(buffer_index)].Load<T>(offset);
    } else {
        return g_buffers[buffer_index].Load<T>(offset);
    }
}

///
template <bool non_uniform>
void buffer_interlocked_add(
    const uint buffer_index, const uint offset, const uint value, out uint original_value)
{
    if (non_uniform) {
        g_rw_buffers[NonUniformResourceIndex(buffer_index)].InterlockedAdd(
            offset, value, original_value);
    } else {
        g_rw_buffers[buffer_index].InterlockedAdd(offset, value, original_value);
    }
}

///
template <bool non_uniform>
void buffer_interlocked_max(
    const uint buffer_index, const uint offset, const uint value, out uint original_value)
{
    if (non_uniform) {
        g_rw_buffers[NonUniformResourceIndex(buffer_index)].InterlockedMax(
            offset, value, original_value);
    } else {
        g_rw_buffers[buffer_index].InterlockedMax(offset, value, original_value);
    }
}

///////////////////////////////////////////////////////////////////////////////////
// Textures

#define GET_TEXTURE(type, value_type)                                                    \
    Texture##type<value_type> get_texture_##type##_##value_type(                         \
        const uint texture_index)                                                        \
    {                                                                                    \
        return g_textures##type##_##value_type[NonUniformResourceIndex(texture_index)];  \
    }

#define GET_RW_TEXTURE(type, value_type)                                                 \
    RWTexture##type<value_type> get_rw_texture_##type##_##value_type(                    \
        const uint texture_index)                                                        \
    {                                                                                    \
        return g_rw_textures##type##_##value_type[NonUniformResourceIndex(               \
            texture_index)];                                                             \
    }

#define GET_TEXTURE_WITHOUT_TYPE(type)                                                   \
    Texture##type get_texture_##type(const uint texture_index)                           \
    {                                                                                    \
        return g_textures##type[NonUniformResourceIndex(texture_index)];                 \
    }

GET_TEXTURE_WITHOUT_TYPE(1D)
GET_TEXTURE_WITHOUT_TYPE(2D)
GET_TEXTURE_WITHOUT_TYPE(3D)

GET_TEXTURE(1D, float)
GET_TEXTURE(1D, float2)
GET_TEXTURE(1D, float4)
GET_TEXTURE(2D, float)
GET_TEXTURE(2D, float2)
GET_TEXTURE(2D, float4)
GET_TEXTURE(3D, float)
GET_TEXTURE(3D, float2)
GET_TEXTURE(3D, float4)

GET_TEXTURE(1D, int)
GET_TEXTURE(1D, int2)
GET_TEXTURE(1D, int4)
GET_TEXTURE(2D, int)
GET_TEXTURE(2D, int2)
GET_TEXTURE(2D, int4)
GET_TEXTURE(3D, int)
GET_TEXTURE(3D, int2)
GET_TEXTURE(3D, int4)

GET_TEXTURE(1D, uint)
GET_TEXTURE(1D, uint2)
GET_TEXTURE(1D, uint4)
GET_TEXTURE(2D, uint)
GET_TEXTURE(2D, uint2)
GET_TEXTURE(2D, uint4)
GET_TEXTURE(3D, uint)
GET_TEXTURE(3D, uint2)
GET_TEXTURE(3D, uint4)

///////////////////////////////////////////////////////////////////////////////////
// RWTextures

GET_RW_TEXTURE(1D, float)
GET_RW_TEXTURE(1D, float2)
GET_RW_TEXTURE(1D, float4)
GET_RW_TEXTURE(2D, float)
GET_RW_TEXTURE(2D, float2)
GET_RW_TEXTURE(2D, float4)
GET_RW_TEXTURE(3D, float)
GET_RW_TEXTURE(3D, float2)
GET_RW_TEXTURE(3D, float4)

GET_RW_TEXTURE(1D, int)
GET_RW_TEXTURE(1D, int2)
GET_RW_TEXTURE(1D, int4)
GET_RW_TEXTURE(2D, int)
GET_RW_TEXTURE(2D, int2)
GET_RW_TEXTURE(2D, int4)
GET_RW_TEXTURE(3D, int)
GET_RW_TEXTURE(3D, int2)
GET_RW_TEXTURE(3D, int4)

GET_RW_TEXTURE(1D, uint)
GET_RW_TEXTURE(1D, uint2)
GET_RW_TEXTURE(1D, uint4)
GET_RW_TEXTURE(2D, uint)
GET_RW_TEXTURE(2D, uint2)
GET_RW_TEXTURE(2D, uint4)
GET_RW_TEXTURE(3D, uint)
GET_RW_TEXTURE(3D, uint2)
GET_RW_TEXTURE(3D, uint4)

///////////////////////////////////////////////////////////////////////////////////
// Samplers

SamplerState get_sampler(const uint sampler_index)
{
    return g_samplers[NonUniformResourceIndex(sampler_index)];
}

#undef GET_TEXTURE
#undef GET_RW_TEXTURE
#undef GET_TEXTURE_WITHOUT_TYPE

} // namespace tundra

#endif // TNDR_SHADERS_TEMPLATES_H
