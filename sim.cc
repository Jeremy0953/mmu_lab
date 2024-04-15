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
            proc_manager = std::make_shared<ProcessManager>(max_page_num, total_proc);
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