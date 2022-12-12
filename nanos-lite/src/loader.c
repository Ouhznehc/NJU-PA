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
  Elf_Ehdr elf_header;
  Elf_Phdr program_header;
  int elf_id = fs_open(filename, 0, 0);
  fs_read(elf_id, &elf_header, sizeof(elf_header));

  assert(*(uint32_t *)elf_header.e_ident == 0x464C457F);// magic number must be elf

  for(int i =  0; i < elf_header.e_phnum; i++){
    fs_lseek(elf_id, elf_header.e_phoff + i * elf_header.e_phentsize, SEEK_SET);
    fs_read(elf_id, &program_header, sizeof(program_header));
    if(program_header.p_type == PT_LOAD){
      fs_lseek(elf_id, program_header.p_offset, SEEK_SET);
      fs_read(elf_id, (void *)program_header.p_vaddr, program_header.p_filesz);
      memset((void *)program_header.p_vaddr + program_header.p_filesz, 0, program_header.p_memsz - program_header.p_filesz);
    }
  }
  return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

