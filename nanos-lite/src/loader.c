// #include <proc.h>
// #include <elf.h>
// #include <fs.h>

// #ifdef __LP64__
// # define Elf_Ehdr Elf64_Ehdr
// # define Elf_Phdr Elf64_Phdr
// # define Elf_Off Elf64_Off
// #else
// # define Elf_Ehdr Elf32_Ehdr
// # define Elf_Phdr Elf32_Phdr
// # define Elf_Off Elf32_Off
// #endif

// size_t ramdisk_read(void *buf, size_t offset, size_t len);
// size_t ramdisk_write(const void *buf, size_t offset, size_t len);
// size_t get_ramdisk_size();

// static uintptr_t loader(PCB *pcb, const char *filename) {
//   int fd = fs_open(filename, 0, 0);
//   Elf_Ehdr elf_header;
//   fs_lseek(fd, 0, SEEK_SET);
//   fs_read(fd, &elf_header, sizeof(elf_header));

//   //assert(*(uint32_t *)elf_header.e_ident == 0xBadC0de);

//   for (int i = 0; i < elf_header.e_phnum; i++){
//     Elf_Phdr section_entry;
//     fs_lseek(fd, i * elf_header.e_phentsize + elf_header.e_phoff, SEEK_SET);
//     fs_read(fd, &section_entry, sizeof(elf_header));
//     void * virt_addr;
//     switch (section_entry.p_type) {
//       case PT_LOAD:
//         virt_addr = (void *)section_entry.p_vaddr;  
//         fs_lseek(fd, section_entry.p_offset, SEEK_SET);
//         fs_read(fd, virt_addr, section_entry.p_filesz);
//         memset(virt_addr + section_entry.p_filesz, 0, section_entry.p_memsz - section_entry.p_filesz);
//         break;
//       default:
//         break;
//     }
//   }
//   return elf_header.e_entry;
// }

// void naive_uload(PCB *pcb, const char *filename) {
//   uintptr_t entry = loader(pcb, filename);
//   Log("Jump to entry = %p", entry);
//   ((void(*)())entry) ();
// }

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

