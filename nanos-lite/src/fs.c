// #include <fs.h>
// #include <am.h>

// typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
// typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

// size_t ramdisk_read(void *buf, size_t offset, size_t len);
// size_t ramdisk_write(const void *buf, size_t offset, size_t len);
// size_t serial_write(const void *buf, size_t offset, size_t len);

// typedef struct {
//   char *name;
//   size_t size;
//   size_t disk_offset;
//   ReadFn read;
//   WriteFn write;
//   size_t open_offset;
// } Finfo;

// enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

// size_t invalid_read(void *buf, size_t offset, size_t len) {
//   panic("should not reach here");
//   return 0;
// }

// size_t invalid_write(const void *buf, size_t offset, size_t len) {
//   panic("should not reach here");
//   return 0;
// }

// size_t fs_write(int fd, const void *buf, size_t len);
// /* This is the information about all files in disk. */
// static Finfo file_table[] __attribute__((used)) = {
//   [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
//   [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
//   [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
//   [FD_FB] = {"/dev/fb", 0, 0, invalid_read, invalid_write},
// #include "files.h"
// };

// void init_fs() {
//   // TODO: initialize the size of /dev/fb
// }

// int fs_open(const char *pathname, int flags, int mode){
//   for(int i = 0; i < sizeof(file_table)/sizeof(Finfo); i++){
//     if(strcmp(pathname, file_table[i].name) == 0){
//       file_table[i].open_offset = 0;
//       return i;
//     }
//   }
//   panic("No such file");
// }

// size_t fs_read(int fd, void *buf, size_t len){
//   Finfo *finfo = &file_table[fd];
//   size_t actual_len = 0;
//   if(finfo->read){
//     ;
//   }else{
//     if(finfo->open_offset + len <= finfo->size){
//       actual_len = len;
//     }else{
//       actual_len = finfo->size - finfo->open_offset;
//     }
//     ramdisk_read(buf, finfo->disk_offset + finfo->open_offset, actual_len);
//     finfo->open_offset += actual_len;
//   }
//   return actual_len;
// }

// size_t fs_write(int fd, const void *buf, size_t len){
//   Finfo *finfo = &file_table[fd];
//   size_t actual_len = 0;
//   if(finfo->write){
//     actual_len = finfo->write(buf, finfo->open_offset, len);
//     finfo->open_offset += actual_len;
//   }else{
//     if(finfo->open_offset + len > finfo->size){
//       panic("Wrong");
//     }
//     ramdisk_write(buf, finfo->disk_offset + finfo->open_offset, len);
//     actual_len = len;
//     finfo->open_offset += actual_len;
//   }
//   return actual_len;  
// }

// size_t fs_lseek(int fd, size_t offset, int whence){
//   Finfo *finfo = &file_table[fd];
//   switch (whence)
//   {
//   case SEEK_CUR:
//     if(finfo->open_offset + offset > finfo->size){
//       panic("Out of bound\n");
//     }
//     finfo->open_offset += offset;
//     break;
  
//   case SEEK_END:
//     if(offset > finfo->size){
//       panic("Out of bound\n");
//     }
//     finfo->open_offset = offset + finfo->size;
//     break;

//   case SEEK_SET:
//     if(offset > finfo->size){
//       panic("Out of bound\n");
//     }
//     finfo->open_offset = offset;
//     break;

//   default:
//     break;
//   }
//   return finfo->open_offset;
// }

// int fs_close(int fd){
//   file_table[fd].open_offset = 0;
//   return 0;
// }

#include <fs.h>
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int file_table_size(){return sizeof(file_table) / sizeof(Finfo);}

int fs_open(const char *pathname, int flags, int mode){
  for(int i = 0; i < file_table_size(); i++)
    if(strcmp(file_table[i].name, pathname) == 0){
      file_table[i].open_offset = 0;
      return i;
    }
  panic("invalid filename!");
}

size_t fs_read(int fd, void *buf, size_t len){
  Finfo *file = &file_table[fd];
  if(len > file->size - file->open_offset) len = file->size - file->open_offset;
  if(file->read) file->read(buf, file->disk_offset + file->open_offset, len);
  else ramdisk_read(buf, file->disk_offset + file->open_offset, len);
  file->open_offset += len;
  return len;
}

size_t fs_write(int fd, const void *buf, size_t len){
  Finfo *file = &file_table[fd];
  if(len > file->size - file->open_offset) len = file->size - file->open_offset;
  if(file->write) file->write(buf, file->disk_offset + file->open_offset, len);
  else ramdisk_write(buf, file->disk_offset + file->open_offset, len);
  file->open_offset += len;
  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  Finfo *file = &file_table[fd];
  switch (whence){
    case SEEK_SET: file->open_offset = offset;                     break;
    case SEEK_CUR: file->open_offset = file->open_offset + offset; break;
    case SEEK_END: file->open_offset = file->size + offset;        break;
    default: panic("invalid whence!");
  }
  if(file->open_offset > file->size) panic("open_offset out of bound!");
  return file->open_offset;
}

int fs_close(int fd){
  return 0;
}
