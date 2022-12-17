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

// static uintptr_t loader(PCB *pcb, const char *filename) {
//   //TODO();
//   Elf_Ehdr ehdr;
//   Elf_Phdr phdr;
//   int elf_id = fs_open(filename, 0, 0);
//   fs_read(elf_id, &ehdr, sizeof(ehdr));

//   assert(*(uint32_t *)ehdr.e_ident == 0x464C457F);// magic number must be elf

//   for(int i =  0; i < ehdr.e_phnum; i++){
//     fs_lseek(elf_id, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
//     fs_read(elf_id, &phdr, sizeof(phdr));
//     if(phdr.p_type == PT_LOAD){
//       fs_lseek(elf_id, phdr.p_offset, SEEK_SET);
//       fs_read(elf_id, (void*)phdr.p_vaddr, phdr.p_filesz);
//       memset((void*)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
//     }
//   }
  
//   return ehdr.e_entry;
// }

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  //ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  Elf_Phdr phdr[ehdr.e_phnum];
  fs_read(fd, phdr, ehdr.e_phnum * sizeof(Elf_Phdr));
  //ramdisk_read(phdr, ehdr.e_ehsize, ehdr.e_phnum * sizeof(Elf_Phdr));
  for(size_t i = 0; i < ehdr.e_phnum; i++){
    if(phdr[i].p_type == PT_LOAD){
      fs_lseek(fd, phdr[i].p_offset, SEEK_SET);
      fs_read(fd, (void *)phdr[i].p_vaddr, phdr[i].p_memsz);
      //ramdisk_read((void *)phdr[i].p_vaddr, phdr[i].p_offset, phdr[i].p_memsz);
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

