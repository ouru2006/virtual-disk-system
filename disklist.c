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








void print_list(struct dir_entry_t entry){
    printf("%c %10d %30s %4d/%02d/%02d %2d:%02d:%02d\n", entry.status==3?'F':'D',entry.size, entry.filename, entry.modify_time.year,entry.modify_time.month,entry.modify_time.day, entry.modify_time.hour, entry.modify_time.minute, entry.modify_time.second);
}
int main(int argc, char* argv[])
{
    if (argc < 2){
      printf("Usage: disklist filesystem directory\n");
      exit(EXIT_FAILURE);
    }

    int fd;
    struct stat sf;
    unsigned char *data;
    struct superblock info;
    struct FAT_info_t FAT_info;
    //struct dir_entry_t entry;
    initial_superblock(&info,&FAT_info);
    if ((fd = open(argv[1], O_RDONLY))){
        fstat(fd, &sf);
        data = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);
        read_diskinfo(data, &info, &FAT_info);


        char *token;
        char filename[31];
        /* get the first token */
        token = strtok(argv[2], "/");

        /* walk through other tokens */
        int next_root = info.Root_start*info.Block_size;
        while( token != NULL ) {
          //printf( "%s\n", token );
          strcpy(&filename[0],token);
          //printf( "%s\n", filename );

          int next_start=find_dir(filename,data,info.Root_blocks,next_root);

          if(next_start==0){
            printf("Cannot find the directory '%s'\n", filename);
            exit(0);
          }else{
            next_root=next_start*info.Block_size;
          }

          token = strtok(NULL, "/");

        }

        struct dir_entry_t entry[info.Root_blocks];
        //int next_root = info.Root_start*info.Block_size;
        int i;
        for(i=0;i<info.Root_blocks;i++){

            read_entry(data,&entry[i],next_root);
            if(entry[i].status!=0)
                print_list(entry[i]);
            next_root+=64;
        }
    } else {
        printf("Failed to open file '%s'\n", argv[1]);
    }

}
