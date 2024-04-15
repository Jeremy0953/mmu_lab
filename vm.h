#pragma once

typedef struct {
    int start_vpage;
    int end_vpage;
    bool write_protected;
    bool file_mapped;
}VMA;

typedef struct {
    unsigned int VALID : 1;
    unsigned int REFERENCED : 1;
    unsigned int MODIFIED : 1;
    unsigned int WRITE_PROTECT : 1;
    unsigned int PAGEDOUT : 1;
    unsigned int frame_number : 7;
    // 20 free bits
    unsigned int FILEMAPPED : 1;
    unsigned int CONFIGURATED : 1;
}pte_t;

