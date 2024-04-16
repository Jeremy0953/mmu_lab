#pragma once

#include <iostream>
#include <fstream>
#include <deque>
#include <memory>
#include <limits.h>
#include "pm.h"
#include "proc.h"
#include "vm.h"
class ProcessManager;
#define TAU 49

enum pager_type{
    FIFO_TYPE,
    RANDOM_TYPE,
    CLOCK_TYPE,
    NRU_TYPE,
    AGING_TYPE,
    WS_TYPE
};

class MyRandom {
public:
    int rand_num = 0;
    std::vector<int> randvals;
    MyRandom(const std::string& rand_file){
        try {
            std::ifstream randfile(rand_file);
            
            if (!randfile) {
                throw std::runtime_error("Failed to open random file.");
            }

            randfile >> rand_num;
            randvals.reserve(rand_num);
            int rand;
            while(randfile >> rand){
                randvals.emplace_back(rand);
            }
            randfile.close();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            // Handle the error accordingly.
        }
    };
};

class Pager {
public:
    Pager(std::shared_ptr<ProcessManager> proc_manager_): proc_manager(proc_manager_){};
    virtual ~Pager() {} 
    virtual frame_t* select_victim_frame() = 0;
    virtual void reset_counter(frame_t* victim_frame){};
    void ins_count_increment(){
        instruction_count += 1;
    }
    void initialize_frame_table(int frame_size){
        this->frame_size = frame_size;
        for(int i = 0; i < frame_size; i++){
            frame_t frame;
            frame.pid = -1;
            frame.dirty = false;
            frame.vpage = -1;
            frame.frame_id = i;
            frame.counter = 0;
            frame_table.push_back(frame);
        }
    }
    void initialize_free_pool(){
        for(int i = 0; i < frame_table.size(); i++){
            frame_t* frame = &frame_table.at(i);
            free_pool.push_back(frame);
        }
    }

    bool has_free_frame(){
        return !free_pool.empty();
    }

    frame_t* get_free_frame(){
        frame_t* frame = free_pool.front();
        free_pool.pop_front();
        return frame;
    }
    void add_to_victim_table(frame_t* frame){
        victim_table.push_back(frame);
    }

    void unmap(unsigned int frame_number){
        frame_t* frame = &frame_table[frame_number];
        frame->pid = -1;
        frame->vpage = -1;
        frame->counter = 0;
        frame->last_used_time = 0;
        frame->dirty = false;
        free_pool.push_back(frame);
    }

    void print_frame_table() {
        std::cout << "FT:";
        for(int i = 0; i < frame_size; i++){
            frame_t* frame = &frame_table[i];
            if(frame->vpage != -1){
                printf(" %d:%d", frame->pid, frame->vpage);
            }else{
                std::cout << " *";
            }
        }
        std::cout << std::endl;
    }
    void set_frame_size(int frame_size){
        this->frame_size = frame_size;
    }
protected:
    std::deque<frame_t> frame_table;
    std::deque<frame_t*> victim_table;
    std::deque<frame_t*> free_pool;
    std::shared_ptr<ProcessManager> proc_manager;
    int frame_size;
    int victim_frame_index = 0;  
    int ofs = 0;
    unsigned long instruction_count = 0;
};

class FIFO : public Pager{
public:
    FIFO(std::shared_ptr<ProcessManager> proc_manager_): Pager(proc_manager_){};
    frame_t* select_victim_frame() override;
    void reset_counter(frame_t* victim_frame){}
};

class CLOCK : public Pager{
public:
    CLOCK(std::shared_ptr<ProcessManager> proc_manager_): Pager(proc_manager_){};
    frame_t* select_victim_frame() override;
    void reset_counter(frame_t* victim_frame){}
};
class NRU : public Pager{
public:
    NRU(std::shared_ptr<ProcessManager> proc_manager_): Pager(proc_manager_){};
    unsigned long last_referenced = 0;
    frame_t* select_victim_frame() override;
    void reset_counter(frame_t* victim_frame){}
};
class AGING: public Pager{
public:
    AGING(std::shared_ptr<ProcessManager> proc_manager_): Pager(proc_manager_){};
    void reset_counter(frame_t* victim_frame){
        victim_frame->counter = 0;
    }
    frame_t* select_victim_frame() override;
};
class WS: public Pager{
public:
    WS(std::shared_ptr<ProcessManager> proc_manager_): Pager(proc_manager_){};
    frame_t* select_victim_frame() override;
    void reset_counter(frame_t* victim_frame){}
};

class RANDOM: public Pager{
public:
    MyRandom my_random;
    frame_t* select_victim_frame() override;
    void reset_counter(frame_t* victim_frame){};
    RANDOM(const std::string& rand_file, std::shared_ptr<ProcessManager> proc_manager_) : my_random(rand_file), Pager(proc_manager_){
    };
};