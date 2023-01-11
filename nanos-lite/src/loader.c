#include <proc.h>
#include <elf.h>
#include <fs.h>

#define nr_page 8
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
  fs_read(fd, phdr, ehdr.e_phnum * sizeof(Elf_Phdr));
  for(size_t i = 0; i < ehdr.e_phnum; i++){
    if(phdr[i].p_type == PT_LOAD){
      fs_lseek(fd, phdr[i].p_offset, SEEK_SET);
      fs_read(fd, (void *)phdr[i].p_vaddr, phdr[i].p_memsz);
      memset((void *)(phdr[i].p_vaddr + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
    }
  }
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

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){
 
  AddrSpace *as = &pcb->as;
  protect(as);
  void *page = new_page(nr_page) + nr_page * PGSIZE;
  for(int i = nr_page; i >= 1; i--) map(as, as->area.end - i * PGSIZE, page - i * PGSIZE, MMAP_READ | MMAP_WRITE);
/*
              |               |
              +---------------+ <---- ustack.end
              |  Unspecified  |
              +---------------+
              |               | <----------+
              |    string     | <--------+ |
              |     area      | <------+ | |
              |               | <----+ | | |
              |               | <--+ | | | |
              +---------------+    | | | | |
              |  Unspecified  |    | | | | |
              +---------------+    | | | | |
              |     NULL      |    | | | | |
              +---------------+    | | | | |
              |    ......     |    | | | | |
              +---------------+    | | | | |
              |    envp[1]    | ---+ | | | |
              +---------------+      | | | |
              |    envp[0]    | -----+ | | |
              +---------------+        | | |
              |     NULL      |        | | |
              +---------------+        | | |
              | argv[argc-1]  | -------+ | |
              +---------------+          | |
              |    ......     |          | |
              +---------------+          | |
              |    argv[1]    | ---------+ |
              +---------------+            |
              |    argv[0]    | -----------+
              +---------------+
              |      argc     |
              +---------------+ <---- cp->GPRx
              |               |
*/

// to allocate space as the graph above
  int envc = 0, argc = 0;
  while(argv && argv[argc]) {printf("Argument argv[%d] is %s\n", argc, argv[argc]); argc++;}
  while(envp && envp[envc]) {printf("Argument envp[%d] is %s\n", envc, envp[envc]); envc++;}

  char *argv_area[argc], *envp_area[envc];
  char *string_area = (char *)page;
  //char *string_area = (char *)heap.end;

  for (int i = 0; i < argc; i++){
    string_area -= strlen(argv[i]) + 1;
    argv_area[i] = string_area;
    strcpy(string_area, argv[i]);
  }

  for (int i = 0; i < envc; i++){
    string_area -= strlen(envp[i]) + 1;
    envp_area[i] = string_area;
    strcpy(string_area, envp[i]);
  }

  intptr_t *ptr = (intptr_t *)string_area;

  ptr -= 40;  // Unspecified

  *ptr = (intptr_t)NULL; ptr--;
  for(int i = envc - 1; i >= 0; i--){
    *ptr = (intptr_t)envp_area[i];
    ptr--;
  }
  *ptr = (intptr_t)NULL; ptr--;
  for(int i = argc - 1; i >= 0; i--){
    *ptr = (intptr_t)argv_area[i];
    ptr--;
  }
  *ptr = argc;

  void *entry = (void *)loader(pcb, filename);
  Area kstack;
  kstack.start = &pcb->cp;
  kstack.end = &pcb->cp + STACK_SIZE;
  Context *context = ucontext(as, kstack, entry);
  pcb->cp = context;

  context->GPRx = (intptr_t)ptr;
}

