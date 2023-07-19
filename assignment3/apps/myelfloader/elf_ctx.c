#include <uk/plat/bootstrap.h>
#include <uk/assert.h>
#include <uk/print.h>
#include <uk/essentials.h>

#include "elf_prog.h"

/* Fields for auxiliary vector
 * (https://lwn.net/Articles/519085/)
 */
#define AT_NULL			0
#define AT_IGNORE		1
#define AT_EXECFD		2
#define AT_PHDR			3
#define AT_PHENT		4
#define AT_PHNUM		5
#define AT_PAGESZ		6
#define AT_BASE			7
#define AT_FLAGS		8
#define AT_ENTRY		9
#define AT_NOTELF		10
#define AT_UID			11
#define AT_EUID			12
#define AT_GID			13
#define AT_EGID			14
#define AT_PLATFORM		15
#define AT_HWCAP		16
#define AT_CLKTCK		17
#define AT_DCACHEBSIZE		19
#define AT_ICACHEBSIZE		20
#define AT_UCACHEBSIZE		21
#define AT_SECURE		23
#define AT_RANDOM		25
#define AT_EXECFN		31
#define AT_SYSINFO_EHDR		33
#define AT_SYSINFO		32

struct auxv_entry {
	long key;
	long val;
};

#if CONFIG_ARCH_X86_64
	static const char *auxv_platform = "x86_64";
#else
#error "Unsupported architecture"
#endif /* CONFIG_ARCH_X86_64 */

void elf_ctx_init(struct ukarch_ctx *ctx, struct elf_prog *prog,
		  const char *argv0, int argc, char *argv[], char *environ[],
		  uint64_t rand[2])
{
    int i, envc, elfvec_len;
    
    envc = 0;
	if (environ)
		for (char **env = environ; *env; ++env)
			++envc;
    
    struct auxv_entry auxv[] = {
		{ AT_PLATFORM, (uintptr_t) auxv_platform },
		{ AT_NOTELF, 0x0 },
		{ AT_UCACHEBSIZE, 0x0 },
		{ AT_ICACHEBSIZE, 0x0 },
		{ AT_DCACHEBSIZE, 0x0 },
		/* path to executable */
		{ AT_EXECFN, (long) (prog->path ? prog->path : prog->name) },
		{ AT_SECURE, 0x0 },
		{ AT_EGID, 0x0 },
		{ AT_GID, 0x0 },
		{ AT_EUID, 0x0 },
		{ AT_UID, 0x0 },
		{ AT_ENTRY, prog->entry },
		{ AT_FLAGS, 0x0 },
		{ AT_CLKTCK, 0x64 }, /* Mimic Linux */
		{ AT_HWCAP, 0x0 },
		{ AT_PAGESZ, 4096 },
		/* base addr of interpreter */
		{ AT_BASE, prog->interp.prog ?
			   prog->interp.prog->start : 0x0 },
		{ AT_RANDOM, (uintptr_t) rand },
		{ AT_PHENT, prog->phdr.entsize },
		{ AT_PHNUM, prog->phdr.num },
		{ AT_PHDR, prog->start + prog->phdr.off },
		{ AT_IGNORE, 0x0 }
	};
    struct auxv_entry auxv_null = { AT_NULL, 0x0 };
    
    
    elfvec_len = ((ARRAY_SIZE(auxv) + 1) * sizeof(struct auxv_entry))
		+ (envc + 1) * sizeof(uintptr_t)
		+ (argc + (argv0 ? 1 : 0) + 1) * sizeof(uintptr_t)
		+ sizeof(long);

    ctx->sp = ALIGN_DOWN(ctx->sp - elfvec_len, UKARCH_SP_ALIGN)
		         + elfvec_len;
		         
    ukarch_rctx_stackpush(ctx, auxv_null);
	for (i = (int) ARRAY_SIZE(auxv) - 1; i >= 0; --i)
		ukarch_rctx_stackpush(ctx, auxv[i]);
		
    ukarch_rctx_stackpush(ctx, (long) NULL);
    if (environ) {
	for (i = envc-1; i >= 0; --i) {
		uk_pr_debug("env[%d]=\"%s\"\n", i, environ[i]);
		ukarch_rctx_stackpush(ctx, (uintptr_t) environ[i]);
	}
    }
    ukarch_rctx_stackpush(ctx, (long) NULL);
    if (argc)
	for (i = argc - 1; i >= 0; --i)
		ukarch_rctx_stackpush(ctx, (uintptr_t) argv[i]);
    if (argv0)
	ukarch_rctx_stackpush(ctx, (uintptr_t) argv0);
    ukarch_rctx_stackpush(ctx, (long) argc + (argv0 ? 1 : 0));

    if (prog->interp.required) {
	struct elf_prog *interp = prog->interp.prog;
	ukarch_ctx_init(ctx, ctx->sp, 0x0, interp->entry);
    } else {
	ukarch_ctx_init(ctx, ctx->sp, 0x0, prog->entry);
    }
}
