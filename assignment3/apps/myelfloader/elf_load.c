#include <libelf.h>
#include <gelf.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#if CONFIG_LIBVFSCORE
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif /* CONFIG_LIBVFSCORE */
#include <uk/assert.h>
#include <uk/print.h>
#include <uk/essentials.h>
#include <uk/arch/limits.h>
#include <uk/errptr.h>
#if CONFIG_LIBUKVMEM
#include <uk/vmem.h>
#include <uk/arch/limits.h>
#endif /* CONFIG_LIBUKVMEM */

#include "libelf_helper.h"
#include "elf_prog.h"

static int find_lower_and_upper_bound(Elf *elf, struct elf_prog *prog)
{
    GElf_Ehdr e_hdr;
    GElf_Phdr p_hdr;
    uintptr_t prog_lowerl;
    uintptr_t prog_upperl;
    int initialized = 0;
    GElf_Ehdr *hdr_ptr = gelf_getehdr(elf, &e_hdr);
    if (hdr_ptr != &e_hdr) {
        return 1;
    }
    for (size_t i = 0; i < e_hdr.e_phnum; i++) {
        GElf_Phdr *phdr_ptr = gelf_getphdr(elf, i, &p_hdr);
        if (phdr_ptr != &p_hdr) {
            continue;
        }
        if (p_hdr.p_type == PT_INTERP) {
            if (prog->interp.required) {
                // Now we do not back multi interp load
                return 1;
            }
            prog->interp.required = true;
            continue;
        }
        if (!(p_hdr.p_type != PT_LOAD)) {
            continue;
        }
        if (!initialized) {
           prog_lowerl = p_hdr.p_paddr;
           prog_upperl = prog_lowerl + p_hdr.p_memsz;
           initialized = 1;
        } else {
           if (prog_lowerl > p_hdr.p_paddr) {
               prog_lowerl = p_hdr.p_paddr;
           }
           uintptr_t p_upper_bound = p_hdr.p_paddr + p_hdr.p_memsz;
           if (p_upper_bound > prog_upperl) {
               prog_upperl = p_upper_bound;
           }
        }
    }
    prog->phdr.off = e_hdr.e_phoff;
    prog->phdr.num = e_hdr.e_phnum;
    prog->phdr.entsize = e_hdr.e_phentsize;
    prog->valen = PAGE_ALIGN_UP(prog_upperl - prog_lowerl);
    return 0;
}

static int load_content_to_mem(int fd, off_t roff, void *dst, size_t len)
{
    char *ptr;
    ssize_t rc;

    ptr = (char *) dst;
    while (len) {
        rc = pread(fd, ptr, len, roff);
        if (unlikely(rc < 0)) {
            if (errno == EINTR)
                continue;
            return -errno;
        }
	if (unlikely(rc == 0))
	    break; 
	len -= rc;
	ptr += rc;
	roff += rc;
    }
    return 0;
}

static int load_section_to_mem(Elf *elf, struct elf_prog *prog, int fd)
{
    GElf_Ehdr e_hdr;
    GElf_Phdr p_hdr;
    GElf_Ehdr *hdr_ptr = gelf_getehdr(elf, &e_hdr);
    size_t phnum;
    if (hdr_ptr != &e_hdr) {
        return 1;
    }
    prog->vabase = uk_memalign(prog->a, __PAGE_SIZE, prog->valen);
    elf_getphnum(elf, &phnum);
    if (prog->interp.required) {
        for (size_t i = 0; i < phnum; i++) {
            GElf_Phdr *phdr_ptr = gelf_getphdr(elf, i, &p_hdr);
            if (phdr_ptr != &p_hdr) {
                continue;
            }
            if (p_hdr.p_type != PT_INTERP) {
                continue;
            }
            prog->interp.path = malloc(p_hdr.p_filesz);
            load_content_to_mem(fd, p_hdr.p_offset, (void*)prog->interp.path, p_hdr.p_filesz);
            prog->interp.path[p_hdr.p_filesz - 1] = '\0';
            break;
        }
    }
    int initialized = 0;
    prog->entry = (uintptr_t) prog->vabase + e_hdr.e_entry;
    for (size_t i = 0; i < phnum; i++) {
        GElf_Phdr *phdr_ptr = gelf_getphdr(elf, i, &p_hdr);
        if (phdr_ptr != &p_hdr) {
            continue;
        }
        if (p_hdr.p_type != PT_LOAD) {
            continue;
        }
        uintptr_t va_start = (uintptr_t)prog->vabase + p_hdr.p_paddr;
        uintptr_t va_end   = va_start + p_hdr.p_filesz;
        if (!initialized || va_start < prog->start) {
            prog->start = va_start;
            initialized = 1;
        }
        load_content_to_mem(fd, p_hdr.p_offset, (void*)va_start, p_hdr.p_filesz);
        va_start = va_end;
        va_end   += (p_hdr.p_memsz - p_hdr.p_filesz);
        va_end   = PAGE_ALIGN_UP(va_end);
        memset((void *)va_start, 0, va_end - va_start);
    }
    return 0;
}

static int map_page(Elf *elf, struct elf_prog *prog)
{
    struct uk_vas *vas;
    GElf_Ehdr ehdr;
    GElf_Phdr phdr;
    size_t phnum;
    vas = uk_vas_get_active();
    GElf_Ehdr *hdr_ptr = gelf_getehdr(elf, &ehdr);
    elf_getphnum(elf, &phnum);
    if (hdr_ptr != &ehdr) {
        return 1;
    }
    for (size_t i = 0; i < phnum; i++) {
        GElf_Phdr *phdr_ptr = gelf_getphdr(elf, i, &phdr);
        if (phdr_ptr != &phdr) {
            continue;
        }
        if (phdr.p_type != PT_LOAD) {
            continue;
        }
        uintptr_t va_start = (uintptr_t)prog->vabase + phdr.p_paddr;
        uintptr_t va_end   = va_start + phdr.p_memsz;
        va_start = PAGE_ALIGN_DOWN(va_start);
        va_end   = PAGE_ALIGN_UP(va_end);
        uintptr_t valen   = va_end - va_start;
        uk_vma_set_attr(vas, va_start, valen,
                      ((phdr.p_flags & PF_R) ? PAGE_ATTR_PROT_READ  : 0x0) | 
                      ((phdr.p_flags & PF_W) ? PAGE_ATTR_PROT_WRITE : 0x0) | 
                      ((phdr.p_flags & PF_X) ? PAGE_ATTR_PROT_EXEC  : 0x0), 
                        0);
    }
    return 0;
}

static struct elf_prog *do_elf_load_vfs(struct uk_alloc *a, const char *path, const char *progname)
{
    if (!path || !progname) {
        return NULL;
    }
    int fd;
    struct stat f_stat;
    
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        return NULL;
    }
    int state_code = fstat(fd, &f_stat);
    if (state_code == -1 || !(f_stat.st_mode & S_IXUSR)) {
        return NULL;
    }
    
    struct elf_prog *prog;
    prog = uk_calloc(a, 1, sizeof(struct elf_prog));
    prog->a = a;
    prog->name = progname;
    prog->path = path;
    
    Elf *elf;
    elf = elf_open(fd);
    if (!elf) {
        return NULL;
    }
    
    int r1 = find_lower_and_upper_bound(elf, prog);
    
    int r2 = load_section_to_mem(elf, prog, fd);
    
    int r3 = map_page(elf, prog);
    
    elf_end(elf);
    close(fd);
    if (!r1 && !r2 && !r3) {
        return prog;
    } else {
        return NULL;
    }
}

struct elf_prog *elf_load_vfs(struct uk_alloc *a, const char *path,
			      const char *progname)
{
    if (!path || !progname) {
        return NULL;
    }
    struct elf_prog *prog = do_elf_load_vfs(a, path, progname);
    if (!prog) {
        return NULL;
    }
    if (prog->interp.required) {
        prog->interp.prog = do_elf_load_vfs(a, prog->interp.path, "<interp>");
    }
    return prog;
}
