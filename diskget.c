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







void copy_file(char * fname,unsigned char *data,struct dir_entry_t entry, uint32_t FAT_table[],int Block_size){
    //printf("%c %10d %30s %10d %10d\n", entry.status==3?'F':'D',entry.size, entry.filename, entry.starting_block,entry.block_count);

    unsigned char buffer[entry.size];
    int next=entry.starting_block;
    int rem_size=entry.size;
    int copyed=0;
    int copy_size=0;
    do{
        copy_size=Block_size<rem_size? Block_size:rem_size;
        //printf("%d %d\n",rem_size,copy_size);
        memcpy(&buffer[copyed],data+next*Block_size,copy_size);
        copyed+=copy_size;
        rem_size-=copy_size;
    //printf("%s\n",buffer);
        next=FAT_table[next];
    }while(next!=0xffffffff);
    //printf("%s %d\n",buffer,copy_size);
    FILE *fp;

    fp = fopen( fname , "w" );
    fwrite(buffer , 1 , sizeof(buffer) , fp );

    fclose(fp);

}
int main(int argc, char* argv[])
{
    if (argc < 4){
      printf("Usage: disklist disk filename local_filename\n");
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
        int i;
        uint32_t FAT_table[info.Block_count];
        for(i=0;i<info.Block_count;i++){
            FAT_table[i] =copy_data(data,info.Block_size*info.FAT_starts+i*4, 4);
        }
        struct dir_entry_t entry[info.Root_blocks];
        int next_root = info.Root_start*info.Block_size;


        char *token;
        char filename[31]="";
        //char ="";
        /* get the first token */
        token = strtok(argv[2], "/");

        while( token != NULL ) {
          //printf( "%s\n", token );
          //strcat(filename,NULL);
          //filename[0]="";
          strcpy(&filename[0],token);
          token = strtok(NULL, "/");
          if(token != NULL){
            //printf( "%s\n", filename );
            int next_start=find_dir(filename,data,info.Root_blocks,next_root);

            if(next_start==0){
              printf("Cannot find the directory '%s'\n", filename);
              exit(0);
            }else{
              next_root=next_start*info.Block_size;
            }
          }
        }

        for(i=0;i<info.Root_blocks;i++){

            read_entry(data,&entry[i],next_root);
            if(entry[i].status!=0&&strcmp((char *)entry[i].filename,filename)==0){
                copy_file(argv[3],data,entry[i],FAT_table,info.Block_size);
                break;
            }
            next_root+=64;
        }
        if(i>=info.Root_blocks) printf("File not found \n");
        //();
    } else {
        printf("Failed to open file '%s'\n", argv[1]);
    }

}
