#include <proc.h>
#include <elf.h>
#include <fs.h>

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

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){
  void *entry = (void *)loader(pcb, filename);
  Area kstack;
  kstack.start = &pcb->cp;
  kstack.end = &pcb->cp + STACK_SIZE;
  Context *context = ucontext(&pcb->as, kstack, entry);
  pcb->cp = context;
  char *brk = (char *)heap.end;
  *brk = 0;
  context->GPRx = (uintptr_t)heap.end - 40;
}

