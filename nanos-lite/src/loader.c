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
  int elf_id = fs_open(filename, 0, 0);
  fs_read(elf_id, &ehdr, sizeof(Elf_Ehdr));

  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);// magic number must be elf

  Elf_Phdr phdr[ehdr.e_phnum];
  fs_read(elf_id, phdr, ehdr.e_phnum * sizeof(Elf_Phdr));
  for(size_t i = 0; i < ehdr.e_phnum; i++){
    if(phdr[i].p_type == PT_LOAD){
      fs_lseek(elf_id, phdr[i].p_offset, SEEK_SET);
      fs_read(elf_id, (void *)phdr[i].p_vaddr, phdr[i].p_memsz);
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

static size_t ceil_4_bytes(size_t byte){
  if(byte & 0x3) return (byte & (~0x3)) + 0x4;
  return byte;
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){

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
//   while(argv && argv[argc]) {printf("Argument argv[%d] = %s\n", argc, argv[argc]); argc++;}
//   while(envp && envp[envc]) {printf("Argument envp[%d] = %s\n", envc, envp[envc]); envc++;}

//   char *argv_area[argc], *envp_area[envc];
//   char *string_area = (char *)new_page(nr_page);
//   //char *string_area = (char *)heap.end;

//   for (int i = 0; i < argc; i++){
//     string_area -= rounded4(strlen(argv[i]) + 1);
//     argv_area[i] = string_area;
//     strcpy(string_area, argv[i]);
//   }

//   for (int i = 0; i < envc; i++){
//     string_area -= rounded4(strlen(envp[i]) + 1);
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
//   Context *context = ucontext(NULL, kstack, entry);
//   pcb->cp = context;
//   context->GPRx = (intptr_t)ptr;
  int envc = 0, argc = 0;
  if (envp){
    for (; envp[envc]; ++envc){}
  }
  if (argv){
    for (; argv[argc]; ++argc){}
  }

  char *envp_ustack[envc];
  char *brk = (char *)new_page(NR_PAGE);
  
  // 拷贝字符区
  for (int i = 0; i < envc; ++i){
    brk -= (ceil_4_bytes(strlen(envp[i]) + 1)); // 分配大小
    envp_ustack[i] = brk;
    strcpy(brk, envp[i]);
  }

  char *argv_ustack[envc];
  for (int i = 0; i < argc; ++i){
    brk -= (ceil_4_bytes(strlen(argv[i]) + 1)); // 分配大小
    argv_ustack[i] = brk;
    strcpy(brk, argv[i]);
  }
  
  intptr_t *ptr_brk = (intptr_t *)(brk);
  
  // 分配envp空间
  ptr_brk -= 1;
  *ptr_brk = 0;
  ptr_brk -= envc;
  for (int i = 0; i < envc; ++i){
    ptr_brk[i] = (intptr_t)(envp_ustack[i]);
  }

  // 分配argv空间
  ptr_brk -= 1;
  *ptr_brk = 0;
  ptr_brk -= argc;
  for (int i = 0; i < argc; ++i){
    ptr_brk[i] = (intptr_t)(argv_ustack[i]);
  }

  ptr_brk -= 1;
  *ptr_brk = argc;
  
  //这条操作会把参数的内存空间扬了，要放在最后
  uintptr_t entry = loader(pcb, filename);
  Area karea;
  karea.start = &pcb->cp;
  karea.end = &pcb->cp + STACK_SIZE;

  Context* context = ucontext(NULL, karea, (void *)entry);
  pcb->cp = context;

  context->GPRx = (intptr_t)ptr_brk;
}

