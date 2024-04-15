#pragma once

#include <iostream>
#include <fstream>
#include <deque>
#include <memory>
#include <limits.h>
#include "pm.h"
#include "proc.h"

#define TAU 49

enum pager_type{
    FIFO,
    RANDOM,
    CLOCK,
    NRU,
    AGING,
    WS
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
    virtual frame_t* select_victim_frame() = 0;
    virtual void reset_counter(frame_t* victim_frame){};
    void ins_count_increment(){
        instruction_count += 1;
    }
    void initialize_frame_table(int frame_size){
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
protected:
    std::shared_ptr<std::vector<Process*>> proc_vector;
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
    frame_t* select_victim_frame() override{
        if(victim_frame_index == frame_size){
            victim_frame_index = 0;
        }
        frame_t* frame = victim_table.at(victim_frame_index);
        victim_frame_index += 1;
        return frame;
    }
    void reset_counter(frame_t* victim_frame){}
};

class CLOCK : public Pager{
public:
    frame_t* select_victim_frame(){

        frame_t* victim_frame;

        while(true){
            if(victim_frame_index == frame_size){
                victim_frame_index = 0;
            }
            victim_frame = victim_table.at(victim_frame_index);
            pte_t* pte = proc_manager->get_pte(victim_frame->pid, victim_frame->vpage);
            if(pte->REFERENCED){
                pte->REFERENCED = 0;
                victim_frame_index +=1;
                continue;
            }else{
                pte->REFERENCED = 1;
                victim_frame_index +=1;
                break;
            }
        }
        return victim_frame;
    }
    void reset_counter(frame_t* victim_frame){}
};
class NRU : public Pager{
public:
    unsigned long last_referenced = 0;
    frame_t* select_victim_frame(){

        if (victim_frame_index >= frame_size) {
            victim_frame_index = 0;
        }

        bool update_reference = (instruction_count - last_referenced >= 48) ? true : false;

        frame_t* victim_frame = victim_table[victim_frame_index];
        pte_t* victim_pte = proc_manager->get_pte(victim_frame->pid, victim_frame->vpage);
        int lowest_class = 2 * victim_pte->REFERENCED + victim_pte->MODIFIED;
        victim_frame_index += 1;

        if(lowest_class == 0 && !update_reference){
            victim_frame_index = ((victim_frame->frame_id == frame_size) ? 0 : victim_frame->frame_id + 1);
            return victim_frame;
        }

        for(int i = 0; i < frame_size; i++) {

            if (victim_frame_index >= frame_size) {
                victim_frame_index = 0;
            }
            frame_t *curr_frame = victim_table[victim_frame_index];
            pte_t *curr_pte = proc_manager->get_pte(curr_frame->pid, curr_frame->vpage);

            int curr_class = 2 * curr_pte->REFERENCED + curr_pte->MODIFIED;
            //int victim_pte_level = 2 * victim_pte->REFERENCED + victim_pte->MODIFIED;
            victim_frame_index += 1;

            if (curr_class == 0 && instruction_count < 48) {
                victim_frame = curr_frame;
                break;
            } else if (curr_class < lowest_class) {
                victim_pte = curr_pte;
                victim_frame = curr_frame;
                lowest_class = curr_class;
            }
        }

        //reset REFERENCE bit
        if(update_reference){
            //cout << "instruction count " + to_string(instruction_count)<< endl;
            for(int i = 0; i < frame_table.size(); i++){
                pte_t* pte = proc_manager->get_pte(frame_table[i].pid, frame_table[i].vpage);
                pte->REFERENCED = 0;
            }
            last_referenced = instruction_count;
            //instruction_count = 0;
        }

        //increment victim_index by 1
        victim_frame_index = ((victim_frame->frame_id == frame_size) ? 0 : victim_frame->frame_id + 1);
        return victim_frame;
    }
    void reset_counter(frame_t* victim_frame){}
};
class AGING: public Pager{
public:
    void reset_counter(frame_t* victim_frame){
        victim_frame->counter = 0;
    }
    frame_t* select_victim_frame(){

        /* 1) right shift counter by 1
         * 2) add current REFERENCE bit to most left
         * 3) reset REFERENCE bit
         * 4) min_counter = min(min_counter, curr_counter)
         */
        if(victim_frame_index >= frame_size){
            victim_frame_index = 0;
        }
        frame_t* victim_frame = &frame_table[victim_frame_index];
        pte_t* victim_pte = proc_manager->get_pte(victim_frame->pid, victim_frame->vpage);
        victim_frame->counter >>= 1;
        if(victim_pte->REFERENCED == 1){
            victim_frame->counter = (victim_frame->counter | 0x80000000);
            victim_pte->REFERENCED = 0;
        }
        victim_frame_index += 1;

        for(int i = 0; i < frame_size - 1; i++ ){

            if(victim_frame_index >= frame_size){
                victim_frame_index = 0;
            }
            frame_t* curr_frame = &frame_table[victim_frame_index];
            pte_t* curr_pte = proc_manager->get_pte(curr_frame->pid, curr_frame->vpage);
            curr_frame->counter >>= 1;
            if(curr_pte->REFERENCED == 1) {
                curr_frame->counter = (curr_frame->counter | 0x80000000);
                curr_pte->REFERENCED = 0;
            }

            //cout << "victim_frame: " + to_string(victim_frame->frame_id) + " " + to_string(victim_frame->counter) << endl;
            //cout << "curr_frame: " + to_string(curr_frame->frame_id) + " " + to_string(curr_frame->counter) << endl;
            if(victim_frame->counter > curr_frame->counter){
                //cout <<"vicim_frame_id " + to_string(curr_frame->frame_id) << endl;
                //cout <<"min: " + to_string(min_counter) << endl;
                victim_frame = curr_frame;
            }

            victim_frame_index += 1;
        }
        victim_frame_index = (victim_frame->frame_id == frame_size) ? 0 : victim_frame->frame_id + 1;
        return victim_frame;
    }
};
class WS: public Pager{
public:
    frame_t* select_victim_frame(){

        unsigned long long min_time = ULLONG_MAX;
        frame_t* victim_frame = victim_table[victim_frame_index];

        for(int i = 0; i < frame_size; i++){

            if(victim_frame_index >= frame_size){
                victim_frame_index = 0;
            }

            frame_t* curr_frame = victim_table[victim_frame_index];
            pte_t* curr_pte = proc_manager->get_pte(curr_frame->pid, curr_frame->vpage);
            unsigned long long frame_age = instruction_count - curr_frame->last_used_time;

            if(curr_pte->REFERENCED == 1){
                curr_pte->REFERENCED = 0;
                curr_frame->last_used_time = instruction_count;
            }else{
                if(frame_age > TAU) {
                    break;
                }
                else if(curr_frame->last_used_time < min_time){
                    victim_frame = curr_frame;
                    min_time = curr_frame->last_used_time;
                }
            }
            victim_frame_index += 1;
        }
        victim_frame_index = ((victim_frame->frame_id == frame_size) ? 0 : victim_frame->frame_id + 1);
        return victim_frame;
    }
    void reset_counter(frame_t* victim_frame){}
};

class RANDOM: public Pager{
public:
    MyRandom my_random;
    frame_t* select_victim_frame(){
        if(ofs == my_random.rand_num){
            ofs = 0;
        }
        int random_index = my_random.randvals[ofs] % frame_size;
        frame_t* victim_frame = &frame_table[random_index];
        ofs++;
        return victim_frame;
    }
    void reset_counter(frame_t* victim_frame){};
    RANDOM(const std::string& rand_file) : my_random(rand_file){
    };
};