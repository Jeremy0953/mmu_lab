#pragma once

#include <iostream>
#include <string>
#include <queue>

enum operation_type {
    PROCID,
    READ,
    WRITE,
    EXIT
};

struct ins_t
{
    operation_type type;
    int vpage;
};

class Instruction_set {
    private:
        std::queue<ins_t> ins_queue;
    public:
        ins_t get_next_instruction();
        void add_instruction(operation_type type, int vpage);
        unsigned long get_instruction_count() { return ins_queue.size(); }
};

void Instruction_set::add_instruction(operation_type type, int vpage)
{
    ins_t ins;
    ins.type = type;
    ins.vpage = vpage;
    ins_queue.push(ins);
}

ins_t Instruction_set::get_next_instruction()
{
    if (ins_queue.empty()) {
        throw std::runtime_error("No more instructions.");
    }
    ins_t ins = std::move(ins_queue.front()); 
    ins_queue.pop();
    return ins;  
}
