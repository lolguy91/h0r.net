#include <arch/x86_64/io/portio.h>
#include <core/error.h>
#include <core/mm/heap.h>
#include <core/drivers/ACPI/SDT.h>
#include <core/libk/macros.h>
#include <vendor/printf.h>

// WARN: Arch specific code in core/utils/ + TODO: smh move to arch/ or make
// architecture neutral
void *laihost_malloc(usize size) { return malloc(size); }
void *laihost_realloc(void *old, usize newsize, usize oldsize) {
    return realloc_plus(old, newsize, oldsize);
}
void laihost_free(void *tofree, UNUSED usize size) { free(tofree); }

void laihost_log(int lvl, const char *msg) { dprintf("[%i] %s\n", lvl, msg); }

void laihost_panic(const char *msg) {
    trigger_psod(HN_ERR_LAI_PANIC, (char *)msg, NULL);
}

void *laihost_scan(char *sig, usize index) { return find_nth_SDT(sig, index); }

void laihost_outb(u16 port, u8 val) { o8(port, val); }
void laihost_outw(u16 port, u16 val) { o16(port, val); }
void laihost_outd(u16 port, u32 val) { o32(port, val); }
u8 laihost_inb(u16 port) { return in8(port); }
u16 laihost_inw(u16 port) { return in16(port); }
u32 laihost_ind(u16 port) { return in32(port); }
