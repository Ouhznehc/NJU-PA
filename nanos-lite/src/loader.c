#include <proc.h>
#include <elf.h>
#include <fs.h>

#define NR_PAGE 8
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
// #ifdef __LP64__
// # define Elf_Ehdr Elf64_Ehdr
// # define Elf_Phdr Elf64_Phdr
// #else
// # define Elf_Ehdr Elf32_Ehdr
// # define Elf_Phdr Elf32_Phdr
// #endif

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define Elf_Half Elf64_Half
# define Elf_Off Elf64_Off
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define Elf_Half Elf32_Half
# define Elf_Off Elf32_Off
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {

  int target_fd = fs_open(filename, 0, 0);
  if (target_fd == -1) {
    return (uintptr_t) yield;
  }
  Elf_Ehdr header;
  fs_read(target_fd, (void *) &header, sizeof(Elf_Ehdr));
//  fs_lseek(target_fd, 0, SEEK_SET);
  if (*((uint32_t *) header.e_ident) == 0x464C457f) {
    //Log("Loading %s as an ELF file...", filename);
    Elf_Half phnum = header.e_phnum;
    Elf_Off phoff = header.e_phoff;
    while (phnum--) {
      Elf_Phdr segment;
      fs_lseek(target_fd, phoff, SEEK_SET);
      fs_read(target_fd, (void *) &segment, header.e_phentsize);
      phoff += header.e_phentsize;
      //Log("Segment at %p with memory size 0x%06x", segment.p_vaddr, segment.p_memsz);
      if (!(segment.p_type == PT_LOAD)) {
        //Log("Not a PT_LOAD segment, ignoring...");
        continue;
      }
      
      fs_lseek(target_fd, segment.p_offset, SEEK_SET);

      for (int i = 0; i < CEIL(segment.p_memsz, PGSIZE); i++) {
        void *n_pgpa = new_page(1);
        map(&pcb->as, (void *) segment.p_vaddr + i * PGSIZE, n_pgpa, MMAP_READ | MMAP_WRITE);
        memset((void *) n_pgpa, 0, MIN(PGSIZE, segment.p_memsz - PGSIZE * i));
        fs_read(target_fd, (void *) n_pgpa, MIN(PGSIZE, segment.p_filesz - PGSIZE * i));
      }
      pcb->max_brk = MAX(pcb->max_brk, segment.p_vaddr + segment.p_memsz);
      // memset((void *) segment.p_vaddr, 0, segment.p_memsz);
      // fs_read(target_fd, (void *) segment.p_vaddr, segment.p_filesz);
    }
    //Log("Program break starts at %p", pcb->max_brk);
    fs_close(target_fd);
    return (uintptr_t) header.e_entry;
  }
  else {
    //Err("%s is not an ELF file! Ignoring...", filename);
  }
  //something not good happens, terminate the program
  return (uintptr_t) yield;
}

// static uintptr_t loader(PCB *pcb, const char *filename) {
//   Elf_Ehdr ehdr;
//   int fd = fs_open(filename, 0, 0);
//   if(fd == -1) panic("Invalid filename!");
//   fs_read(fd, &ehdr, sizeof(Elf_Ehdr));

//   assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);// magic number must be elf

//   Elf_Phdr phdr[ehdr.e_phnum];
//   fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
//   fs_read (fd, phdr, ehdr.e_phnum * sizeof(Elf_Phdr));
//   for(size_t i = 0; i < ehdr.e_phnum; i++){
//     if(phdr[i].p_type != PT_LOAD) continue;
//     fs_lseek(fd, phdr[i].p_offset, SEEK_SET);
//     int nr_page = ROUNDUP(phdr[i].p_memsz, PGSIZE) / PGSIZE;
//     void *page = new_page(nr_page);
//     memset(page, 0, nr_page * PGSIZE);
//     for(int j = 0; j < nr_page; j++) {
//       map(&pcb->as, (void *)phdr[i].p_vaddr + j * PGSIZE, page + j * PGSIZE, MMAP_READ | MMAP_WRITE);
//       printf("loader map from va = %08p to pa = %08p\n", (void *)phdr[i].p_vaddr + j * PGSIZE ,page + j * PGSIZE);
//     }
//     fs_read (fd, page, phdr[i].p_memsz);
//     pcb->max_brk = MAX(phdr[i].p_vaddr + phdr[i].p_memsz, pcb->max_brk);
//   }
//   //pcb->max_brk = 0xe0000000;
//   printf("max_brk initial value is %08p\n", pcb->max_brk);
//   return ehdr.e_entry;
// }

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
  void *page = new_page(NR_PAGE) + NR_PAGE * PGSIZE;
  for(int i = NR_PAGE; i >= 1; i--) {
  map(as, as->area.end - i * PGSIZE, page - i * PGSIZE, MMAP_READ | MMAP_WRITE);
  printf("context_uload map from va = %08p to pa = %08p\n", as->area.end - i * PGSIZE ,page - i * PGSIZE, MMAP_READ);
  }
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

