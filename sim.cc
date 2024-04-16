#include "sim.h"
#include <string.h>


void sim::read_input(const std::string& filename){

    int total_proc = -1;
    int curr_proc_id = 0;
    int vma_num = 0;
    int vma_count = 0;
    bool is_new_proc = true;
    std::string line;
    std::ifstream inputFile(filename);

    if(!inputFile.is_open()){
        std::cout << "Cannot Open File" << std::endl;
        exit(1);
    }
    while(getline(inputFile, line)){

        if(line[0] == '#') continue;
        char* c = const_cast<char*>(line.c_str());
        char* token = strtok(c, " \t\n");

        // get number of processes
        if(total_proc == -1){
            total_proc = std::stoi(token);
            proc_manager = std::make_shared< ProcessManager>(max_page_num, total_proc);
            //proc_manager->describe();
            continue;
        }
        
        // read processes
        if(curr_proc_id < total_proc){
            std::vector<char*> temp;
            // 1) create new process 2) get total vma of new process
            if(is_new_proc){
                vma_num = std::stoi(token);
                vma_count = 0;
                is_new_proc = false;
                continue;
            }
            //read vma details
            else{
                while(token){
                    temp.push_back(token);
                    token = strtok(NULL, " \t\n");
                }
                VMA vma;
                vma.start_vpage = std::stoi(temp.at(0));
                vma.end_vpage = std::stoi(temp.at(1));
                vma.write_protected = std::stoi(temp.at(2));
                vma.file_mapped = std::stoi(temp.at(3));
                proc_manager->add_vma(curr_proc_id, vma);
                vma_count += 1;
                if(vma_num == vma_count){
                    is_new_proc = true;
                    curr_proc_id += 1;
                }
            }
        }
        //read instructions
        else{
            char type = token[0];
            token = strtok(NULL, " \t\n");
            int vpage = std::stoi(token);
            switch (type)
            {
            case 'c':
                ins_set.add_instruction(PROCID, vpage);
                break;
            case 'r':
                ins_set.add_instruction(READ, vpage);
                break;
            case 'w':
                ins_set.add_instruction(WRITE, vpage);
                break;
            case 'e':
                ins_set.add_instruction(EXIT, vpage);
                break;
            default:
                break;
            }
        }
        inst_count = ins_set.get_instruction_count();
    }
    inputFile.close();
    //proc_manager->print_vma();
}

void sim::set_options(bool o_option, bool page_table_option, bool frame_table_option, bool statistic_option) {
    this->o_option = o_option;
    this->page_table_option = page_table_option;
    this->frame_table_option = frame_table_option;
    this->statistic_option = statistic_option;
}

void sim::set_frame_size(int frame_size) {
    this->frame_size = frame_size;
}

void sim::simulate(){
    switch (pager_type_)
    {
    case FIFO_TYPE:
        pager = std::make_unique<FIFO>(proc_manager);
        break;
    case RANDOM_TYPE:
        pager = std::make_unique<RANDOM>(rFileName, proc_manager);
        break;
    case CLOCK_TYPE:
        pager = std::make_unique<CLOCK>(proc_manager);
        break;
    case NRU_TYPE:
        pager = std::make_unique<NRU>(proc_manager);
        break;
    case AGING_TYPE:
        pager = std::make_unique<AGING>(proc_manager);
        break;
    case WS_TYPE:
        pager = std::make_unique<WS>(proc_manager);
        break;
    default:
        break;
    }
    pager->initialize_frame_table(frame_size);
    pager->initialize_free_pool();
    int i = -1;
    while (!ins_set.empty())
    {   
        i++;
        ins_t ins = ins_set.get_next_instruction();
        pager->ins_count_increment();
        switch (ins.type)
        {
        case PROCID:
            ctx_switches += 1;
            curr_proc = proc_manager->get_process(ins.id);
            if(o_option) printf("%d: ==> c %d\n", i, ins.id);
            break;
        
        case READ:
        case WRITE: {
            read_write += 1;
            int vpage = ins.id;
            if(o_option) printf("%d: ==> %c %d\n", i, ins.toChar(), vpage);
            pte_t *pte = curr_proc->get_pte(vpage);
            
            if (!(pte->CONFIGURATED)) {
                proc_manager->configurate_pte(curr_proc->get_pid(), vpage);
            }
            if (!(pte->VALID)) {

                if (curr_proc->get_vma(vpage)== nullptr) {
                    curr_proc->segv();
                    if(o_option) std::cout << " SEGV" << std::endl;
                    continue;
                }
                frame_t *victim_frame;
                //choose from free_pool
                if (pager->has_free_frame()) {
                    frame_t* free_frame = pager->get_free_frame();
                    free_frame->pid = curr_proc->get_pid();
                    free_frame->vpage = vpage;
                    pte->frame_number = free_frame->frame_id;
                    victim_frame = free_frame;
                    pager->add_to_victim_table(victim_frame);
                }
                else {

                    // 1) choose victim frame
                    victim_frame = pager->select_victim_frame();
                    int prev_proc_id = victim_frame->pid;
                    Process* prev_proc = proc_manager->get_process(prev_proc_id);
                    pte_t *prev_pte = proc_manager->get_pte(prev_proc_id, victim_frame->vpage);
                    // 2) unmap pte
                    if(o_option) std::cout << " UNMAP " << victim_frame->pid << ":" << victim_frame->vpage << std::endl;
                    prev_proc->unmap();

                    // 3) check modified and file mapped
                    if (prev_pte->MODIFIED && prev_pte->FILEMAPPED) {
                        prev_proc->fout();
                        if(o_option) std::cout << " FOUT" << std::endl;
                    } else if (prev_pte->MODIFIED) {
                        prev_pte->PAGEDOUT = 1;
                        prev_proc->out();
                        if(o_option) std::cout << " OUT" << std::endl;
                    }

                    // 4) reset previous pte bit
                    prev_pte->VALID = 0;
                    prev_pte->MODIFIED = 0;
                    prev_pte->REFERENCED = 0;
                    prev_pte->frame_number = 0;
                }

                // 5) new_pte <-> frame page
                victim_frame->vpage = vpage;
                victim_frame->pid = curr_proc->get_pid();
                pte->frame_number = victim_frame->frame_id;
                pte->VALID = 1;

                // 6) check new_pte bit
                if (pte->FILEMAPPED) {
                    curr_proc->fin();
                    if(o_option) std::cout << " FIN" << std::endl;
                } else if (pte->PAGEDOUT) {
                    curr_proc->in();
                    if(o_option) std::cout << " IN" << std::endl;
                } else {
                    curr_proc->zero();
                    if(o_option) std::cout << " ZERO" << std::endl;
                }

                // 7) map frame page
                curr_proc->map();
                pager->reset_counter(victim_frame);
                if(o_option) std::cout << " MAP " + std::to_string(victim_frame->frame_id) << std::endl;
            }

            // 8) r/w operation => pte has been referenced
            pte->REFERENCED = 1;

            // 9) w operation check write_protect => MODIFIED or SEGPROT
            if(pte->WRITE_PROTECT && ins.type == WRITE){
                curr_proc->segprot();
                if(o_option) std::cout << " SEGPROT" << std::endl;
            }else if(ins.type == WRITE){
                pte->MODIFIED = 1;
                //cout << "proc_id: " + to_string(curr_proc->pid) + " page M : " + to_string(curr_proc->page_table[vpage].MODIFIED) << endl;
            }
            break;
        }
        
        case EXIT:
            if(o_option) printf("%d: ==> e %d\n", i, curr_proc->get_pid());
            if(o_option) printf("EXIT current process %d\n", curr_proc->get_pid());
            process_exits += 1;

            // check page table and reset bits
            curr_proc->exit(o_option, *pager);

            // 3) curr_proc = nullptr
            curr_proc = nullptr;
            break;

        default:
            std::cout<<"error in instruction type"<<std::endl;
            break;
        }
    }
}

void sim::print_statistics() {
    if(page_table_option){
        proc_manager->print_all_ptes();
    }
    if(frame_table_option){
        pager->print_frame_table();
    }
    if(statistic_option){
        unsigned long long cost = 0;
        proc_manager->print_static(cost);
        cost += read_write * 1 + ctx_switches * 130 + process_exits * 1230;
        printf("TOTALCOST %llu %llu %llu %llu %lu\n",
               inst_count, ctx_switches, process_exits, cost, sizeof(pte_t));
    }
}