#include "meshlet_mesh.h"
#include "core/std/defer.h"
#include "core/std/span.h"
#include "core/std/tuple.h"
#include <cgltf/cgltf.h>
#include <meshoptimizer.h>
#include <algorithm>
#include <fstream>
#include <ios>
#include <iterator>

namespace meshoptimizer {

using namespace tundra;

namespace optimize {

void optimize_vertex_cache_in_place(
    core::Span<u32> indices, const usize vertex_count) noexcept
{
    meshopt_optimizeVertexCache(
        indices.data(), indices.data(), indices.size(), vertex_count);
}

template <typename T>
void optimize_overdraw_in_place(
    core::Span<u32> indices,
    const core::Span<const T> vertices,
    const f32 threshold) noexcept
{
    meshopt_optimizeOverdraw(
        indices.data(),
        indices.data(),
        indices.size(),
        reinterpret_cast<const f32*>(vertices.data()),
        vertices.size(),
        sizeof(T),
        threshold);
}

template <typename T>
void optimize_vertex_fetch_in_place(
    core::Span<u32> indices, core::Span<T> vertices) noexcept
{
    meshopt_optimizeVertexFetch(
        vertices.data(),
        indices.data(),
        indices.size(),
        reinterpret_cast<const f32*>(vertices.data()),
        vertices.size(),
        sizeof(T));
}

} // namespace optimize

namespace clusterize {

struct BuildMeshletsResult {
    core::Array<meshopt_Meshlet> meshlets;
    core::Array<u32> meshlet_vertices;
    core::Array<u8> meshlet_triangles;
};

template <typename T>
[[nodiscard]] BuildMeshletsResult build_meshlets(
    const core::Span<const u32> indices,
    const core::Span<const T> vertices,
    const u64 max_vertices,
    const u64 max_triangles,
    const f32 cone_weight)
{
    const usize max_meshlet_count = meshopt_buildMeshletsBound(
        indices.size(), max_vertices, max_triangles);

    core::Array<meshopt_Meshlet> meshlets(max_meshlet_count);
    core::Array<u8> meshlet_triangles(max_meshlet_count * max_triangles * 3);
    core::Array<u32> meshlet_vertices(max_meshlet_count * max_vertices);

    const usize meshlet_count = meshopt_buildMeshlets(
        meshlets.data(),
        meshlet_vertices.data(),
        meshlet_triangles.data(),
        indices.data(),
        indices.size(),
        reinterpret_cast<const f32*>(vertices.data()),
        vertices.size(),
        sizeof(T),
        max_vertices,
        max_triangles,
        cone_weight);

    meshlets.resize(meshlet_count);
    const meshopt_Meshlet& last = meshlets.back();

    const usize meshlet_triangle_count = last.triangle_offset +
                                         ((static_cast<usize>(last.triangle_count) * 3u));
    // const usize meshlet_triangle_count =
    //     last.triangle_offset +
    //     (((static_cast<usize>(last.triangle_count) * 3u) + 3u) & ~3u);
    meshlet_triangles.resize(meshlet_triangle_count);
    meshlet_vertices.resize(last.vertex_offset + static_cast<usize>(last.vertex_count));

    return BuildMeshletsResult {
        .meshlets = meshlets,
        .meshlet_vertices = meshlet_vertices,
        .meshlet_triangles = meshlet_triangles,
    };
}

template <typename T>
[[nodiscard]] meshopt_Bounds compute_meshlet_bounds(
    const meshopt_Meshlet& meshlet,
    const core::Span<const u32> meshlet_vertices,
    const core::Span<const u8> meshlet_triangles,
    const core::Span<const T> vertices)
{
    return meshopt_computeMeshletBounds(
        meshlet_vertices.data() + meshlet.vertex_offset,
        meshlet_triangles.data() + meshlet.triangle_offset,
        meshlet.triangle_count,
        reinterpret_cast<const f32*>(vertices.data()),
        vertices.size(),
        sizeof(T));
}

} // namespace clusterize

} // namespace meshoptimizer

namespace tundra {

static constexpr u64 MESHLET_MESH_MAGIC_NUMBER = []() consteval {
    constexpr char LETTERS[] = { 'm', 's', 'h', 'l', 't', '_', 'm', 'h' };
    u64 v = 0;
    for (u64 i = 0; i < (sizeof(LETTERS) / sizeof(*LETTERS)); ++i) {
        v |= static_cast<u64>(LETTERS[i]) << (i * 8u);
    }

    return v;
}();

MeshletMesh MeshletMesh::load(const core::String& path) noexcept
{
    const auto load_array = []<typename T>(std::ifstream& input) -> core::Array<T> {
        u64 arr_size = 0;
        input.read(reinterpret_cast<char*>(&arr_size), sizeof(arr_size));
        core::Array<T> temp;
        temp.resize(arr_size);

        input.read(
            reinterpret_cast<char*>(temp.data()),
            static_cast<std::streamsize>(arr_size) * sizeof(T));
        return temp;
    };

    std::ifstream input(path, std::ios::binary | std::ios::in);
    tndr_assert(input.is_open(), "File does not exist.");

    u64 magic = 0;
    input.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    tndr_assert(magic == MESHLET_MESH_MAGIC_NUMBER, "");

    math::Vec3 mesh_center;
    f32 mesh_radius;
    core::Array<core::Array<math::Vec2>> uvs;

    input.read(reinterpret_cast<char*>(&mesh_center), sizeof(mesh_center));
    input.read(reinterpret_cast<char*>(&mesh_radius), sizeof(mesh_radius));

    core::Array<Meshlet> meshlets = load_array.operator()<MeshletMesh::Meshlet>(input);
    core::Array<u8> meshlet_triangles = load_array.operator()<u8>(input);
    core::Array<u32> meshlet_vertices = load_array.operator()<u32>(input);
    core::Array<math::Vec3> vertices = load_array.operator()<math::Vec3>(input);
    core::Array<math::Vec3> normals = load_array.operator()<math::Vec3>(input);
    core::Array<math::Vec4> tangents = load_array.operator()<math::Vec4>(input);

    u64 arr_size = 0;
    input.read(reinterpret_cast<char*>(&arr_size), sizeof(arr_size));
    for ([[maybe_unused]] u64 i = 0; i < arr_size; ++i) {
        uvs.push_back(load_array.operator()<math::Vec2>(input));
    }

    return MeshletMesh {
        .center = mesh_center,
        .radius = mesh_radius,
        .meshlets = core::move(meshlets),
        .meshlet_triangles = core::move(meshlet_triangles),
        .meshlet_vertices = core::move(meshlet_vertices),
        .vertices = core::move(vertices),
        .normals = core::move(normals),
        .tangents = core::move(tangents),
        .uvs = core::move(uvs),
    };
}

void MeshletMesh::import(
    const core::String& mesh_path, const core::String& output_path) noexcept
{
    const cgltf_options options {};
    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse_file(&options, mesh_path.c_str(), &data);
    tndr_assert(result == cgltf_result_success, "");

    tndr_defer {
        cgltf_free(data);
    };

    result = cgltf_load_buffers(&options, data, mesh_path.c_str());
    tndr_assert(result == cgltf_result_success, "");
    tndr_assert(data->meshes_count == 1, "");

    const cgltf_mesh& mesh = data->meshes[0];

    core::Array<u32> indices;
    core::Array<math::Vec3> vertices;
    core::Array<math::Vec3> normals;
    core::Array<math::Vec4> tangents;
    core::Array<core::Array<math::Vec2>> uvs;
    // Each primitive begins its indices from 0
    usize indices_offset = 0;

    // Primitive defines the geometry to be rendered with a material.
    for (usize primitive_index = 0; primitive_index < mesh.primitives_count;
         ++primitive_index) {
        const cgltf_primitive& primitive = mesh.primitives[primitive_index];

        if (primitive.indices != nullptr) {
            for (usize i = 0; i < primitive.indices->count; ++i) {
                indices.push_back(static_cast<u32>(
                    indices_offset + cgltf_accessor_read_index(primitive.indices, i)));
            }
        }

        const core::Span<cgltf_attribute> attributes = core::Span {
            primitive.attributes,
            primitive.attributes + primitive.attributes_count,
        };

        for (const cgltf_attribute& attribute : attributes) {
            switch (attribute.type) {
                case cgltf_attribute_type_position: {
                    const cgltf_accessor* accessor = attribute.data;
                    const cgltf_size num_components = cgltf_num_components(
                        accessor->type);
                    tndr_assert(num_components == 3, "");

                    for (usize i = 0; i < accessor->count; ++i) {
                        math::Vec3 temp;
                        cgltf_accessor_read_float(
                            accessor, i, reinterpret_cast<cgltf_float*>(&temp), 3);
                        vertices.push_back(temp);
                    }

                    indices_offset += accessor->count;

                    break;
                }
                case cgltf_attribute_type_normal: {
                    const cgltf_accessor* accessor = attribute.data;
                    const cgltf_size num_components = cgltf_num_components(
                        accessor->type);
                    tndr_assert(num_components == 3, "");

                    for (usize i = 0; i < accessor->count; ++i) {
                        math::Vec3 temp;
                        cgltf_accessor_read_float(
                            accessor, i, reinterpret_cast<cgltf_float*>(&temp), 3);
                        normals.push_back(temp);
                    }

                    break;
                }
                case cgltf_attribute_type_tangent: {
                    const cgltf_accessor* accessor = attribute.data;
                    const cgltf_size num_components = cgltf_num_components(
                        accessor->type);
                    tndr_assert(num_components == 4, "");

                    for (usize i = 0; i < accessor->count; ++i) {
                        math::Vec4 temp;
                        cgltf_accessor_read_float(
                            accessor, i, reinterpret_cast<cgltf_float*>(&temp), 4);
                        tangents.push_back(temp);
                    }

                    break;
                }
                case cgltf_attribute_type_texcoord: {
                    const cgltf_accessor* accessor = attribute.data;
                    const cgltf_size num_components = cgltf_num_components(
                        accessor->type);
                    tndr_assert(num_components == 2, "");

                    core::Array<math::Vec2> uv;
                    uv.reserve(accessor->count);

                    for (usize i = 0; i < accessor->count; ++i) {
                        math::Vec2 temp;
                        cgltf_accessor_read_float(
                            accessor, i, reinterpret_cast<cgltf_float*>(&temp), 2);
                        uv.push_back(temp);
                    }

                    uvs.push_back(core::move(uv));

                    break;
                }
                default:
                    break;
            }
        }
    }

    meshoptimizer::optimize::optimize_vertex_cache_in_place(
        core::as_span(indices), vertices.size());
    meshoptimizer::optimize::optimize_overdraw_in_place(
        core::as_span(indices), core::as_span(std::as_const(vertices)), 1.05f);
    meshoptimizer::optimize::optimize_vertex_fetch_in_place(
        core::as_span(indices), core::as_span(vertices));

    constexpr usize max_vertices = 64;
    constexpr usize max_triangles = 128;
    constexpr f32 cone_weight = 0.5f;

    const meshoptimizer::clusterize::BuildMeshletsResult build_meshlets_result =
        meshoptimizer::clusterize::build_meshlets(
            core::as_span(std::as_const(indices)),
            core::as_span(std::as_const(vertices)),
            max_vertices,
            max_triangles,
            cone_weight);

    core::Array<Meshlet> final_meshlets;
    final_meshlets.reserve(build_meshlets_result.meshlets.size());

    const core::Array<u8>& meshlet_triangles = build_meshlets_result.meshlet_triangles;
    const core::Array<u32>& meshlet_vertices = build_meshlets_result.meshlet_vertices;

    for (const meshopt_Meshlet& meshlet : build_meshlets_result.meshlets) {
        const meshopt_Bounds bounds = meshoptimizer::clusterize::compute_meshlet_bounds(
            meshlet,
            core::as_span(core::as_const(build_meshlets_result.meshlet_vertices)),
            core::as_span(core::as_const(build_meshlets_result.meshlet_triangles)),
            core::as_span(core::as_const(vertices)));

        final_meshlets.push_back(Meshlet {
            .center = math::Vec3 { bounds.center },
            .radius = bounds.radius,
            .cone_axis_and_cutoff = (u32(bounds.cone_axis_s8[0]) << 24u) |
                                    (u32(bounds.cone_axis_s8[1]) << 16u) |
                                    (u32(bounds.cone_axis_s8[2]) << 8u) |
                                    (u32(bounds.cone_cutoff_s8) << 0u),
            .triangle_offset = meshlet.triangle_offset,
            .triangle_count = meshlet.triangle_count,
            .vertex_offset = meshlet.vertex_offset,
            .vertex_count = meshlet.vertex_count,
        });
    }

    const auto [mesh_center, mesh_radius] = [&] {
        math::Vec3 center = math::Vec3 {};
        f32 radius = 0;

        for (const math::Vec3& vertex : vertices) {
            center += vertex;
            radius = math::max(radius, math::distance(math::Vec3 {}, vertex));
        }

        center /= static_cast<f32>(vertices.size());

        return core::make_tuple(center, radius);
    }();

    //////////////////////////////////////////////////////////////////////////////////////
    // Write

    const auto write_array = []<typename T>(
                                 std::ofstream& output, const core::Array<T>& arr) {
        const u64 arr_size = arr.size();
        output.write(reinterpret_cast<const char*>(&arr_size), sizeof(arr_size));
        output.write(
            reinterpret_cast<const char*>(arr.data()),
            static_cast<std::streamsize>(arr.size() * sizeof(T)));
    };

    std::ofstream output(output_path, std::ios::binary | std::ios::out | std::ios::trunc);

    output.write(
        reinterpret_cast<const char*>(&MESHLET_MESH_MAGIC_NUMBER),
        sizeof(MESHLET_MESH_MAGIC_NUMBER));

    output.write(reinterpret_cast<const char*>(&mesh_center), sizeof(mesh_center));
    output.write(reinterpret_cast<const char*>(&mesh_radius), sizeof(mesh_radius));

    write_array(output, final_meshlets);
    write_array(output, meshlet_triangles);
    write_array(output, meshlet_vertices);
    write_array(output, vertices);
    write_array(output, normals);
    write_array(output, tangents);

    const u64 uvs_size = uvs.size();
    output.write(reinterpret_cast<const char*>(&uvs_size), sizeof(uvs_size));
    for (const core::Array<math::Vec2>& uv : uvs) {
        write_array(output, uv);
    }
}

} // namespace tundra
