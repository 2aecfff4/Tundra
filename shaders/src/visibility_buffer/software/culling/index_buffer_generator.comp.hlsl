#include "bindings.hlsli"
#include "defines.hlsli"
#include "math/quat.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/commands.hlsli"
#include "visibility_buffer/inc/frustum_culling.hlsli"
#include "visibility_buffer/inc/instance_transform.hlsli"
#include "visibility_buffer/inc/mesh_descriptor.hlsli"
#include "visibility_buffer/inc/mesh_instance.hlsli"
#include "visibility_buffer/inc/unpacked_index.hlsli"
#include "visibility_buffer/inc/visible_mesh_instance.hlsli"
#include "visibility_buffer/inc/visible_meshlet.hlsli"

///
struct IndexBufferGeneratorUBO {
    float4x4 world_to_clip;
    uint max_num_indices;
    uint generator_index;

    struct {
        uint mesh_instance_transforms_srv;
        uint visible_meshlets_srv;
        uint mesh_descriptors_srv;
        uint meshlet_offsets_srv;
    } in_;

    struct {
        uint index_buffer_uav;
        uint draw_meshlets_draw_args_uav;
    } out_;
};

///
struct Input {
    uint meshlet_index : SV_GroupID;
    uint triangle_index : SV_GroupThreadID;
    uint group_thread_id : SV_GroupThreadID;
    uint thread_id : SV_DispatchThreadID;
};

///
// groupshared uint global_index_offset = 0;

///
groupshared uint3 g_indices[128];

///
[numthreads(128, 1, 1)] void main(const Input input) {
    const IndexBufferGeneratorUBO ubo = tundra::load_ubo<IndexBufferGeneratorUBO>();

    if (input.thread_id == 0) {
        tundra::buffer_store<false>(
            ubo.out_.draw_meshlets_draw_args_uav,
            sizeof(DrawIndexedIndirectCommand) * ubo.generator_index,
            0,
            DrawIndexedIndirectCommand::create(0, 1, 0, 0, 0));
    }

    DeviceMemoryBarrierWithGroupSync();

    const uint meshlet_offset = tundra::buffer_load<false, uint>(
        ubo.in_.meshlet_offsets_srv, 0, ubo.generator_index);
    const uint visible_meshlet_index = meshlet_offset + input.meshlet_index;

    const VisibleMeshlet visible_meshlet = tundra::buffer_load<false, VisibleMeshlet>(
        ubo.in_.visible_meshlets_srv, 0, visible_meshlet_index);
    const InstanceTransform instance_transform =
        tundra::buffer_load<false, InstanceTransform>(
            ubo.in_.mesh_instance_transforms_srv,
            0,
            visible_meshlet.instance_transform_index);
    const MeshDescriptor mesh_descriptor = tundra::buffer_load<false, MeshDescriptor>(
        ubo.in_.mesh_descriptors_srv, 0, visible_meshlet.mesh_descriptor_index);
    const Meshlet meshlet = mesh_descriptor.get_meshlet(visible_meshlet.meshlet_index);

    if (input.group_thread_id == 0) {
        for (uint triangle_index = 0; triangle_index < meshlet.triangle_count;
             ++triangle_index) {
            g_indices[triangle_index] = tundra::buffer_load<true, uint3>(
                mesh_descriptor.mesh_data_buffer_srv,
                mesh_descriptor.meshlet_triangles_offset +
                    (meshlet.triangle_offset * sizeof(uint)),
                triangle_index);
        }
    }

    DeviceMemoryBarrierWithGroupSync();

    const uint triangle_index = input.triangle_index;

    bool is_visible = false;
    if (triangle_index < meshlet.triangle_count) {
        const float3 vertices[3] = {
            mesh_descriptor.get_vertex(meshlet, g_indices[triangle_index][0]),
            mesh_descriptor.get_vertex(meshlet, g_indices[triangle_index][1]),
            mesh_descriptor.get_vertex(meshlet, g_indices[triangle_index][2]),
        };

        float4 transformed_vertices[3];
        UNROLL
        for (uint i = 0; i < 3; ++i) {
            const float3 vec = (quat_rotate_vector(instance_transform.quat, vertices[i]) *
                                instance_transform.scale) +
                               instance_transform.position;
            transformed_vertices[i] = mul(ubo.world_to_clip, float4(vec, 1.f));
        }

        // "Triangle Scan Conversion using 2D Homogeneous Coordinates" - Marc Olano, Trey Greer
        const float det = determinant(float3x3(
            transformed_vertices[0].xyw,
            transformed_vertices[1].xyw,
            transformed_vertices[2].xyw));

        is_visible = det > 0.f;
    }

    const uint triangle_offset = WavePrefixCountBits(is_visible);
    const uint visible_triangles_count = WaveActiveCountBits(is_visible);

    uint index_offset = 0;
    if (WaveIsFirstLane()) {
        tundra::buffer_interlocked_add<false>(
            ubo.out_.draw_meshlets_draw_args_uav,
            sizeof(DrawIndexedIndirectCommand) * ubo.generator_index,
            visible_triangles_count * 3,
            index_offset);
    }

    index_offset = WaveReadLaneFirst(index_offset);

    if (is_visible && (triangle_offset < meshlet.triangle_count) &&
        ((index_offset + triangle_offset * 3) < ubo.max_num_indices)) {
        const uint shifted_visible_meshlet_index = visible_meshlet_index
                                                   << VERTEX_ID_BITS;

        tundra::buffer_store<false>(
            ubo.out_.index_buffer_uav,
            index_offset * sizeof(uint),
            triangle_offset,
            shifted_visible_meshlet_index | g_indices[triangle_index]);
    }
}
