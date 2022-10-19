#pragma once
#include "core/core.h"
#include "core/utils/enum_flags.h"
#include "math/quat.h"
#include "math/vector3.h"

namespace tundra::shader {

///
struct MeshInstance {
    u32 mesh_descriptor_index;
};

///
struct VisibleMeshInstance {
    u32 mesh_descriptor_index;
    u32 instance_transform_index;
    u32 lod_level;
};

///
struct InstanceTransform {
    math::Quat quat;
    math::Vec3 position;
    f32 scale;
};

/// Vertex buffer layout
/// bits:...6 5 4 3 2 1 0
///       | | | | | | | |- positions
///       | | | | | | |--- normals
///       | | | | | |----- tangents
///       | | | | |------- uv0
///       | | | |--------- uv1
///       | | |----------- uv2
///       | |------------- unused
///       |--------------- unused
enum class VertexBufferLayout : u32 {
    NONE = 0,
    POSITIONS = 1 << 0,
    NORMALS = 1 << 1,
    TANGENTS = 1 << 2,
    UV0 = 1 << 3,
    UV1 = 1 << 4,
    UV2 = 1 << 5,
};

TNDR_ENUM_CLASS_FLAGS(VertexBufferLayout)

///
struct MeshDescriptor {
    math::Vec3 center;
    float radius;

    /// Buffer layout:
    /// - Meshlets
    /// - Primitive index buffer
    /// - Vertex index buffer
    /// - Vertex buffer
    ///   - SOA [position][position][position][normal][normal][normal][uv][uv][uv]
    u32 mesh_data_buffer_srv;

    u32 meshlet_count;

    u32 meshlet_triangles_offset;
    u32 meshlet_triangles_count;

    u32 meshlet_vertices_offset;
    u32 meshlet_vertices_count;

    u32 vertex_buffer_offset;
    u32 vertex_count;

    VertexBufferLayout vertex_buffer_layout;
};

///
struct Meshlet {
    math::Vec3 center;
    float radius;

    /// const float cone_axis0 = (int)((cone_axis_and_cutoff & 0xFF000000) >> 24) / 127.0,
    /// const float cone_axis1 = (int)((cone_axis_and_cutoff & 0x00FF0000) >> 16) / 127.0,
    /// const float cone_axis2 = (int)((cone_axis_and_cutoff & 0x0000FF00) >> 8) / 127.0);
    /// const float cone_cutoff = (int)(cone_axis_and_cutoff & 0x000000FF) / 127.0;
    u32 cone_axis_and_cutoff;

    u32 triangle_offset;
    u32 triangle_count;
    u32 vertex_offset;
    u32 vertex_count;
};

///
struct VisibleMeshlet {
    u32 mesh_descriptor_index;
    u32 meshlet_index;
    u32 instance_transform_index;
};

} // namespace tundra::shader

namespace tundra::renderer {

///
struct MeshInstance {
    u32 mesh_descriptor_index;
};

///
struct MeshDescriptor {
    math::Vec3 center;
    float radius;

    u32 meshlet_count;
    u32 meshlet_triangles_count;
    u32 meshlet_vertices_count;
    u32 vertex_count;

    shader::VertexBufferLayout vertex_buffer_layout;
};

} // namespace tundra::renderer
