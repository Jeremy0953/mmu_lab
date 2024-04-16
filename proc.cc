#include <vector>
#include "proc.h"
#include "vm.h"

void Process::exit(bool o_option, Pager &pager) {
            for(int i = 0; i < page_table.size(); i++){
                pte_t* pte = get_pte(i);

                // 1) UNMAP pte from frame page
                if(pte->VALID){
                    if(o_option) printf(" UNMAP %d:%d\n", pid, i );
                    unmap();
                    pager.unmap(pte->frame_number);
                    // 2) check MODIFIED and FILEMAPPED
                    if (pte->MODIFIED && pte->FILEMAPPED){
                        fout();
                        if (o_option) std::cout << " FOUT" << std::endl;
                    }
                }

                // 3) reset pte bits
                pte->VALID = 0;
                pte->frame_number = -1;
                pte->WRITE_PROTECT = 0;
                pte->CONFIGURATED = 0;
                pte->PAGEDOUT = 0;
                pte->REFERENCED = 0;
                pte->FILEMAPPED = 0;
                pte->MODIFIED = 0;
            }
};