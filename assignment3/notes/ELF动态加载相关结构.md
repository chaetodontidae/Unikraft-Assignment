## ELF动态加载相关结构

### GElf_Ehdr

```
typedef Elf64_Ehdr	GElf_Ehdr;	/* ELF header */
```

实际上是Elf64_Ehdr结构，描述ELF头信息

```
typedef struct {
	unsigned char   e_ident[EI_NIDENT]; /* ELF identification. */
	Elf64_Half      e_type;	     /* Object file type (ET_*). */
	Elf64_Half      e_machine;   /* Machine type (EM_*). */
	Elf64_Word      e_version;   /* File format version (EV_*). */
	Elf64_Addr      e_entry;     /* Start address. */
	Elf64_Off       e_phoff;     /* File offset to the PHDR table. */
	Elf64_Off       e_shoff;     /* File offset to the SHDRheader. */
	Elf64_Word      e_flags;     /* Flags (EF_*). */
	Elf64_Half      e_ehsize;    /* Elf header size in bytes. */
	Elf64_Half      e_phentsize; /* PHDR table entry size in bytes. */
	Elf64_Half      e_phnum;     /* Number of PHDR entries. */
	Elf64_Half      e_shentsize; /* SHDR table entry size in bytes. */
	Elf64_Half      e_shnum;     /* Number of SHDR entries. */
	Elf64_Half      e_shstrndx;  /* Index of section name string table. */
} Elf64_Ehdr;
```

详细描述可参考维基百科——[Executable and Linkable Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format)

### GElf_Phdr

```
typedef Elf64_Phdr	GElf_Phdr;	/* Program header */
```

实际结构为 Elf64_Phdr，描述程序头信息

```
/* 64 bit PHDR entry. */
typedef struct {
	Elf64_Word	p_type;	     /* Type of segment. */
	Elf64_Word	p_flags;     /* Segment flags. */
	Elf64_Off	p_offset;    /* File offset to segment. */
	Elf64_Addr	p_vaddr;     /* Virtual address in memory. */
	Elf64_Addr	p_paddr;     /* Physical address (if relevant). */
	Elf64_Xword	p_filesz;    /* Size of segment in file. */
	Elf64_Xword	p_memsz;     /* Size of segment in memory. */
	Elf64_Xword	p_align;     /* Alignment constraints. */
} Elf64_Phdr;
```

### auxv_entry

定义于elfloader.c中，是一个key，value结构，这边重点了解一下auxv_entry的相关概念

```
struct auxv_entry {
	long key;
	long val;
};
```

当一个程序运行时，需要从操作系统获取其执行的环境信息。关于具体条目的详细描述可见：[auxiliaryvector.html](https://refspecs.linuxfoundation.org/LSB_1.3.0/IA64/spec/auxiliaryvector.html)

## ELF动态加载相关函数

### strup

```
char *strdup(const char *s);
```

拷贝字符串s，返回新拷贝字符串的指针。拷贝字符串所用的内存空间通过malloc进行分配，后续可以通过free释放。

### uk_thread_create_container

```
struct uk_thread *uk_thread_create_container(struct uk_alloc *a,
					     struct uk_alloc *a_stack,
					     size_t stack_len,
					     struct uk_alloc *a_uktls,
					     bool no_ectx,
					     const char *name,
					     void *priv,
					     uk_thread_dtor_t dtor);
```

分配一个包含栈与TLS的uk_thread线程。分配后并没有设置线程的入口且线程并没有被标记为可运行。这样一个线程只有在指定运行入口并且标记为可运行后才可以传递给调度器进行运行。

参数简介：

- a：内存分配器指针
- a_stack：栈内存分配器指针
- stack_len：需要分配的栈的大小，设置为0时采用默认大小
- a_uktls：线程本地存储内存的分配器，如果为NULL则不分配线程本地存储
- no_ectx：设置为true后不会预存空间保存拓展CPU状态，运行的函数需要是ISR-safe的
- name：线程名
- priv：线程拓展数据的指针
- dtor：线程销毁的回调函数

### elf_load_vfs（原 elf-loader 应用函数）

```
struct elf_prog *elf_load_vfs(struct uk_alloc *a, const char *path,
			      const char *progname);
```

通过文件描述符加载ELF程序，加载后释放ELF描述符。返回elf_prog结构，用于在elf_ctx_init方法中准备应用线程的运行

参数信息：

- a：分配加载ELF程序section所用空间的分配器
- path：虚拟文件系统中加载ELF文件的路径，建议至少使用elf_prog完毕后释放内存
- progname：内核调试时使用的程序名，最好能够与加载的程序名对应

### fstat

```
int fstat(int fd, struct stat *buf);
```

返回文件的相关信息，对于文件本身无权限要求但是具备文件路径中所有目录的执行权限

stat相关结构：

```
struct stat {
	dev_t st_dev;             /* 设备类型文件的id */
	ino_t st_ino;             /* 文件inode编号 */
	nlink_t st_nlink;         /* 硬链接的数目 */

	mode_t st_mode;           /* 文件保护状态信息 */
	uid_t st_uid;             /* 创建者id */
	gid_t st_gid;             /* 创建者用户组id */
	unsigned int    __pad0;   
	dev_t st_rdev;            /* 设备类型id */
	off_t st_size;            /* 文件总大小 */
	blksize_t st_blksize;     /* 文件系统的块大小 */
	blkcnt_t st_blocks;       /* 分配的块数量（512字节） */

	struct timespec st_atim;  /* 最后访问文件的时间 */
	struct timespec st_mtim;  /* 文件最后的修改时间 */
	struct timespec st_ctim;  /* 最后状态的改变时间 */
	long unused[3];
};
```

### elf_open

```
Elf *elf_open(int fd);
```

提示：使用时可能需要移植性问题

返回以ELF_C_READ模式开启的ELF文件描述符，失败时返回NULL

失败错误码记录：

```
     [ELF_E_ARGUMENT]  The argument fd was of an unsupported file type.

     [ELF_E_ARGUMENT]  The argument sz was zero, or the argument image was
                       NULL.

     [ELF_E_IO]        The file descriptor in argument fd was invalid.

     [ELF_E_IO]        The file descriptor in argument fd could not be read.

     [ELF_E_RESOURCE]  An out of memory condition was encountered.

     [ELF_E_SEQUENCE]  Functions elf_open() or elf_openmemory() was called
                       before a working version was established with
                       elf_version(3).
```

### gelf_getehdr

```
GElf_Ehdr	*gelf_getehdr(Elf *_elf, GElf_Ehdr *_dst);
```

将ELF header部分拷贝至dst指针指向的空间中，拷贝成功返回dst指针，失败则返回NULL

### gelf_getphdr

```
GElf_Phdr	*gelf_getphdr(Elf *_elf, int _index, GElf_Phdr *_dst);
```

拷贝ELF文件中的指定下标的程序头结构，返回与gelf_getehdr类似

### uk_sched_thread_add

```
int uk_sched_thread_add(struct uk_sched *s, struct uk_thread *t);
```

向线程调度器中启动线程

## elfloader动态链接启动基本流程图

![image-20230713103432993](./images/image-20230713103432993.png)

大致流程如图所示，后续动态链接的实现对该部分进行了参考
