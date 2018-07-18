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







uint32_t find_nextFAT(unsigned char *data, struct superblock info,int next){
    int i;

    for(i=next+1;i<info.Block_count;i++){
        if(copy_data(data,info.Block_size*info.FAT_starts+i*4, 4)==0){
            break;
        }
    }
    if(i<info.Block_count)
        return i;
    else{
        printf("file system has no enough space.\n");
        exit(0);
        //return 0;
    }
}

void copy_file(unsigned char *buffer,unsigned char *data,struct dir_entry_t entry, struct superblock info){
    //printf("%c %10d %30s %10d %10d\n", entry.status==3?'F':'D',entry.size, entry.filename, entry.starting_block,entry.block_count);


    int next=htonl(entry.starting_block);
    int rem_size=htonl(entry.size);
    int copyed=0;
    int copy_size=0;
    do{
        copy_size=info.Block_size<rem_size? info.Block_size:rem_size;
        //printf("%d %d\n",rem_size,copy_size);
        memcpy(data+next*info.Block_size,&buffer[copyed],copy_size);
        //printf("%d %d %d\n",next,copyed,copy_size);
        copyed+=copy_size;
        rem_size-=copy_size;
        //printf("%s\n",buffer);

        if(rem_size>0){

            uint32_t temp=ntohl(find_nextFAT(data,info,next));
            memcpy(data+info.Block_size*info.FAT_starts+next*4,&temp,4);
            next=htonl(temp);
        }else{
            uint32_t temp=0xffffffff;
            memcpy(data+info.Block_size*info.FAT_starts+next*4,&temp,4);
        }
        //printf("%d %d\n",rem_size,copy_size);
    }while(rem_size>0);
    //printf("%s %d\n",buffer,copy_size);


}

int find_nextavadir(unsigned char *data, int Root_blocks,int next_root){
  struct dir_entry_t entry[Root_blocks];
  int i;
  for(i=0;i<Root_blocks;i++){
      read_entry(data,&entry[i],next_root);
      if(entry[i].status==0){
          //print_list(entry[i]);
          return next_root;
          break;
        }
      next_root+=64;
  }
  printf("directory have no space \n");
  exit(0);
}
int create_subdir(char *filename,unsigned char *data,struct superblock info,int Root_s){
  int entry_blocks=info.Root_blocks;
  int next=find_nextFAT(data,info,0);
  struct dir_entry_t new_dir;
  new_dir.status=0x05;
  new_dir.starting_block=ntohl(next);
  new_dir.block_count=ntohl(entry_blocks);
  new_dir.size=ntohl(info.Block_size*entry_blocks);

  time_t theTime = time(NULL);
  struct tm *aTime = localtime(&theTime);

  new_dir.modify_time.year=ntohs(aTime->tm_year + 1900);
  new_dir.modify_time.month=aTime->tm_mon + 1;
  new_dir.modify_time.day=aTime->tm_mday;
  new_dir.modify_time.hour=aTime->tm_hour;
  new_dir.modify_time.minute=aTime->tm_min;
  new_dir.modify_time.second=aTime->tm_sec;

  memcpy(new_dir.filename,filename,31);

  memcpy(data+Root_s,&new_dir.status, 1);
  memcpy(data+Root_s+1,&new_dir.starting_block, 4);
  memcpy(data+Root_s+5,&new_dir.block_count, 4);
  memcpy(data+Root_s+9,&new_dir.size,4);
  memcpy(data+Root_s+20,&new_dir.modify_time.year, 2);
  memcpy(data+Root_s+22,&new_dir.modify_time.month, 1);
  memcpy(data+Root_s+23,&new_dir.modify_time.day, 1);
  memcpy(data+Root_s+24,&new_dir.modify_time.hour, 1);
  memcpy(data+Root_s+25,&new_dir.modify_time.minute, 1);
  memcpy(data+Root_s+26,&new_dir.modify_time.second, 1);
  //entry->filename[0]=copy_data(data, Root_s+27, 1);
  memcpy(data+ Root_s+27,&new_dir.filename,31);


  int rem_size=info.Block_size*entry_blocks;
  //int copyed=0;
  //memcpy(entry->unused,data+ Root_s+58,6);
  do{
      //memcpy(data+next*Block_size,&buffer[copyed],copy_size);
      //printf("%d %d %d\n",next,copyed,copy_size);

      rem_size-=info.Block_size;
      //printf("%s\n",buffer);
      if(rem_size>0){

          uint32_t temp=ntohl(find_nextFAT(data,info,next));
          memcpy(data+info.Block_size*info.FAT_starts+next*4,&temp,4);

          next=htonl(temp);
      }else{
          uint32_t temp=0xffffffff;

          memcpy(data+info.Block_size*info.FAT_starts+next*4,&temp,4);
      }
      //printf("%d %d\n",rem_size,copy_size);
  }while(rem_size>0);
  return htonl(new_dir.starting_block)*info.Block_size;
}
unsigned char* read_file(char * filename,struct dir_entry_t* new_dir,uint32_t Block_count){
    FILE * pFile;
    long lSize;
    unsigned char * buffer;
    size_t result;

    pFile = fopen ( filename , "rb" );
    if (pFile==NULL) {
        printf("File not found\n");
        exit (1);

    }

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);

    // allocate memory to contain the whole file:
    buffer = (unsigned char*) malloc (sizeof(unsigned char)*lSize);
    if (buffer == NULL) {
        fputs ("Memory error",stderr);
        exit (2);}

    // copy the file into the buffer:
    result = fread (buffer,1,lSize,pFile);
    if (result != lSize) {fputs ("Reading error",stderr); exit (3);}
    //printf("%s,%zu\n", buffer,result);
    /* the whole file is now loaded in the memory buffer. */



    new_dir->status=0x03;
    //new_dir.starting_block=ntohl(startblock);
    new_dir->block_count=ntohl((result+(result-1))/Block_count);
    new_dir->size=ntohl(result);

    time_t theTime = time(NULL);
    struct tm *aTime = localtime(&theTime);

    /*
    int day = aTime->tm_mday;
    int month = aTime->tm_mon + 1; // Month is 0 - 11, add 1 to get a jan-dec 1-12 concept
    int year = aTime->tm_year + 1900; // Year is # years since 1900
    */
    new_dir->modify_time.year=ntohs(aTime->tm_year + 1900);
    new_dir->modify_time.month=aTime->tm_mon + 1;
    new_dir->modify_time.day=aTime->tm_mday;
    new_dir->modify_time.hour=aTime->tm_hour;
    new_dir->modify_time.minute=aTime->tm_min;
    new_dir->modify_time.second=aTime->tm_sec;

    //strcpy(&new_dir->filename,filename);


    //printf("%d/%d/%d",htons(a),aTime->tm_min,aTime->tm_sec);
    // terminate
    fclose (pFile);
    return buffer;

}
int main(int argc, char* argv[])
{
    if (argc < 4){
      printf("Usage: disklist disk local_filename filename\n");
      exit(EXIT_FAILURE);
    }

    int fd, fd_local;
    struct stat sf, sf_local;
    unsigned char *data, *data_local;
    struct superblock info;
    struct FAT_info_t FAT_info;
    int i;


    //struct dir_entry_t entry;
    initial_superblock(&info,&FAT_info);
    if ((fd = open(argv[1], O_RDWR))){
        fstat(fd, &sf);
        data = mmap(NULL,sf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        read_diskinfo(data, &info, &FAT_info);

        //char str[80] = "This is - www.tutorialspoint.com - website";
        //const char s[2] = "/";
        char *token;
        char filename[31]="";
        //char ="";
        /* get the first token */
        token = strtok(argv[3], "/");

        /* walk through other tokens */
        int next_root = info.Root_start*info.Block_size;
        while( token != NULL ) {
          //printf( "%s\n", token );
          //strcat(filename,NULL);
          //filename[0]="";
          strcpy(&filename[0],token);
          token = strtok(NULL, "/");
          if(token != NULL){
            //printf( "%s\n", filename );
            int next_start=find_dir(filename,data,info.Root_blocks,next_root);
            //printf( "aaaaa\n");
            if(next_start==0){
              next_root= find_nextavadir(data, info.Root_blocks,next_root);
              next_root=create_subdir(filename,data,info,next_root);
            }else{
              next_root=next_start*info.Block_size;
            }
          }
        }
        //printf( "%s\n", filename );

        struct dir_entry_t new_dir;
        data_local=read_file(argv[2],&new_dir,info.Block_count);
        new_dir.starting_block=ntohl(find_nextFAT(data,info,0));
        memcpy(new_dir.filename,filename,31);
        //printf("%d\n",ntohl(50));
        struct dir_entry_t entry[info.Root_blocks];

        //unsigned char temp[6]="hello";

        for(i=0;i<info.Root_blocks;i++){

            read_entry(data,&entry[i],next_root);
            if(entry[i].status==0){
                int Root_s=next_root;

                memcpy(data+next_root,&new_dir.starting_block,4);
                memcpy(data+Root_s,&new_dir.status, 1);
                memcpy(data+Root_s+1,&new_dir.starting_block, 4);
                memcpy(data+Root_s+5,&new_dir.block_count, 4);
                memcpy(data+Root_s+9,&new_dir.size,4);
                memcpy(data+Root_s+20,&new_dir.modify_time.year, 2);
                memcpy(data+Root_s+22,&new_dir.modify_time.month, 1);
                memcpy(data+Root_s+23,&new_dir.modify_time.day, 1);
                memcpy(data+Root_s+24,&new_dir.modify_time.hour, 1);
                memcpy(data+Root_s+25,&new_dir.modify_time.minute, 1);
                memcpy(data+Root_s+26,&new_dir.modify_time.second, 1);
                //entry->filename[0]=copy_data(data, Root_s+27, 1);
                memcpy(data+ Root_s+27,&new_dir.filename,31);

                //memcpy(entry->unused,data+ Root_s+58,6);

                copy_file(data_local,data,new_dir,info);
                //printf("%sn",data_local);
                free(data_local);
                break;
            }
            next_root+=64;

        }
        if(i>=info.Root_blocks) printf("root directory have no space \n");
        //();

    } else {
        printf("Failed to open file system '%s'\n", argv[1]);
    }

}
