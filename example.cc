/* Compile with: clang -std=c++11 -Wl,-export-dynamic,-rpath=. libdwarf.so \
   example.cc -o example -lelf */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include "libdwarf.h"

struct MemoryArena{
    uint8_t *base;
    size_t size;
    size_t used;
};

void * push_size(MemoryArena *arena, size_t size, bool clear=false){
    assert((arena->used + size) <= arena->size);

    void *result = arena->base + arena->used;
    arena->used += size;
    if(clear) memset(result, 0, size);

    return result;
}

static MemoryArena dwarf_arena;
void * stack_malloc(size_t size){ return push_size(&dwarf_arena, size); }
void * stack_calloc(size_t nmemb, size_t size){ return push_size(&dwarf_arena, nmemb*size, true); }
void stack_free(void *){}

int main(void){
    // Memory arena initialization
    int bytes_to_allocate = 1024*1024;
    dwarf_arena.base = (uint8_t *) malloc(bytes_to_allocate);
    dwarf_arena.size = bytes_to_allocate;
    dwarf_arena.used = 0;

    int fd = open("inferior", O_RDONLY); // ELF target

    Dwarf_Debug dbg;
    Dwarf_Error err;
    dwarf_init(fd, DW_DLC_READ, 0, 0, &dbg, &err, stack_malloc, stack_calloc, stack_free);

    // Call as many dwarf_* functions as necessary to build
    //  my own representation of debug information without
    //  worrying about dwarf_dealloc or dwarf_finish

    // Recycling of dwarf memory arena for other purposes
    dwarf_arena.used = 0;
    MemoryArena some_other_arena = dwarf_arena;
    dwarf_arena = {};

    // Code that does something useful with the debug symbols

    return 0;
}
