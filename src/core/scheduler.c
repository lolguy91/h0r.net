#include "scheduler.h"
#include <config.h>
#include <drivers/LAPIC.h>
#include <vendor/printf.h>
#include <klibc/string.h>
#include <core/mm/heap.h>

process_t processes[MAX_PROCESSES];
uint32_t num_processes = 0;
uint32_t current_process = 0;
bool sig_first;

void schedule(Registers *regs) {
    if (sig_first) {
        sig_first = false;
    }else{
        memcpy(&processes[current_process].regs, regs, sizeof(Registers));
    }
    next_process:
    while(!processes[current_process].present){
        current_process++;
        if (current_process >= num_processes) current_process = 0;
    }

    if (processes[current_process].signal) {
        //TODO: proper signal handler
        memset(&processes[current_process], 0, sizeof(processes[current_process]));
        goto next_process;
    }

    memcpy(regs, &processes[current_process].regs, sizeof(Registers));

    lapic_timer_oneshot(1, 32);
    lapic_eoi();
}
void sched_init() {
    memset(processes, 0, sizeof(processes));
    sig_first = true;
    num_processes = 0;
    current_process = 0;
    register_ISR(32, schedule);
}

uint32_t sched_add_process(char* name, void (*entry)(void)){
    if (num_processes < MAX_PROCESSES) {
        processes[num_processes].present = true;
        processes[num_processes].name = name;
        processes[num_processes].signal = 0;
        processes[num_processes].regs.rip = (uint64_t)entry;
        processes[num_processes].regs.rsp = (uint64_t)malloc(0x1000);
        processes[num_processes].regs.cs = 0x8;
        processes[num_processes].regs.ss = 0x10;
        num_processes++;
        return num_processes - 1;
    }
    return -1;
}
void sched_kill(uint32_t pid){
    if (pid < MAX_PROCESSES) {
        processes[pid].signal = 1;
    }
}