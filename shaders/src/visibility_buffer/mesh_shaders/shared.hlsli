

/// https://docs.vulkan.org/spec/latest/chapters/limits.html#limits-minmax
/// maxMeshWorkGroupTotalCount: minimum 2^22
#define TASK_NUM_WORKGROUP_LIMIT (1 << 22)

///
#define TASK_WORKGROUP_SIZE (64)

///
struct DrawMeshTasksIndirectCommand {
    uint x;
    uint y;
    uint z;

    static DrawMeshTasksIndirectCommand create(const uint x, const uint y, const uint z)
    {
        DrawMeshTasksIndirectCommand cmd;
        cmd.x = x;
        cmd.y = y;
        cmd.z = z;
        return cmd;
    }
};

///
struct MeshTaskCommand {
    uint instance_id;
    uint meshlet_offset;
    /// Number of threads we want to execute inside of a task shader.
    /// This is important, because there may be situation
    /// where the group size is bigger than number of meshlets we want to process.
    uint num_meshlets;

    static MeshTaskCommand create(
        const uint instance_id, //
        const uint meshlet_offset,
        const uint num_meshlets)
    {
        MeshTaskCommand cmd;
        cmd.instance_id = instance_id;
        cmd.meshlet_offset = meshlet_offset;
        cmd.num_meshlets = num_meshlets;
        return cmd;
    }
};

/// Nvidia recommends to stay bellow 108 bytes,
/// but if that is not possible, then keep it at least under 236 bytes.
/// Right now we are over the limit - 260 bytes.
struct Payload {
    uint instance_id;
    uint meshlet_indices[64];
};

/// Maybe we could export barycentric coordinates? Is it worth it?
/// Is it better than recomputing barycentric coordinates in material shaders?
/// `VK_KHR_fragment_shader_barycentric`
/// Can we use half2 for that?
struct VertexOutput {
    float4 position : SV_Position;
    uint64_t instance_meshlet_id : TEXCOORD0;
};

