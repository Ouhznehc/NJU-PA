// #include <fs.h>

// typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
// typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

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

// /* This is the information about all files in disk. */
// static Finfo file_table[] __attribute__((used)) = {
//   [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
//   [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
//   [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
//   [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write},
// #include "files.h"
// };

// void init_fs() {
//   // TODO: initialize the size of /dev/fb
// }

// int file_table_size(){return sizeof(file_table) / sizeof(Finfo);}

// int fs_open(const char *pathname, int flags, int mode){
//   for(int i = 0; i < file_table_size(); i++)
//     if(strcmp(file_table[i].name, pathname) == 0){
//       file_table[i].open_offset = 0;
//       return i;
//     }
//   panic("invalid filename!");
// }

// size_t fs_read(int fd, void *buf, size_t len){
//   Finfo *file = &file_table[fd];
//   if(len > file->size - file->open_offset) len = file->size - file->open_offset;
//   if(file->read) file->read(buf, file->disk_offset + file->open_offset, len);
//   else ramdisk_read(buf, file->disk_offset + file->open_offset, len);
//   file->open_offset += len;
//   return len;
// }

// size_t fs_write(int fd, const void *buf, size_t len){
//   Finfo *file = &file_table[fd];
//   if(len > file->size - file->open_offset) len = file->size - file->open_offset;
//   if(file->write) file->write(buf, file->disk_offset + file->open_offset, len);
//   else ramdisk_write(buf, file->disk_offset + file->open_offset, len);
//   file->open_offset += len;
//   return len;
// }

// size_t fs_lseek(int fd, size_t offset, int whence){
//   Finfo *file = &file_table[fd];
//   switch (whence){
//     case SEEK_SET: file->open_offset = offset;                     break;
//     case SEEK_CUR: file->open_offset = file->open_offset + offset; break;
//     case SEEK_END: file->open_offset = file->size + offset;        break;
//     default: panic("invalid whence!");
//   }
//   if(file->open_offset > file->size) panic("open_offset out of bound!");
//   return file->open_offset;
// }

// int fs_close(int fd){
//   return 0;
// }

#include <fs.h>

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

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS, FD_FB, FD_DISP};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_EVENTS] = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write},
  [FD_DISP]   = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

void init_fs() {
  file_table[FD_FB].size = io_read(AM_GPU_CONFIG).width * io_read(AM_GPU_CONFIG).height * sizeof(uint32_t);
}

int fs_open(const char *pathname, int flags, int mode){
  for(int i = 0; i < sizeof(file_table) / sizeof(Finfo); i++)
    if(strcmp(file_table[i].name, pathname) == 0){
      file_table[i].open_offset = 0;
      if(!file_table[i].read)file_table[i].read = ramdisk_read;
      if(!file_table[i].write)file_table[i].write = ramdisk_write;
      return i;
    }
  printf("The file indicated by pathname was not found\n");
  assert(0);
}

size_t fs_read(int fd, void *buf, size_t len){
  size_t res = 0;
  Finfo *tmp = &file_table[fd];
  if(tmp->read){
    if(len > tmp->size - tmp->open_offset && (void *)tmp->read == (void *)ramdisk_read)
      len = tmp->size - tmp->open_offset;
    res = tmp->read(buf, tmp->disk_offset + tmp->open_offset, len);
    tmp->open_offset += res;
  }
  return res;
}

size_t fs_write(int fd, const void *buf, size_t len){
  size_t res = 0;
  Finfo *tmp = &file_table[fd];
  if(tmp->write){
    if(len > tmp->size - tmp->open_offset && (void *)tmp->write == (void *)ramdisk_write)
      len = tmp->size - tmp->open_offset;
    res = tmp->write(buf, tmp->disk_offset + tmp->open_offset, len);
    tmp->open_offset += res;
  }
  return res;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  Finfo *tmp = &file_table[fd];
  switch (whence){
    case SEEK_SET:
      tmp->open_offset = offset;
      break;
    case SEEK_CUR:
      tmp->open_offset = tmp->open_offset + offset;
      break;
    case SEEK_END:
      tmp->open_offset = tmp->size + offset;
      break;
    default:
      printf("fs_lseek with wrong whence = %d\n", whence);
      assert(0);
  }
  if(tmp->open_offset > tmp->size){
    printf("offest beyond the boundary\n");
    assert(0);
  }
  return tmp->open_offset;
}

int fs_close(int fd){
  return 0;
}