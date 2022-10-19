#ifndef TNDR_MESHLET_RENDERER_INC_COMMANDS_H
#define TNDR_MESHLET_RENDERER_INC_COMMANDS_H

///
struct DispatchIndirectCommand {
    uint x;
    uint y;
    uint z;

    static DispatchIndirectCommand create(const uint x, const uint y, const uint z)
    {
        DispatchIndirectCommand cmd;
        cmd.x = x;
        cmd.y = y;
        cmd.z = z;
        return cmd;
    }
};

///
struct DrawIndexedIndirectCommand {
    uint index_count;
    uint instance_count;
    uint first_index;
    int vertex_offset;
    uint first_instance;

    static DrawIndexedIndirectCommand create(
        const uint index_count,
        const uint instance_count,
        const uint first_index,
        const int vertex_offset,
        const uint first_instance)
    {
        DrawIndexedIndirectCommand cmd;
        cmd.index_count = index_count;
        cmd.instance_count = instance_count;
        cmd.first_index = first_index;
        cmd.vertex_offset = vertex_offset;
        cmd.first_instance = first_instance;
        return cmd;
    }
};

#endif // TNDR_MESHLET_RENDERER_INC_COMMANDS_H
