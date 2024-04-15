#pragma once

typedef struct{
    int pid;
    int frame_id;
    int vpage;
    bool dirty;
    unsigned int counter : 32;
    unsigned long last_used_time;
}frame_t;