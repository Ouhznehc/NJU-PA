#include <proc.h>
#include <elf.h>
#include <fs.h>

#define NR_PAGE 8
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;
  int fd = fs_open(filename, 0, 0);
  if(fd == -1) panic("Invalid filename!");
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));

  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);// magic number must be elf

  Elf_Phdr phdr[ehdr.e_phnum];
  fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
  fs_read (fd, phdr, ehdr.e_phnum * sizeof(Elf_Phdr));
  for(size_t i = 0; i < ehdr.e_phnum; i++){
    if(phdr[i].p_type != PT_LOAD) continue;
    fs_lseek(fd, phdr[i].p_offset, SEEK_SET);
    int nr_page = ROUNDUP(phdr[i].p_memsz, PGSIZE) / PGSIZE;
    void *page = new_page(nr_page);
    memset(page, 0, nr_page * PGSIZE);
    for(int j = 0; j < nr_page; j++) {
      map(&pcb->as, (void *)phdr[i].p_vaddr + j * PGSIZE, page + j * PGSIZE, MMAP_READ | MMAP_WRITE);
      printf("loader map from va = %08p to pa = %08p\n", (void *)phdr[i].p_vaddr + j * PGSIZE ,page + j * PGSIZE);
    }
    fs_read (fd, page, phdr[i].p_memsz);
    pcb->max_brk = MAX(phdr[i].p_vaddr + phdr[i].p_memsz, pcb->max_brk);
  }
  //pcb->max_brk = 0xe0000000;
  printf("max_brk initial value is %08p\n", pcb->max_brk);
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg){
  Area kstack;
  kstack.start = &pcb->cp;
  kstack.end = &pcb->cp + STACK_SIZE;

  pcb->cp = kcontext(kstack, entry, arg);

}

// void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){
 
//   AddrSpace *as = &pcb->as;
//   protect(as);
//   void *page = new_page(NR_PAGE) + NR_PAGE * PGSIZE;
//   for(int i = NR_PAGE; i >= 1; i--) {
//   map(as, as->area.end - i * PGSIZE, page - i * PGSIZE, MMAP_READ | MMAP_WRITE);
//   printf("context_uload map from va = %08p to pa = %08p\n", as->area.end - i * PGSIZE ,page - i * PGSIZE, MMAP_READ);
//   }
// /*
//               |               |
//               +---------------+ <---- ustack.end
//               |  Unspecified  |
//               +---------------+
//               |               | <----------+
//               |    string     | <--------+ |
//               |     area      | <------+ | |
//               |               | <----+ | | |
//               |               | <--+ | | | |
//               +---------------+    | | | | |
//               |  Unspecified  |    | | | | |
//               +---------------+    | | | | |
//               |     NULL      |    | | | | |
//               +---------------+    | | | | |
//               |    ......     |    | | | | |
//               +---------------+    | | | | |
//               |    envp[1]    | ---+ | | | |
//               +---------------+      | | | |
//               |    envp[0]    | -----+ | | |
//               +---------------+        | | |
//               |     NULL      |        | | |
//               +---------------+        | | |
//               | argv[argc-1]  | -------+ | |
//               +---------------+          | |
//               |    ......     |          | |
//               +---------------+          | |
//               |    argv[1]    | ---------+ |
//               +---------------+            |
//               |    argv[0]    | -----------+
//               +---------------+
//               |      argc     |
//               +---------------+ <---- cp->GPRx
//               |               |
// */

// // to allocate space as the graph above
//   int envc = 0, argc = 0;
//   while(argv && argv[argc]) {printf("Argument argv[%d] is %s\n", argc, argv[argc]); argc++;}
//   while(envp && envp[envc]) {printf("Argument envp[%d] is %s\n", envc, envp[envc]); envc++;}

//   char *argv_area[argc], *envp_area[envc];
//   char *string_area = (char *)page;
//   //char *string_area = (char *)heap.end;

//   for (int i = 0; i < argc; i++){
//     string_area -= strlen(argv[i]) + 1;
//     argv_area[i] = string_area;
//     strcpy(string_area, argv[i]);
//   }

//   for (int i = 0; i < envc; i++){
//     string_area -= strlen(envp[i]) + 1;
//     envp_area[i] = string_area;
//     strcpy(string_area, envp[i]);
//   }

//   intptr_t *ptr = (intptr_t *)string_area;

//   ptr -= 40;  // Unspecified

//   *ptr = (intptr_t)NULL; ptr--;
//   for(int i = envc - 1; i >= 0; i--){
//     *ptr = (intptr_t)envp_area[i];
//     ptr--;
//   }
//   *ptr = (intptr_t)NULL; ptr--;
//   for(int i = argc - 1; i >= 0; i--){
//     *ptr = (intptr_t)argv_area[i];
//     ptr--;
//   }
//   *ptr = argc;

//   void *entry = (void *)loader(pcb, filename);
//   Area kstack;
//   kstack.start = &pcb->cp;
//   kstack.end = &pcb->cp + STACK_SIZE;
//   Context *context = ucontext(as, kstack, entry);
//   pcb->cp = context;

//   context->GPRx = (intptr_t)ptr;
// }

void context_uload(PCB *pcb, const char *filename, const char *argv[], const char *envp[]) {

  protect(&pcb->as);

  size_t argc = 0, envc = 0, argsz = 0, envsz = 0;


  if (argv) for (; argv[argc]; argc++) {
//    Log("%s", argv[argc]);
    argsz += strlen(argv[argc]) + 1;
  }
  if (envp) for (; envp[envc]; envc++) {
//    Log("%s", envp[envc]);
    envsz += strlen(envp[envc]) + 1;
  }

  for (int i = 0; i < STACK_SIZE / PGSIZE; i++) {
    void *n_pgpa = new_page(1);
    map(&pcb->as, pcb->as.area.end - STACK_SIZE + i * PGSIZE, n_pgpa, MMAP_READ | MMAP_WRITE);
  }

  char      *n_area = new_page(0) - argsz - envsz;
  uintptr_t *n_envp = (uintptr_t *) n_area - envc - 1;
  uintptr_t *n_argv = (uintptr_t *) n_envp - argc - 1;
  size_t    *n_argc = (size_t *)    n_argv - 1;


  for (int i = 0; i < argc; i++, *n_area++ = '\0') {
    n_argv[i] = (uintptr_t) n_area;
    memcpy(n_area, argv[i], strlen(argv[i]));
    n_area += strlen(argv[i]);
  }
  n_argv[argc] = 0;
  for (int i = 0; i < envc; i++, *n_area++ = '\0'){
    n_envp[i] = (uintptr_t) n_area;
    memcpy(n_area, envp[i], strlen(envp[i]));
    n_area += strlen(envp[i]);
  }
  n_envp[envc] = 0;

  *n_argc = argc;

  Area nstack = {
    .start = (void *) pcb,
    .end = ((void *) pcb) + STACK_SIZE
  };
  
  void (*entry)() = (void (*)()) loader(pcb, filename);
  pcb->cp = ucontext(&pcb->as, nstack, entry);
  pcb->cp->GPRx = (uintptr_t) n_argc;
  return;
}

