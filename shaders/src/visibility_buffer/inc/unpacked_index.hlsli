#ifndef TNDR_MESHLET_RENDERER_INC_UNPACKED_INDEX_H
#define TNDR_MESHLET_RENDERER_INC_UNPACKED_INDEX_H

#define MESHLET_ID_BITS 25
#define VERTEX_ID_BITS 7

#define VERTEX_ID_MASK 0x7F

///
struct UnpackedIndex {
    uint meshlet_id;
    uint vertex_id;

    static UnpackedIndex create(const uint index)
    {
        UnpackedIndex u;
        u.meshlet_id = index >> VERTEX_ID_BITS;
        u.vertex_id = index & VERTEX_ID_MASK;
        return u;
    }
};

#endif // TNDR_MESHLET_RENDERER_INC_MESHLET_H
