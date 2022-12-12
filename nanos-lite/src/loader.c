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
  int elf_id = fs_open(filename, 0, 0);
  fs_read(elf_id, &elf_header, sizeof(elf_header));

  assert(*(uint32_t *)elf_header.e_ident == 0x464C457F);// magic number must be elf

  size_t ph_offest = elf_header.e_phoff;

  for(int i =  0; i < elf_header.e_phnum; i++){
    Elf_Phdr elf_phdr;
    fs_lseek(elf_id, ph_offest + i * elf_header.e_phentsize, SEEK_SET);
    fs_read(elf_id, &elf_phdr, sizeof(elf_phdr));
    if(elf_phdr.p_type == PT_LOAD){
      fs_lseek(elf_id, elf_phdr.p_offset, SEEK_SET);
      fs_read(elf_id, (void *)elf_phdr.p_vaddr, elf_phdr.p_filesz);
      memset((void *)elf_phdr.p_vaddr + elf_phdr.p_filesz, 0, elf_phdr.p_memsz - elf_phdr.p_filesz);
    }
  }
  return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

