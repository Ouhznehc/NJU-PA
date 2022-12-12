// #include <proc.h>
// #include <elf.h>
// extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
// #ifdef __LP64__
// # define Elf_Ehdr Elf64_Ehdr
// # define Elf_Phdr Elf64_Phdr
// #else
// # define Elf_Ehdr Elf32_Ehdr
// # define Elf_Phdr Elf32_Phdr
// #endif

// static uintptr_t loader(PCB *pcb, const char *filename) {
//   //TODO();
//   //ramdisk_read(0x830003fc, )
//   return 0;
// }

// void naive_uload(PCB *pcb, const char *filename) {
//   uintptr_t entry = loader(pcb, filename);
//   Log("Jump to entry = %p", entry);
//   ((void(*)())entry) ();
// }

#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr elf_ehdr;
  int elf = fs_open(filename, 0, 0);
  fs_read(elf, &elf_ehdr, sizeof(elf_ehdr));
  assert(*(uint32_t *)elf_ehdr.e_ident == 0x464C457F);
  #if defined(__ISA_AM_NATIVE__)
  # define EXPECT_TYPE EM_X86_64
  #elif defined(__ISA_X86__)
  # define EXPECT_TYPE EM_X86_64
  #elif defined(__ISA_MIPS32__)
  # define EXPECT_TYPE EF_MIPS_ARCH_32
  #elif defined(__ISA_RISCV32__) || defined(__ISA_RISCV64__)
  # define EXPECT_TYPE EM_RISCV
  #elif
  # error unsupported ISA __ISA__
  #endif

  if(elf_ehdr.e_machine != EXPECT_TYPE){
    printf("ISA type error!\n");
    assert(0);
  }

  size_t ph_offest = elf_ehdr.e_phoff;
  for(int i =  0; i < elf_ehdr.e_phnum; i++){
    Elf_Phdr elf_phdr;
    fs_lseek(elf, ph_offest + i * elf_ehdr.e_phentsize, SEEK_SET);
    fs_read(elf, &elf_phdr, sizeof(elf_phdr));
    if(elf_phdr.p_type == PT_LOAD){
      fs_lseek(elf, elf_phdr.p_offset, SEEK_SET);
      fs_read(elf, (void *)elf_phdr.p_vaddr, elf_phdr.p_filesz);
      memset((void *)elf_phdr.p_vaddr + elf_phdr.p_filesz, 0, elf_phdr.p_memsz - elf_phdr.p_filesz);
    }
  }
  return elf_ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}


