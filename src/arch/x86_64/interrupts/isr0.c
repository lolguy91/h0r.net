
#include <arch/x86_64/i8259.h>
#include <arch/x86_64/interrupts/interrupts.h>
#include <drivers/io/portio.h>
#include <utils/logging/logger.h>

void schedule(Registers *regs);

void special_isr_0_handlr(__attribute__((unused)) Registers *state) {
    schedule(state);
}

void spisr_init() { register_ISR(PIC_REMAP_OFFSET, special_isr_0_handlr); }