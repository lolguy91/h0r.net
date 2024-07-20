#include "interrupts.h"
#include <arch/x86_64/interrupts/IDT.h>
#include <arch/x86_64/io/portio.h>
#include <core/utils/log.h>

void initialize_interrupts() {
    init_ISR();
    enable_IDT();
}
void register_ISR(int irq, ISRHandler handler) {
    ISR_RegisterHandler(irq, handler);
}
void enable_interrupts() {
    log_trace("Interrupts enabled!\n");
    __asm__ volatile("sti");
}
void disable_interrupts() {
    log_trace("Interrupts disabled!\n");
    __asm__ volatile("cli");
}
