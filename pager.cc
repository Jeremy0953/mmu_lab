#include "pager.h"
#include <iostream>
#include <vector>
#include "vm.h"
#include "proc.h"

frame_t* FIFO::select_victim_frame() {
        if(victim_frame_index == frame_size){
            victim_frame_index = 0;
        }
        frame_t* frame = victim_table.at(victim_frame_index);
        victim_frame_index += 1;
        return frame;
    }

frame_t* CLOCK::select_victim_frame() {

        frame_t* victim_frame;

        while(true){
            if(victim_frame_index == frame_size){
                victim_frame_index = 0;
            }
            victim_frame = victim_table[victim_frame_index];
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

frame_t* NRU::select_victim_frame() {

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

frame_t* AGING::select_victim_frame(){

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
frame_t* WS::select_victim_frame() {
    unsigned long long min_time = ULLONG_MAX;
    frame_t* victim_frame = victim_table[victim_frame_index];

    for(int i = 0; i < frame_size; i++) {
        if(victim_frame_index >= frame_size) {
            victim_frame_index = 0;
        }

        frame_t* curr_frame = victim_table[victim_frame_index];
        pte_t* curr_pte = proc_manager->get_pte(curr_frame->pid, curr_frame->vpage);
        unsigned long long frame_age = instruction_count - curr_frame->last_used_time;

        if(curr_pte->REFERENCED == 1) {
            curr_pte->REFERENCED = 0;
            curr_frame->last_used_time = instruction_count;
        } else {
            if(frame_age > TAU) {
                victim_frame = curr_frame;
                break;
            }
            else if(curr_frame->last_used_time < min_time) {
                victim_frame = curr_frame;
                min_time = curr_frame->last_used_time;
            }
        }
        victim_frame_index += 1;
    }
    victim_frame_index = ((victim_frame->frame_id == frame_size) ? 0 : victim_frame->frame_id + 1);
    return victim_frame;
}

frame_t* RANDOM::select_victim_frame(){
        if(ofs == my_random.rand_num){
            ofs = 0;
        }
        int random_index = my_random.randvals[ofs] % frame_size;
        frame_t* victim_frame = &frame_table[random_index];
        ofs++;
        return victim_frame;
}