#include "heap.h"
#include <klibc/string.h>
#include <vendor/printf.h>
#include <utils/log.h>
#include <utils/error.h>

mem_segment *head;

void heap_init(uint64_t heap_start, size_t heap_size) {
    head = (mem_segment *)heap_start;
    head->length = heap_size - sizeof(mem_segment);
    head->next = 0;
    head->previous = 0;
    head->nextfree = 0;
    head->previousfree = 0;
    head->isfree = true;
}
void *malloc(size_t size) {
    log_trace("Allocating %d bytes\n", size);
    uint64_t remainder = size % 8;
    size -= remainder;
    if (remainder != 0)
        size += 8;
    mem_segment *current = head;
    while (true) {
        if (current->length >= size) {
            if (current->length > size + sizeof(mem_segment)) {
                mem_segment *new_seg =
                    (mem_segment *)((uint64_t)current + sizeof(mem_segment) +
                                    size); // making a new segment
                new_seg->isfree = true;
                new_seg->length =
                    (((uint64_t)current->length) - sizeof(mem_segment) + size);
                new_seg->next = current->next;
                new_seg->nextfree = current->nextfree;
                new_seg->previous = current;
                new_seg->previousfree = current->previousfree;

                current->nextfree = new_seg;
                current->next = new_seg;

                current->length = size;
            }
            if (current == head) {
                head = current->nextfree;
            }
            current->isfree = false;

            if (current->previousfree != 0)
                current->previousfree->nextfree = current->nextfree;
            if (current->nextfree != 0)
                current->nextfree->previousfree = current->previousfree;
            if (current->previous != 0)
                current->previous->nextfree = current->nextfree;
            if (current->next != 0)
                current->next->previousfree = current->previousfree;

            return current + 1;
        }
        if (current->nextfree == 0) {
            trigger_psod(HN_ERR_OUT_OF_MEM, "Current->NextFree is 0", NULL);
        }
        current = current->nextfree;
    }
    trigger_psod(HN_ERR_OUT_OF_MEM, "Out of Memory", NULL);
}

void combine_segs(mem_segment *a, mem_segment *b) {
    if (a == 0 || b == 0) {
        return;
    }
    if (a < b) {
        a->length += b->length + sizeof(mem_segment);
        a->next = b->next;
        a->nextfree = b->nextfree;
        b->next->previous = a;
        if (a->isfree) {
            b->next->previousfree = a;
            b->nextfree->previousfree = a;
        }
    }
    if (a > b) {
        b->length += a->length + sizeof(mem_segment);
        b->next = a->next;
        b->nextfree = a->nextfree;
        a->next->previous = b;
        if (a->isfree) {
            a->next->previousfree = b;
            a->nextfree->previousfree = b;
        }
    }
}

void free(void *tofree) {
    if (tofree == 0) {
        log_error("Tried to free 0x0\n");
        return;
    }
    log_trace("Freeing 0x%p\n", tofree);
    mem_segment *current = ((mem_segment *)tofree) - 1;
    current->isfree = true;

    if (current < head)
        head = current;

    if (current->nextfree != 0) {
        if (current->nextfree->previousfree < current) {
            current->nextfree->previousfree = current;
        }
    }
    if (current->previousfree != 0) {
        if (current->previousfree->nextfree < current) {
            current->previousfree->nextfree = current;
        }
    }
    if (current->next != 0) {
        if (current->next->previous < current) {
            current->next->previous = current;
            if (current->next->isfree)
                combine_segs(current, current->next); // removing fragmentation
        }
    }
    if (current->previous != 0) {
        if (current->previous->next < current) {
            current->previous->next = current;
            if (current->previous->isfree)
                combine_segs(current,
                             current->previous); // removing fragmentation
        }
    }
}
void *calloc(size_t size) {
    void *malloced = malloc(size);
    memset(malloced, 0, size);
    return malloced;
}
void *realloc(void *old, size_t size) {
    mem_segment *oldseg = ((mem_segment *)old) - 1;

    size_t smaller_size = size;
    dprintf("oldseg addr: %p", oldseg);
    if (oldseg->length < size)
        smaller_size = oldseg->length;

    free(old);
    void *newmem = malloc(size);
    memcpy(newmem, old, smaller_size);
    return newmem;
}
void *realloc_plus(void *old, size_t size, size_t oldsize) {
    size_t smaller_size = size;
    if (oldsize < size)
        smaller_size = oldsize;

    free(old);
    void *newmem = malloc(size);
    memcpy(newmem, old, smaller_size);
    return newmem;
}
