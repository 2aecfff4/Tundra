#ifndef TNDR_MESHLET_RENDERER_INC_VISIBLE_MESH_INSTANCE_H
#define TNDR_MESHLET_RENDERER_INC_VISIBLE_MESH_INSTANCE_H

///
struct VisibleMeshInstance {
    uint mesh_descriptor_index;
    uint instance_transform_index;
    uint lod_level;

    static VisibleMeshInstance create(
        const uint mesh_descriptor_index,
        const uint instance_transform_index,
        const uint lod_level)
    {
        VisibleMeshInstance visible_mesh_instance;
        visible_mesh_instance.mesh_descriptor_index = mesh_descriptor_index;
        visible_mesh_instance.instance_transform_index = instance_transform_index;
        visible_mesh_instance.lod_level = lod_level;
        return visible_mesh_instance;
    }
};

#endif // TNDR_MESHLET_RENDERER_INC_VISIBLE_MESH_INSTANCE_H
