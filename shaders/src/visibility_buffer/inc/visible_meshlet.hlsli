#ifndef TNDR_MESHLET_RENDERER_INC_VISIBLE_MESHLET_H
#define TNDR_MESHLET_RENDERER_INC_VISIBLE_MESHLET_H

///
struct VisibleMeshlet {
    uint mesh_descriptor_index;
    uint meshlet_index;
    uint instance_transform_index;

    static VisibleMeshlet create(
        const uint mesh_descriptor_index,
        const uint meshlet_index,
        const uint instance_transform_index)
    {
        VisibleMeshlet visible_meshlet;
        visible_meshlet.mesh_descriptor_index = mesh_descriptor_index;
        visible_meshlet.meshlet_index = meshlet_index;
        visible_meshlet.instance_transform_index = instance_transform_index;
        return visible_meshlet;
    }
};

#endif // TNDR_MESHLET_RENDERER_INC_VISIBLE_MESHLET_H
