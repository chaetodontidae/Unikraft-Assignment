#include <uk/config.h>
#include <libelf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#if CONFIG_LIBVFSCORE
#include <fcntl.h>
#include <sys/stat.h>
#endif /* CONFIG_LIBVFSCORE */
#include <uk/errptr.h>
#include <uk/essentials.h>
#include <uk/plat/memory.h>
#if CONFIG_LIBPOSIX_PROCESS
#include <uk/process.h>
#endif /* CONFIG_LIBPOSIX_PROCESS */
#include <uk/thread.h>
#include <uk/sched.h>
#if CONFIG_LIBUKSWRAND
#include <uk/swrand.h>
#endif /* CONFIG_LIBUKSWRAND */

#include "elf_prog.h"

#if CONFIG_LIBPOSIX_ENVIRON
extern char **environ;
#else /* !CONFIG_LIBPOSIX_ENVIRON */
#define environ NULL
#endif /* !CONFIG_LIBPOSIX_ENVIRON */

#ifndef PAGES2BYTES
#define PAGES2BYTES(x) ((x) << __PAGE_SHIFT)
#endif

/**
 *
 * record holds basic information for loaded application
 * which is used for replacing application when the app is updated
 * 
 */
 struct app_record {
     char *path;
     char *app_name;
     struct timespec st_mtim;  /* the program last modified time, which is used to determine whether program has been modified */
     struct uk_sched *s;
     struct uk_thread *thread;
     struct app_record *next;
 };

char *get_simple_name_from_path(char *path)
{
    static char seperator = '/';
    if (!path) {
        return NULL;
    }
    char *last_ch = path;
    while (last_ch) {
        last_ch = strrchr(path, seperator);
        if (!last_ch[1]) {
            last_ch[0] = '\0';
            continue;
        } else {
            break;
        }
    }
    if (!last_ch) {
        return NULL;
    }
    return ++last_ch;
}

static __constructor void _libelf_init(void) {
	if (elf_version(EV_CURRENT) == EV_NONE)
		UK_CRASH("Failed to initialize libelf: Version error");
}

static struct timespec get_current_mtim(char *path)
{
    int fd = open(path, O_RDONLY);
    struct stat f_stat;
    fstat(fd, &f_stat);
    close(fd);
    return f_stat.st_mtim;
}

static void reload_application(struct app_record *record, int argc, char *argv[], uint64_t rand[2])
{
    // record->thread->sched = record->s;
    // printf("address of app_thread->sched is:%x\n", record->thread->sched);
    // uk_sched_thread_terminate(record->thread);
    // uk_sched_thread_remove(record->thread);
    // uk_thread_release(record->thread);
    
    char *path = record->path;
    char *program_name = record->app_name;
    struct uk_thread *app_thread;
    app_thread = uk_thread_create_container(uk_alloc_get_default(),
                                            uk_alloc_get_default(),
                                            PAGES2BYTES(CONFIG_APPELFLOADER_STACK_NBPAGES),
					    uk_alloc_get_default(),
					    false,
					    program_name,
					    NULL, NULL);
    if (app_thread == NULL) {
        uk_pr_err("Fail to create application thread");
        return ;
    }
    
    struct elf_prog *elf_program = NULL;
    elf_program = elf_load_vfs(uk_alloc_get_default(), path, program_name);
    if (!elf_program) {
        // uk_pr_err("Fail to load application:%s \n", program_name);
        printf("Fail to load application:%s \n", program_name);
        return ;
    }
    
    /* step4: init elf context, prepare thread to run */
    uk_swrand_fill_buffer(rand, sizeof(rand));
    elf_ctx_init(&app_thread->ctx,
                 elf_program,
                 program_name,
                 argc,
                 argv,
                 environ, rand);
                 
    /* step5: set thread RUNNABLE bit, run thread */
    app_thread->flags |= UK_THREADF_RUNNABLE;
    uk_posix_process_create(uk_alloc_get_default(),
			    app_thread,
			    uk_thread_current());
    uk_sched_thread_add(uk_sched_current(), app_thread);
    app_thread->sched = uk_sched_current();
    
    record->thread = app_thread;
    record->st_mtim = get_current_mtim(record->path);
}

static void check_program_update(struct app_record *record, int argc, char *argv[], uint64_t rand[2])
{
    if (record == NULL) {
        return ;
    }
    struct timespec recent = get_current_mtim(record->path);
    if ((recent.tv_sec != record->st_mtim.tv_sec) || (recent.tv_nsec != record->st_mtim.tv_nsec)) {
        printf("start to reload\n");
        reload_application(record, argc, argv, rand);
    }
    if (record->next != NULL) {
        check_program_update(record->next, argc, argv, rand);   
    }
}

int main(int argc, char **argv)
{
    /* step1: get program path from command line arguments */
    if (argc < 2 || !argv) {
        uk_pr_err("Need program path to find the program and execute, example:\"/helloworld\"");
        return 1;
    }
    const char *path;
    char* program_full_path;
    const char *program_name;
    uint64_t rand[2];
    path = argv[1];
    program_full_path = strdup(path);
    program_name = get_simple_name_from_path(program_full_path);
    if (program_name == NULL) {
        uk_pr_err("The program path is invalid");
        return 1;
    }
    argv = &argv[2];
    argc -= 2;
    
    /* step2: alloc a thread container for loading and running program later */
    struct uk_thread *app_thread;
    app_thread = uk_thread_create_container(uk_alloc_get_default(),
                                            uk_alloc_get_default(),
                                            PAGES2BYTES(CONFIG_APPELFLOADER_STACK_NBPAGES),
					    uk_alloc_get_default(),
					    false,
					    program_name,
					    NULL, NULL);
    if (app_thread == NULL) {
        uk_pr_err("Fail to create application thread");
        return 1;
    }
    
    /* step3: load program from vfs */
    struct elf_prog *elf_program = NULL;
    elf_program = elf_load_vfs(uk_alloc_get_default(), path, program_name);
    if (!elf_program) {
        // uk_pr_err("Fail to load application:%s \n", program_name);
        printf("Fail to load application:%s \n", program_name);
        return 1;
    }
    
    /* step4: init elf context, prepare thread to run */
    uk_swrand_fill_buffer(rand, sizeof(rand));
    elf_ctx_init(&app_thread->ctx,
                 elf_program,
                 program_name,
                 argc,
                 argv,
                 environ, rand);
                 
    /* step5: set thread RUNNABLE bit, run thread */
    app_thread->flags |= UK_THREADF_RUNNABLE;
    uk_posix_process_create(uk_alloc_get_default(),
			    app_thread,
			    uk_thread_current());
    uk_sched_thread_add(uk_sched_current(), app_thread);
    
    struct app_record *head = malloc(sizeof(struct app_record));
    head->path = path;
    head->app_name = program_name;
    head->thread = app_thread;
    head->st_mtim = get_current_mtim(path);
    head->s = app_thread->sched;
    head->next = NULL;
 
    for(;;) {
        check_program_update(head, argc, argv, rand);
        sleep(8);
    }
        
    return 0;           
}
