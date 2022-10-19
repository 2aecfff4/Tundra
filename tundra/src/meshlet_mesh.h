#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/containers/string.h"
#include "math/vector3.h"
#include "math/vector4.h"

namespace tundra {

struct MeshletMesh {
    ///
    struct Meshlet {
        math::Vec3 center;
        f32 radius;

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

    math::Vec3 center;
    f32 radius;

    core::Array<Meshlet> meshlets;
    /// primitive to vertex_index_buffer
    core::Array<u8> meshlet_triangles;
    /// vertex_index_buffer to index
    core::Array<u32> meshlet_vertices;
    /// index to vertex
    core::Array<math::Vec3> vertices;
    core::Array<math::Vec3> normals;
    core::Array<math::Vec4> tangents;
    core::Array<core::Array<math::Vec2>> uvs;

public:
    ///
    [[nodiscard]] static MeshletMesh load(const core::String& path) noexcept;
    ///
    static void import(
        const core::String& mesh_path, const core::String& output_path) noexcept;
};

} // namespace tundra