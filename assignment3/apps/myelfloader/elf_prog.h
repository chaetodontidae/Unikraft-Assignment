#ifndef ELF_PROG_H
#define ELF_PROG_H

#include <uk/config.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <uk/alloc.h>
#include <uk/arch/ctx.h>
#include <uk/essentials.h>

/**
 * struct of elf program
 * borrow from origin app-elfloader
 *
 */
struct elf_prog {
    struct uk_alloc *a;
    const char *name;
    const char *path; /* path to executable */
    void *vabase;	/* base address of loaded image in virtual memory */
    size_t valen;	/* length of loaded image in virtual memory */
    
    /* Needed by elf_ctx_init(): */
    uintptr_t start;
    uintptr_t entry;
    struct {
        uintptr_t off;
        size_t num;
        size_t entsize;
    } phdr;
    struct {
        bool required;
        char *path;
        struct elf_prog *prog;
    } interp;
};


#if CONFIG_LIBVFSCORE
/**
 *
 * define the same interface with origin in app-elfloader
 * for this assignment I only implement vfs way to load elf program
 *
 */
struct elf_prog *elf_load_vfs(struct uk_alloc *a, const char *path,
			      const char *progname);
#endif /* CONFIG_LIBVFSCORE */

/**
 *
 * interface to init elf context, prepare for running the program on thread
 *
 */
void elf_ctx_init(struct ukarch_ctx *ctx, struct elf_prog *prog,
		  const char *argv0, int argc, char *argv[], char *environ[],
		  uint64_t rand[2]);
		  
#endif
