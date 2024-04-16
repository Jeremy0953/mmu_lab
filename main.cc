#include <iostream>
#include <getopt.h>
#include <string>
#include "sim.h"

int main(int argc, char *argv[]){

    int ch;
    int frame_size;
    std::string options;
    bool o_option = false;
    bool page_table_option = false;
    bool frame_table_option = false;
    bool statistic_option = false;
    pager_type pager_type_;
    while ((ch = getopt(argc, argv, "f:a:o:")) != -1){
        switch (ch) {
            case 'f':
                frame_size = std::stoi(optarg);
                break;
            case 'a':
                switch (optarg[0]){
                case 'f':
                    pager_type_ = FIFO_TYPE;
                    break;
                
                case 'r':
                    pager_type_ = RANDOM_TYPE;
                    break;
                
                case 'c':
                    pager_type_ = CLOCK_TYPE;
                    break;
                
                case 'e':
                    pager_type_ = NRU_TYPE;
                    break;
                
                case 'a':
                    pager_type_ = AGING_TYPE;
                    break;
                
                case 'w':
                    pager_type_ = WS_TYPE;
                    break;
                default:
                    break;
                }
                break;
            case 'o':
                // cout << "**********" << endl;
                options = optarg;
                // cout << endl << "options = " << options << endl;
                for(char opt : options){
                    switch (opt) {
                        case 'O':
                            // cout << endl << "*********" << endl;
                            o_option = true;
                            break;
                        case 'P':
                                                    // cout << endl << "*********" << endl;
                            page_table_option = true;
                            break;
                        case 'F':
                        //   cout << endl << "*********" << endl;
                            frame_table_option = true;
                            break;
                        case 'S':
                        //   cout << endl << "*********" << endl;
                            statistic_option = true;
                            break;
                        default:
                            break;
                    }
                }
//            case '?':
//                printf("error optopt: %c\n", optopt);
//                printf("error opterr: %d\n", opterr);
//                break;
            // default:
            //     exit(-1);
        }
    }
    //   cout << endl << "*********" << endl;
    // cout << "frame_size: " + to_string(frame_size) << endl;
    // cout << "pager: " + pager << endl;
    // cout << page_table_option << endl;
    sim simulator;
    simulator.set_options(o_option, page_table_option, frame_table_option, statistic_option);
    simulator.set_frame_size(frame_size);
    simulator.set_pager_type(pager_type_);
    simulator.read_input(argv[argc - 2]);
    simulator.set_rFileName(argv[argc - 1]);
    simulator.simulate();
    simulator.print_statistics();
}