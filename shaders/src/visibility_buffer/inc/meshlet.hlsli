#ifndef TNDR_MESHLET_RENDERER_INC_MESHLET_H
#define TNDR_MESHLET_RENDERER_INC_MESHLET_H

///
struct Meshlet {
    float3 center;
    float radius;

    /// const float cone_axis0 = (int)((cone_axis_and_cutoff & 0xFF000000) >> 24) / 127.0,
    /// const float cone_axis1 = (int)((cone_axis_and_cutoff & 0x00FF0000) >> 16) / 127.0,
    /// const float cone_axis2 = (int)((cone_axis_and_cutoff & 0x0000FF00) >> 8) / 127.0);
    /// const float cone_cutoff = (int)(cone_axis_and_cutoff & 0x000000FF) / 127.0;
    uint cone_axis_and_cutoff;

    uint triangle_offset;
    uint triangle_count;
    uint vertex_offset;
    uint vertex_count;
};

#endif // TNDR_MESHLET_RENDERER_INC_MESHLET_H
