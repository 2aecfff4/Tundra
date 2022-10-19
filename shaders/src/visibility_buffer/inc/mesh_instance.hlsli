#ifndef TNDR_MESHLET_RENDERER_INC_MESH_INSTANCE_H
#define TNDR_MESHLET_RENDERER_INC_MESH_INSTANCE_H

///
struct MeshInstance {
    uint mesh_descriptor_index;

    static MeshInstance create(const uint mesh_descriptor_index)
    {
        MeshInstance mesh_instance;
        mesh_instance.mesh_descriptor_index = mesh_descriptor_index;
        return mesh_instance;
    }
};

#endif // TNDR_MESHLET_RENDERER_INC_MESH_INSTANCE_H
