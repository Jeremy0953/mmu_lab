#pragma once

#include <iostream>
#include <vector>
#include "vm.h"

struct summary_t{
    unsigned long unmaps = 0;
    unsigned long maps = 0;
    unsigned long ins = 0;
    unsigned long outs = 0;
    unsigned long fins = 0;
    unsigned long fouts = 0;
    unsigned long zeros = 0;
    unsigned long segv = 0;
    unsigned long segprot = 0;
};

class Process{
    public:
        Process(int max_page_num, int pid_): pid(pid_) {
            page_table = std::vector<pte_t>(max_page_num);
        };
        int get_pid() const { return pid; }
        summary_t get_summary() const { return summary; }
        void unmap() { summary.unmaps += 1; }
        void map() { summary.maps += 1; }
        void in() { summary.ins += 1; }
        void out() { summary.outs += 1; }
        void fin() { summary.fins += 1; }
        void fout() { summary.fouts += 1; }
        void zero() { summary.zeros += 1; }
        void segv() { summary.segv += 1; }
        void segprot() { summary.segprot += 1; }
        pte_t* get_pte(int vpage) { return &page_table[vpage]; }
        void add_vma(VMA vma) { vma_vector.push_back(vma); }
        VMA* get_vma(int vpage){
            for(int i = 0; i < vma_vector.size(); i++){
                VMA* vma = &vma_vector.at(i);
                if(vpage >= vma->start_vpage && vpage <= vma->end_vpage){
                    return vma;
                }
            }
            return nullptr;
        }
    private:
        int pid;
        summary_t summary;
        std::vector<VMA> vma_vector;
        std::vector<pte_t> page_table;
};

class ProcessManager{
    private:
        std::vector<Process> proc_vector;
    public:
        ProcessManager(int max_page_num, int proc_num){
            for(int i = 0; i < proc_num; i++){
                proc_vector.push_back(Process(max_page_num, i));
            }
        }
        void unmap(int pid) { proc_vector[pid].unmap(); }
        void map(int pid) { proc_vector[pid].map(); }
        void in(int pid) { proc_vector[pid].in(); }
        void out(int pid) { proc_vector[pid].out(); }
        void fin(int pid) { proc_vector[pid].fin(); }
        void fout(int pid) { proc_vector[pid].fout(); }
        void zero(int pid) { proc_vector[pid].zero(); }
        void segv(int pid) { proc_vector[pid].segv(); }
        void segprot(int pid) { proc_vector[pid].segprot(); }
        summary_t get_summary(int pid) { return proc_vector[pid].get_summary(); }
        pte_t* get_pte(int pid, int vpage) { return proc_vector[pid].get_pte(vpage);}
        Process* get_process(int pid) { return &proc_vector[pid]; }
        void add_vma(int pid, VMA vma) { proc_vector[pid].add_vma(vma); }
        void configurate_pte(int pid, int vpage){
            Process* proc = &proc_vector.at(pid);
            pte_t* pte = proc->get_pte(vpage);
            VMA* vma = proc->get_vma(vpage);
            if (vma != nullptr){
                pte->WRITE_PROTECT = vma->write_protected;
                pte->FILEMAPPED = vma->file_mapped;
                pte->CONFIGURATED = 1;
            }
            else{
                std::cout << "Error: VMA not found" << std::endl;
            }
        }
};