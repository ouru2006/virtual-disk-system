#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "help.h"





void print_info(struct superblock info, struct FAT_info_t FAT_info){
  printf("Super block information: \n" );
  printf("Block size: %d\n",info.Block_size);
  printf("Block count: %d\n",info.Block_count);
  printf("FAT starts: %d\n",info.FAT_starts);
  printf("FAT blocks: %d\n",info.FAT_blocks );
  printf("Root directory start: %d\n",info.Root_start);
  printf("Root directory blocks: %d\n",info.Root_blocks);
  printf("\n");

  printf("FAT information:\n");
  printf("Free Blocks: %d\n",FAT_info.Free_b );
  printf("Reserved Blocks: %d\n",FAT_info.Res_b);
  printf("Allocated Blocks: %d\n",FAT_info.Alloc_b);

}
int main(int argc, char* argv[])
{
    if (argc < 2){
      printf("Usage: diskinfo filesyetem\n");
      exit(EXIT_FAILURE);
    }

    int fd;
    struct stat sf;
    unsigned char *data;
    struct superblock info;
    struct FAT_info_t FAT_info;
    initial_superblock(&info,&FAT_info);
    if ((fd = open(argv[1], O_RDONLY))){
        fstat(fd, &sf);
        data = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);
        read_diskinfo(data, &info, &FAT_info);
        print_info(info,FAT_info);
    } else {
        printf("Failed to open file '%s'\n", argv[1]);
    }

}
