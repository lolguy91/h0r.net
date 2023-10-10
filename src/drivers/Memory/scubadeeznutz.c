#include "scubadeeznutz.h"
#include <klibc/memory.h>
#include <arch/x86/interrupts/interrupts.h>
#include <drivers/Memory/Heap.h>
#include <drivers/Memory/PFA.h>
#include <logging/logger.h>

// "hippity hoppity,SCUBA DEEZ NUTZ"
// https://github.com/schkwve/luxe/

struct limine_memmap_response *g_mmap;
struct limine_kernel_address_response *g_kernel_addr;

static addr_space_t g_kern_as;
vector_static(mem_map_t, mmap_list);

void scuba_init(struct limine_memmap_response *mmap,
                struct limine_kernel_address_response *kernel_addr) {

    // General Initialization
    vector_init(&mmap_list);
    g_mmap = mmap;
    g_kernel_addr = kernel_addr;

    // Initializing the Adress Space
    g_kern_as.pml4 = malloc(8 * BLOCK_SIZE);
    memset(g_kern_as.pml4, 0, 8 * BLOCK_SIZE);
    vector_init(&g_kern_as.mem_list);

    // Map the whole mem space to MEM_VIRT_OFF
    scuba_map(NULL, MEM_VIRT_OFF, 0, NUM_BLOCKS(get_highest_block()),
              VIRT_FLAGS_DEFAULT | VIRT_FLAGS_USERMODE, true);
    log_info("mapped %d bytes memory to 0x%llx", get_highest_block(),
             MEM_VIRT_OFF);

    // Map the whole mem space again but this time with correct flags(wtf?)
    for (size_t i = 0; i < mmap->entry_count; i++) {
        struct limine_memmap_entry *entry = mmap->entries[i];

        switch (entry->type) {
        case LIMINE_MEMMAP_KERNEL_AND_MODULES: {
            uint64_t virt_addr = kernel_addr->virtual_base + entry->base -
                                 kernel_addr->physical_base;
            scuba_map(NULL, virt_addr, entry->base, NUM_BLOCKS(entry->length),
                      VIRT_FLAGS_DEFAULT | VIRT_FLAGS_USERMODE, true);
            log_info("mapped kernel 0x%llx -> 0x%llx", entry->base, virt_addr,
                     entry->length);
            break;
        }
        case LIMINE_MEMMAP_FRAMEBUFFER: {
            scuba_map(NULL, PHYS_TO_VIRT(entry->base), entry->base,
                      NUM_BLOCKS(entry->length),
                      VIRT_FLAGS_DEFAULT | VIRT_FLAG_WCOMB |
                          VIRT_FLAGS_USERMODE,
                      true);
            log_info("mapped framebuffer 0x%llx -> 0x%llx", entry->base,
                     PHYS_TO_VIRT(entry->base));
            break;
        }
        default: {
            scuba_map(NULL, PHYS_TO_VIRT(entry->base), entry->base,
                      NUM_BLOCKS(entry->length),
                      VIRT_FLAGS_DEFAULT | VIRT_FLAGS_USERMODE, true);
            log_info("mapped 0x%llx -> 0x%llx", entry->base,
                     PHYS_TO_VIRT(entry->base));
            break;
        }
        }
    }
    __asm__ volatile("mov %0, %%cr3" ::"r"(VIRT_TO_PHYS(g_kern_as.pml4))
                     : "memory");
    log_info("done");
}

void scuba_map(addr_space_t *as, uint64_t virt_addr, uint64_t phys_addr,
               uint64_t np, uint64_t flags, bool us) {
    if (us && (as == NULL)) {
        mem_map_t mm = {virt_addr, phys_addr, flags, np};
        vector_push_back(&mmap_list, mm);
    }

    for (size_t i = 0; i < np * BLOCK_SIZE; i += BLOCK_SIZE) {
        _scuba_map(as, virt_addr + i, phys_addr + i, flags);
    }
}

void scuba_unmap(addr_space_t *as, uint64_t virt_addr, uint64_t np, bool us) {
    if (us && (as == NULL)) {
        size_t len = vector_length(&mmap_list);
        for (size_t i = 0; i < len; i++) {
            mem_map_t m = vector_at(&mmap_list, i);
            if (m.virt_addr != virt_addr) {
                vector_erase(&mmap_list, i);
                break;
            }
        }
    }

    for (size_t i = 0; i < np * BLOCK_SIZE; i += BLOCK_SIZE) {
        _scuba_unmap(as, virt_addr + i);
    }
}

addr_space_t *create_ads() {
    addr_space_t *as = malloc(sizeof(addr_space_t));
    if (!as)
        return NULL;

    memset(as, 0, sizeof(addr_space_t));
    as->pml4 = malloc(BLOCK_SIZE * 8);
    if (!as->pml4) {
        free(as);
        return NULL;
    }
    memset(as->pml4, 0, BLOCK_SIZE * 8);

    size_t len = vector_length(&mmap_list);
    for (size_t i = 0; i < len; i++) {
        mem_map_t m = vector_at(&mmap_list, i);
        scuba_map(as, m.virt_addr, m.phys_addr, m.np, m.flags, false);
    }

    return as;
}

void _scuba_map(addr_space_t *as, uint64_t virt_addr, uint64_t phys_addr,
                uint64_t flags) {
    addr_space_t *as2 = (as == NULL ? &g_kern_as : as);

    uint64_t *pml4 = as2->pml4;
    uint16_t pml4e = (virt_addr >> 39) & 0x1ff;
    uint64_t *pdpt = (uint64_t *)PHYS_TO_VIRT(pml4[pml4e] & ~(0xfff));
    if (!(pml4[pml4e] & VIRT_FLAG_PRESENT)) {
        pdpt = (uint64_t *)PHYS_TO_VIRT(request_pages(8));
        memset(pdpt, 0, BLOCK_SIZE * 8);
        pml4[pml4e] = ((VIRT_TO_PHYS(pdpt) & ~(0xfff)) | VIRT_FLAGS_USERMODE);
        vector_push_back(&as2->mem_list, VIRT_TO_PHYS(pdpt));
    }

    uint16_t pdpe = (virt_addr >> 30) & 0x1ff;
    uint64_t *pd = (uint64_t *)PHYS_TO_VIRT(pdpt[pdpe] & ~(0xfff));
    if (!(pdpt[pdpe] & VIRT_FLAG_PRESENT)) {
        pd = (uint64_t *)PHYS_TO_VIRT(request_pages(8));
        memset(pd, 0, BLOCK_SIZE * 8);
        pdpt[pdpe] = ((VIRT_TO_PHYS(pd) & ~(0xfff)) | VIRT_FLAGS_USERMODE);
        vector_push_back(&as2->mem_list, VIRT_TO_PHYS(pd));
    }

    uint16_t pde = (virt_addr >> 21) & 0x1ff;
    uint64_t *pt = (uint64_t *)PHYS_TO_VIRT(pd[pde] & ~(0xfff));
    if (!(pd[pde] & VIRT_FLAG_PRESENT)) {
        pt = (uint64_t *)PHYS_TO_VIRT(request_pages(8));
        memset(pt, 0, BLOCK_SIZE * 8);
        pd[pde] = ((VIRT_TO_PHYS(pt) & ~(0xfff)) | VIRT_FLAGS_USERMODE);
        vector_push_back(&as2->mem_list, VIRT_TO_PHYS(pt));
    }

    uint16_t pte = (virt_addr >> 12) & 0x1ff;
    pt[pte] = (phys_addr | flags);

    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    if (cr3 == (uint64_t)(VIRT_TO_PHYS(as2->pml4))) {
        __asm__ volatile("invlpg (%0)" ::"b"((void *)virt_addr) : "memory");
    }
}

void _scuba_unmap(addr_space_t *as, uint64_t virt_addr) {
    addr_space_t *as2 = (as == NULL ? &g_kern_as : as);

    uint64_t *pml4 = as2->pml4;
    uint16_t pml4e = (virt_addr >> 39) & 0x1ff;
    if (!(pml4[pml4e] & VIRT_FLAG_PRESENT)) {
        return;
    }

    uint64_t *pdpt = (uint64_t *)PHYS_TO_VIRT(pml4[pml4e] & ~(0x1ff));
    uint16_t pdpe = (virt_addr >> 30) & 0x1ff;
    if (!(pdpt[pdpe] & VIRT_FLAG_PRESENT)) {
        return;
    }

    uint64_t *pd = (uint64_t *)PHYS_TO_VIRT(pdpt[pdpe] & ~(0x1ff));
    uint16_t pde = (virt_addr >> 21) & 0x1ff;
    if (!(pd[pde] & VIRT_FLAG_PRESENT)) {
        return;
    }

    uint64_t *pt = (uint64_t *)PHYS_TO_VIRT(pd[pde] & ~(0x1ff));
    uint16_t pte = (virt_addr >> 12) & 0x1ff;
    if (!(pt[pte] & VIRT_FLAG_PRESENT)) {
        return;
    }

    pt[pte] = 0;
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    if (cr3 == (uint64_t)(VIRT_TO_PHYS(as2->pml4))) {
        __asm__ volatile("invlpg (%0)" ::"b"((void *)virt_addr) : "memory");
    }

    for (int i = 0; i < 512 * 8; i++) {
        if (pt[i] != 0) {
            return;
        }
    }

    pd[pde] = 0;
    free_pages((void *)VIRT_TO_PHYS(pt), 8);

    for (int i = 0; i < 512 * 8; i++) {
        if (pd[i] != 0) {
            return;
        }
    }

    pdpt[pdpe] = 0;
    free_pages((void *)VIRT_TO_PHYS(pd), 8);

    for (int i = 0; i < 512 * 8; i++) {
        if (pdpt[i] != 0) {
            return;
        }
    }

    pml4[pml4e] = 0;
    free_pages((void *)VIRT_TO_PHYS(pdpt), 8);
}

uint64_t scuba_get_phys_addr(addr_space_t *as, uint64_t virt_addr) {
    addr_space_t *as2 = (as == NULL ? &g_kern_as : as);

    uint64_t *pml4 = as2->pml4;
    uint16_t pml4e = (virt_addr >> 39) & 0x1ff;
    if (!(pml4[pml4e] & VIRT_FLAG_PRESENT))
        return (uint64_t)NULL;

    uint64_t *pdpt = (uint64_t *)PHYS_TO_VIRT(pml4[pml4e] & ~(0x1ff));
    uint16_t pdpe = (virt_addr >> 30) & 0x1ff;
    if (!(pdpt[pdpe] & VIRT_FLAG_PRESENT))
        return (uint64_t)NULL;

    uint64_t *pd = (uint64_t *)PHYS_TO_VIRT(pdpt[pdpe] & ~(0x1ff));
    uint16_t pde = (virt_addr >> 21) & 0x1ff;
    if (!(pd[pde] & VIRT_FLAG_PRESENT))
        return (uint64_t)NULL;

    uint64_t *pt = (uint64_t *)PHYS_TO_VIRT(pd[pde] & ~(0x1ff));
    uint16_t pte = (virt_addr >> 12) & 0x1ff;
    if (!(pt[pte] & VIRT_FLAG_PRESENT))
        return (uint64_t)NULL;

    return (pt[pte] & ~(0xFFF));
}