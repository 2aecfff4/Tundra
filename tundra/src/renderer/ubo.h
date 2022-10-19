#pragma once
#include "core/core.h"
#include "math/matrix4.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/vector4.h"
#include "renderer/config.h"
#include <array>

namespace tundra::renderer::ubo {

///
struct ClearIndirectBufferUBO {
    u32 indirect_buffer_uav;
};

///
struct InstanceCullingUBO {
    std::array<math::Vec4, 6> frustum_planes;
    u32 num_instances;

    struct {
        u32 mesh_descriptors_srv;
        u32 mesh_instances_srv;
        u32 mesh_instance_transforms_srv;
    } in_;

    struct {
        u32 visible_mesh_instances_uav;
        u32 meshlet_culling_dispatch_args_uav;
    } out_;
};

///
struct MeshletCullingUBO {
    math::Mat4 world_to_view;
    std::array<math::Vec4, 6> frustum_planes;
    math::Vec3 camera_position;
    u32 max_num_meshlets;

    struct {
        u32 visible_mesh_instances_srv;
        u32 mesh_descriptors_srv;
        u32 mesh_instance_transforms_srv;
    } in_;

    struct {
        u32 visible_meshlets_uav;
        u32 visible_meshlets_count_uav;
    } out_;
};

///
struct ClearIndexBufferGeneratorDispatchArgs {
    struct {
        u32 meshlet_offset_uav;
    } out_;
};

///
struct GenerateDispatchArgsUBO {
    u32 num_index_buffers_in_fight;
    u32 num_meshlets_per_index_buffer;

    struct {
        u32 num_visible_meshlets_srv;
    } in_;

    struct {
        u32 index_buffer_generator_dispatch_args_uav;
        u32 index_buffer_generator_meshlet_offsets_uav;
        u32 draw_meshlets_draw_counts_uav;
        u32 draw_meshlets_draw_args_uav;
    } out_;

    struct {
        u32 meshlet_offset_uav;
    } inout_;
};

///
struct IndexBufferGeneratorUBO {
    math::Mat4 world_to_clip;
    u32 max_num_indices;
    u32 generator_index;

    struct {
        u32 mesh_instance_transforms_srv;
        u32 visible_meshlets_srv;
        u32 mesh_descriptors_srv;
        u32 meshlet_offsets_srv;
    } in_;

    struct {
        u32 index_buffer_uav;
        u32 draw_meshlets_draw_args_uav;
    } out_;
};

///
struct VisibilityBufferPassUBO {
    /// View
    math::Mat4 world_to_view;
    /// Projection
    math::Mat4 view_to_clip;

    struct {
        u32 visible_meshlets_srv;
        u32 mesh_descriptors_srv;
        u32 mesh_instance_transforms_srv;
    } in_;
};

inline constexpr u64 CLEAR_INDIRECT_BUFFER_UBO_OFFSET = 0;
inline constexpr u64 INSTANCE_CULLING_UBO_OFFSET = CLEAR_INDIRECT_BUFFER_UBO_OFFSET +
                                                   sizeof(ClearIndirectBufferUBO);

inline constexpr u64 MESHLET_CULLING_UBO_OFFSET = INSTANCE_CULLING_UBO_OFFSET +
                                                  sizeof(InstanceCullingUBO);

inline constexpr u64 CLEAR_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_OFFSET =
    MESHLET_CULLING_UBO_OFFSET + sizeof(MeshletCullingUBO);

inline constexpr u64 GENERATE_DISPATCH_ARGS_UBO_OFFSET =
    CLEAR_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_OFFSET +
    sizeof(ClearIndexBufferGeneratorDispatchArgs);

inline constexpr u64 INDEX_BUFFER_GENERATOR_UBO_OFFSET =
    GENERATE_DISPATCH_ARGS_UBO_OFFSET + sizeof(GenerateDispatchArgsUBO);

inline constexpr u64 VISIBILITY_BUFFER_PASS_UBO_OFFSET =
    INDEX_BUFFER_GENERATOR_UBO_OFFSET +
    (sizeof(GenerateDispatchArgsUBO) * config::NUM_INDEX_BUFFERS_IN_FIGHT);

inline constexpr u64 TOTAL_UBO_SIZE = CLEAR_INDIRECT_BUFFER_UBO_OFFSET +
                                      INSTANCE_CULLING_UBO_OFFSET +
                                      MESHLET_CULLING_UBO_OFFSET +
                                      GENERATE_DISPATCH_ARGS_UBO_OFFSET +
                                      INDEX_BUFFER_GENERATOR_UBO_OFFSET +
                                      VISIBILITY_BUFFER_PASS_UBO_OFFSET +
                                      (sizeof(VisibilityBufferPassUBO) *
                                       config::NUM_INDEX_BUFFERS_IN_FIGHT);

} // namespace tundra::renderer::ubo
