struct  __attribute__((__packed__)) superblock {
    uint8_t identifier[8];
    uint16_t Block_size;
    uint32_t Block_count;
    uint32_t FAT_starts;
    uint32_t FAT_blocks;
    uint32_t Root_start;
    uint32_t Root_blocks;
};
struct __attribute__((__packed__)) dir_entry_timedate_t {
  uint16_t year;
  uint8_t  month;
  uint8_t  day;
  uint8_t  hour;
  uint8_t  minute;
  uint8_t  second;
};

struct __attribute__((__packed__)) dir_entry_t {
  uint8_t                       status;
  uint32_t                      starting_block;
  uint32_t                      block_count;
  uint32_t                      size;
  struct   dir_entry_timedate_t modify_time;
  struct   dir_entry_timedate_t create_time;
  uint8_t                       filename[31];
  uint8_t                       unused[6];
};

struct  __attribute__((__packed__)) FAT_info_t{
    uint32_t Free_b;
    uint32_t Res_b;
    uint32_t Alloc_b;
};

void initial_superblock(struct superblock *info, struct FAT_info_t *FAT_info){
    //strcpyinfo->identifier, "CSC360FS" );
    info->Block_size=0x200;
    info->Block_count=0x00001400;
    info->FAT_starts=0x00000001;
    info->FAT_blocks=0x00000028;
    info->Root_start=0x00000029;
    info->Root_blocks=0x00000008;

    FAT_info->Free_b=0;
    FAT_info->Res_b=0;
    FAT_info->Alloc_b=0;

}

int copy_data(unsigned char *data,int start, int length){

    int buffer;
    memcpy(&buffer, data+start, length);
    /*
     int hex_int=0;
     int i;

     for(i=0;i<length;i++){
     //printf("buffer:  %d\n",buffer[i]);
     hex_int=(hex_int)*256+buffer[i];
     }*/
    if(length==1)
        return buffer;
    else if(length==2)
        return htons(buffer);
    else
        return htonl(buffer);
}
void read_diskinfo(unsigned char *data,struct superblock *info, struct FAT_info_t *FAT_info){
    //strcpy( info->identifier, "CSC360FS" );
    info->Block_size=copy_data(data,8, 2);
    info->Block_count=copy_data(data,10, 4);
    info->FAT_starts=copy_data(data,14, 4);
    info->FAT_blocks=copy_data(data,18, 4);
    info->Root_start=copy_data(data,22, 4);
    info->Root_blocks=copy_data(data,26, 4);

    int i;

    for(i=0;i<info->Block_count;i++){
        int value =copy_data(data,info->Block_size*info->FAT_starts+i*4, 4);
        if(value==0)
            FAT_info->Free_b++;
        else if(value ==1)
            FAT_info->Res_b++;
        else if((value>1&&value<=0xffffff00) || value==0xffffffff)
            FAT_info->Alloc_b++;

    }

}
void read_entry(unsigned char *data,struct dir_entry_t *entry,int Root_s){
    entry->status=copy_data(data,Root_s, 1);
    entry->starting_block=copy_data(data,Root_s+1, 4);
    entry->block_count=copy_data(data,Root_s+5, 4);
    entry->size=copy_data(data, Root_s+9, 4);

    entry->create_time.year=copy_data(data, Root_s+13, 2);
    entry->create_time.month=copy_data(data, Root_s+15, 1);
    entry->create_time.day=copy_data(data, Root_s+16, 1);
    entry->create_time.hour=copy_data(data, Root_s+17, 1);
    entry->create_time.minute=copy_data(data, Root_s+18, 1);
    entry->create_time.second=copy_data(data, Root_s+19, 1);

    //entry->modify_time.year=copy_data(data, Root_s+13, 2);

    entry->modify_time.year=copy_data(data, Root_s+20, 2);
    entry->modify_time.month=copy_data(data, Root_s+22, 1);
    entry->modify_time.day=copy_data(data, Root_s+23, 1);
    entry->modify_time.hour=copy_data(data, Root_s+24, 1);
    entry->modify_time.minute=copy_data(data, Root_s+25, 1);
    entry->modify_time.second=copy_data(data, Root_s+26, 1);
    //entry->filename[0]=copy_data(data, Root_s+27, 1);
    memcpy(entry->filename,data+ Root_s+27,31);

    memcpy(entry->unused,data+ Root_s+58,6);


}
int find_dir(char *filename,unsigned char *data, int Root_blocks,int next_root){
  struct dir_entry_t entry[Root_blocks];
  int i;
  for(i=0;i<Root_blocks;i++){
      read_entry(data,&entry[i],next_root);
      if(entry[i].status!=0&&strcmp((char *)entry[i].filename,filename)==0){
          //print_list(entry[i]);
          return entry[i].starting_block;
          break;
        }
      next_root+=64;
  }
  return 0;

}
