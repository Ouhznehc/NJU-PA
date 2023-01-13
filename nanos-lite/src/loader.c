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
    //! we assume that different segment would not in the same page
    uint32_t memsz_nr_page = (phdr[i].p_vaddr + phdr[i].p_memsz - 1) / PGSIZE;
    uint32_t vaddr_nr_page = phdr[i].p_vaddr / PGSIZE;
    int nr_page = memsz_nr_page - vaddr_nr_page + 1;
    void *page = new_page(nr_page);
    void *vaddr = (void *)ROUNDDOWN(phdr[i].p_vaddr, PGSIZE);
    uint32_t page_offset = phdr[i].p_vaddr & (PGSIZE - 1);
    memset(page, 0, nr_page * PGSIZE);
    for(int j = 0; j < nr_page; j++) {
      map(&pcb->as, vaddr + j * PGSIZE, page + j * PGSIZE, MMAP_READ | MMAP_WRITE);
      //printf("loader map from va = %08p to pa = %08p\n", (void *)phdr[i].p_vaddr + j * PGSIZE ,page + j * PGSIZE);
    }
    fs_read (fd, page + page_offset, phdr[i].p_filesz);
    pcb->max_brk = MAX(phdr[i].p_vaddr + phdr[i].p_memsz, pcb->max_brk);
  }
  //pcb->max_brk = 0xe0000000;
  //printf("max_brk initial value is %08p\n", pcb->max_brk);
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
//     map(as, as->area.end - i * PGSIZE, page - i * PGSIZE, MMAP_READ | MMAP_WRITE);
//     //printf("context_uload map from va = %08p to pa = %08p\n", as->area.end - i * PGSIZE ,page - i * PGSIZE, MMAP_READ);
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
//   context->GPRx = (uintptr_t)ptr - (uintptr_t)page + (uintptr_t)as->area.end;
// }

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){
  int envc = 0, argc = 0;
  AddrSpace *as = &pcb->as;
  protect(as);
  
  if (envp){
    for (; envp[envc]; ++envc){}
  }
  if (argv){
    for (; argv[argc]; ++argc){}
  }
  char *envp_ustack[envc];

  void *alloced_page = new_page(NR_PAGE) + NR_PAGE * 4096; //得到栈顶

  //这段代码有古怪，一动就会出问题，莫动
  //这个问题确实已经被修正了，TMD，真cao dan
  // 2021/12/16
  
  map(as, as->area.end - 8 * PGSIZE, alloced_page - 8 * PGSIZE, 1); 
  map(as, as->area.end - 7 * PGSIZE, alloced_page - 7 * PGSIZE, 1);
  map(as, as->area.end - 6 * PGSIZE, alloced_page - 6 * PGSIZE, 1); 
  map(as, as->area.end - 5 * PGSIZE, alloced_page - 5 * PGSIZE, 1);
  map(as, as->area.end - 4 * PGSIZE, alloced_page - 4 * PGSIZE, 1); 
  map(as, as->area.end - 3 * PGSIZE, alloced_page - 3 * PGSIZE, 1);
  map(as, as->area.end - 2 * PGSIZE, alloced_page - 2 * PGSIZE, 1); 
  map(as, as->area.end - 1 * PGSIZE, alloced_page - 1 * PGSIZE, 1); 
  
  char *brk = (char *)(alloced_page - 4);
  // 拷贝字符区
  for (int i = 0; i < envc; ++i){
    brk -= strlen(envp[i]) + 1; // 分配大小
    envp_ustack[i] = brk;
    strcpy(brk, envp[i]);
  }

  char *argv_ustack[envc];
  for (int i = 0; i < argc; ++i){
    brk -= strlen(argv[i]) + 1; // 分配大小
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
  ptr_brk = ptr_brk - argc;
  
  // printf("%p\n", ptr_brk);
  printf("%p\t%p\n", alloced_page, ptr_brk);
  //printf("%x\n", ptr_brk);
  //assert((intptr_t)ptr_brk == 0xDD5FDC);
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

  Context* context = ucontext(as, karea, (void *)entry);
  pcb->cp = context;

  printf("新分配ptr=%p\n", as->ptr);
  printf("UContext Allocted at %p\n", context);
  printf("Alloced Page Addr: %p\t PTR_BRK_ADDR: %p\n", alloced_page, ptr_brk);

  ptr_brk -= 1;
  *ptr_brk = 0;//为了t0_buffer
  //设置了sp
  context->gpr[2]  = (uintptr_t)ptr_brk - (uintptr_t)alloced_page + (uintptr_t)as->area.end;

  //似乎不需要这个了，但我还不想动
  context->GPRx = (uintptr_t)ptr_brk - (uintptr_t)alloced_page + (uintptr_t)as->area.end + 4;
  //context->GPRx = (intptr_t)(ptr_brk);
}

