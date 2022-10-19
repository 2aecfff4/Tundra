#ifndef TNDR_SHADERS_BINDINGS_H
#define TNDR_SHADERS_BINDINGS_H

#ifdef LM_VULKAN
#define TNDR_BINDING(s, b) [[vk::binding(b, s)]]
#define TNDR_PUSH_CONSTANT [[vk::push_constant]]
#else
#define TNDR_BINDING(set, binding)
#define TNDR_PUSH_CONSTANT
#endif

///////////////////////////////////////////////////////////////////////////////////
// Buffers

TNDR_BINDING(0, 0) ByteAddressBuffer g_buffers[] : register(t0, space0);
TNDR_BINDING(0, 0) RWByteAddressBuffer g_rw_buffers[] : register(u0, space1);

///////////////////////////////////////////////////////////////////////////////////
// Textures

TNDR_BINDING(1, 0) Texture1D<float> g_textures1D_float[] : register(t0, space2);
TNDR_BINDING(1, 0) Texture1D<float2> g_textures1D_float2[] : register(t0, space3);
TNDR_BINDING(1, 0) Texture1D<float4> g_textures1D_float4[] : register(t0, space4);

TNDR_BINDING(1, 0) Texture2D<float> g_textures2D_float[] : register(t0, space5);
TNDR_BINDING(1, 0) Texture2D<float2> g_textures2D_float2[] : register(t0, space6);
TNDR_BINDING(1, 0) Texture2D<float4> g_textures2D_float4[] : register(t0, space7);

TNDR_BINDING(1, 0) Texture3D<float> g_textures3D_float[] : register(t0, space8);
TNDR_BINDING(1, 0) Texture3D<float2> g_textures3D_float2[] : register(t0, space9);
TNDR_BINDING(1, 0) Texture3D<float4> g_textures3D_float4[] : register(t0, space10);

TNDR_BINDING(1, 0) Texture1D<uint> g_textures1D_uint[] : register(t0, space11);
TNDR_BINDING(1, 0) Texture1D<uint2> g_textures1D_uint2[] : register(t0, space12);
TNDR_BINDING(1, 0) Texture1D<uint4> g_textures1D_uint4[] : register(t0, space13);
TNDR_BINDING(1, 0) Texture2D<uint64_t> g_textures2D_uint64[] : register(t0, space14);

TNDR_BINDING(1, 0) Texture2D<uint> g_textures2D_uint[] : register(t0, space15);
TNDR_BINDING(1, 0) Texture2D<uint2> g_textures2D_uint2[] : register(t0, space16);
TNDR_BINDING(1, 0) Texture2D<uint4> g_textures2D_uint4[] : register(t0, space17);

TNDR_BINDING(1, 0) Texture3D<uint> g_textures3D_uint[] : register(t0, space18);
TNDR_BINDING(1, 0) Texture3D<uint2> g_textures3D_uint2[] : register(t0, space19);
TNDR_BINDING(1, 0) Texture3D<uint4> g_textures3D_uint4[] : register(t0, space20);

TNDR_BINDING(1, 0) Texture1D<int> g_textures1D_int[] : register(t0, space21);
TNDR_BINDING(1, 0) Texture1D<int2> g_textures1D_int2[] : register(t0, space22);
TNDR_BINDING(1, 0) Texture1D<int4> g_textures1D_int4[] : register(t0, space23);

TNDR_BINDING(1, 0) Texture2D<int> g_textures2D_int[] : register(t0, space24);
TNDR_BINDING(1, 0) Texture2D<int2> g_textures2D_int2[] : register(t0, space25);
TNDR_BINDING(1, 0) Texture2D<int4> g_textures2D_int4[] : register(t0, space26);

TNDR_BINDING(1, 0) Texture3D<int> g_textures3D_int[] : register(t0, space27);
TNDR_BINDING(1, 0) Texture3D<int2> g_textures3D_int2[] : register(t0, space28);
TNDR_BINDING(1, 0) Texture3D<int4> g_textures3D_int4[] : register(t0, space29);

///////////////////////////////////////////////////////////////////////////////////
// RWTextures

TNDR_BINDING(2, 0) RWTexture1D<float> g_rw_textures1D_float[] : register(u0, space30);
TNDR_BINDING(2, 0) RWTexture1D<float2> g_rw_textures1D_float2[] : register(u0, space31);
TNDR_BINDING(2, 0) RWTexture1D<float4> g_rw_textures1D_float4[] : register(u0, space32);

TNDR_BINDING(2, 0) RWTexture2D<float> g_rw_textures2D_float[] : register(u0, space33);
TNDR_BINDING(2, 0) RWTexture2D<float2> g_rw_textures2D_float2[] : register(u0, space34);
TNDR_BINDING(2, 0) RWTexture2D<float4> g_rw_textures2D_float4[] : register(u0, space35);

TNDR_BINDING(2, 0) RWTexture3D<float> g_rw_textures3D_float[] : register(u0, space36);
TNDR_BINDING(2, 0) RWTexture3D<float2> g_rw_textures3D_float2[] : register(u0, space37);
TNDR_BINDING(2, 0) RWTexture3D<float4> g_rw_textures3D_float4[] : register(u0, space38);

TNDR_BINDING(2, 0) RWTexture1D<uint> g_rw_textures1D_uint[] : register(u0, space39);
TNDR_BINDING(2, 0) RWTexture1D<uint2> g_rw_textures1D_uint2[] : register(u0, space41);
TNDR_BINDING(2, 0) RWTexture1D<uint4> g_rw_textures1D_uint4[] : register(u0, space42);

TNDR_BINDING(2, 0) RWTexture2D<uint> g_rw_textures2D_uint[] : register(u0, space43);
TNDR_BINDING(2, 0) RWTexture2D<uint2> g_rw_textures2D_uint2[] : register(u0, space44);
TNDR_BINDING(2, 0) RWTexture2D<uint4> g_rw_textures2D_uint4[] : register(u0, space45);
TNDR_BINDING(2, 0) RWTexture2D<uint64_t> g_rw_textures2D_uint64[] : register(u0, space46);

TNDR_BINDING(2, 0) RWTexture3D<uint> g_rw_textures3D_uint[] : register(u0, space47);
TNDR_BINDING(2, 0) RWTexture3D<uint2> g_rw_textures3D_uint2[] : register(u0, space48);
TNDR_BINDING(2, 0) RWTexture3D<uint4> g_rw_textures3D_uint4[] : register(u0, space49);

TNDR_BINDING(2, 0) RWTexture1D<int> g_rw_textures1D_int[] : register(u0, space50);
TNDR_BINDING(2, 0) RWTexture1D<int2> g_rw_textures1D_int2[] : register(u0, space51);
TNDR_BINDING(2, 0) RWTexture1D<int4> g_rw_textures1D_int4[] : register(u0, space52);

TNDR_BINDING(2, 0) RWTexture2D<int> g_rw_textures2D_int[] : register(u0, space53);
TNDR_BINDING(2, 0) RWTexture2D<int2> g_rw_textures2D_int2[] : register(u0, space54);
TNDR_BINDING(2, 0) RWTexture2D<int4> g_rw_textures2D_int4[] : register(u0, space55);

TNDR_BINDING(2, 0) RWTexture3D<int> g_rw_textures3D_int[] : register(u0, space56);
TNDR_BINDING(2, 0) RWTexture3D<int2> g_rw_textures3D_int2[] : register(u0, space57);
TNDR_BINDING(2, 0) RWTexture3D<int4> g_rw_textures3D_int4[] : register(u0, space58);

TNDR_BINDING(1, 0) Texture1D g_textures1D[] : register(t0, space59);
TNDR_BINDING(1, 0) Texture2D g_textures2D[] : register(t0, space60);
TNDR_BINDING(1, 0) Texture3D g_textures3D[] : register(t0, space61);

///////////////////////////////////////////////////////////////////////////////////
// Samplers

TNDR_BINDING(3, 0) SamplerState g_samplers[] : register(s0, space62);

///////////////////////////////////////////////////////////////////////////////////
// Push constant

struct PushConstant {
    uint buffer_index;
    uint buffer_offset;
};

TNDR_PUSH_CONSTANT ConstantBuffer<PushConstant> g_push_constant : register(b0, space63);

#endif // TNDR_SHADERS_BINDINGS_H
