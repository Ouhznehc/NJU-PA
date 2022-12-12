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
  //TODO();
  Elf_Ehdr ehdr;
  Elf_Phdr phdr;
  int elf_id = fs_open(filename, 0, 0);
  fs_read(elf_id, &ehdr, sizeof(ehdr));

  assert(*(uint32_t *)ehdr.e_ident == 0x464C457F);// magic number must be elf

  for(int i =  0; i < ehdr.e_phnum; i++){
    fs_lseek(elf_id, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
    fs_read(elf_id, &phdr, sizeof(phdr));
    if(phdr.p_type == PT_LOAD){
      fs_lseek(elf_id, phdr.p_offset, SEEK_SET);
      fs_read(elf_id, (void*)phdr.p_vaddr, phdr.p_filesz);
      memset((void*)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
    }
  }
  
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

