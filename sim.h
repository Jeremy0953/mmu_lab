#pragma once

#include <iostream>
#include <fstream>
#include <memory>

#include "ins.h"

#include "proc.h"
#include "vm.h"
#include "pm.h"

#define max_page_num 64
#define MAX_SIZE 30

class sim
{
private:
    std::shared_ptr<ProcessManager> proc_manager; 
    std::unique_ptr<Pager> pager;
    Instruction_set ins_set;
    bool o_option = false;
    bool page_table_option = false;
    bool frame_table_option = false;
    bool statistic_option = false;
    int frame_size = 0;
    unsigned long long inst_count;
    unsigned long long process_exits;
    unsigned long long ctx_switches;
    unsigned long long read_write;
    pager_type pager_type_;
    std::string rFileName;
    Process* curr_proc;
    Process* proc;
public:
    void read_input(const std::string& filename);
    void set_options(bool o_option, bool page_table_option, bool frame_table_option, bool statistic_option);
    void simulate();
    void print_statistics();
    void set_frame_size(int frame_size);
    void set_pager_type(pager_type pager_type) { this->pager_type_ = pager_type; };
    void set_rFileName(const std::string& rFileName) { this->rFileName = rFileName; };
};

