#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "list.h"
#include <stdbool.h>
#define ONEBLOCK 1
#define NEXTENTRY 1
#define FILENAME 1
#define DIRECTORY 2
#define NEXT_FAT_ENTRY 3
#define BLOCK_WIDTH 512
#define RT_OFFSET 9728
#define ATTRIBUTE_DIR 0x10
#define ATTRIBUTE_FILE 0x20
#define ALLOCATE_SIZE 100
#define FAT_LEN 3072
#define ROOT_LEN 6656
#define LINE_WIDTH 32
#define EXT_LOC 8
#define ATT_LOC 11
#define FLC_LOC 26
#define FILE_SZ_LOC 28
#define DEL_CHAR 0xE5
#define CP_8_BYTES 8
#define CP_3_BYTES 3
#define CP_1_BYTE 1
#define CP_2_BYTES 2
#define CP_4_BYTES 4
#define NEXTNIBBLE 2
#define SECTOR_LOC 31
#define END_FILE 0xfff
#define DEL_FILE 0x00
/*
Function: find_data
Inputs: Fat table, logical cluster
Purpose: Find the correct data entry for the given logical cluster
translate into 0xf60 and 0x05f
*/
int total_files = 0;
int find_data(unsigned int * table,unsigned short int entry){
     return BLOCK_WIDTH * (SECTOR_LOC + entry);
}
/*
Function: outputdata
Inputs: Output directory name, Data ptr with all the attributes of the file, the image pointer, FAT table, logical cluster
Outputs: None is void
Purpose: outputs the data to the assigned directory as such fileX.Y where X is total count and Y is the extention
*/
void outputdata(char * directory,DataPtr data, char* ptr, unsigned int * table, unsigned short int entry){
    char * out_file = malloc(sizeof(char) * ALLOCATE_SIZE);
    char * file_out_path = malloc(sizeof(char) * ALLOCATE_SIZE);
    strcpy(file_out_path,directory);
    sprintf(out_file,"file%d.%s",total_files,data->extension);
    strcat(file_out_path,out_file);
    FILE * data_out = fopen(file_out_path,"w+");
    if(data->file_size == 0 && *data->attributes != ATTRIBUTE_DIR){
        //empty fille, close file and escape!
        total_files++;
        fclose(data_out);
        return;
    }
    int loc = find_data(table,entry);
    int byte_limit = data->file_size;
    int i = 0;
    if(data->filename[0] != '_'){
        //first logical cluster
        for(i = 0; i < BLOCK_WIDTH; i++){
            if(byte_limit != 0){
            fprintf(data_out,"%c",ptr[loc + i]);
            byte_limit--;
            }
        }
         while (table[entry] != END_FILE){
             entry = table[entry];
             loc = find_data(table,entry);
            for(i = 0; i < BLOCK_WIDTH; i++){
                  if(byte_limit != 0){
                    fprintf(data_out,"%c",ptr[loc + i]);
                    byte_limit--;
                  }
                  else{
                      break;
                  }
               }
         }
    }
    else{
        //recover
        
            //keep going while table[entry + i] == 0x00!
            if(table[entry] != DEL_FILE){
                //close the file, nothing there content was overwritten
                total_files++;
                fclose(data_out);
                return;
            }
            int i = 0;
            for(i = 0; i < BLOCK_WIDTH; i++){
                if(byte_limit != 0){
                    fprintf(data_out,"%c",ptr[loc + i]);
                    byte_limit--;
                }
            }
            entry++;
           //loc+= 512;
           while(table[entry] == DEL_FILE){
                loc += BLOCK_WIDTH;
                entry++;
                for(i = 0; i < BLOCK_WIDTH; i++){
                    if(byte_limit != 0){
                        fprintf(data_out,"%c",ptr[loc + i]);
                        byte_limit--;
                    }
                }
           }
    }  
    total_files++;
    fclose(data_out);
    }
  
/*
Function: assign_root_attributes
Inputs: FAT table, image ptr, directory offset, current directory path, directory name
Outputs: none
Purpose: The function will go throught the root directory and parse its entries. 
        If there is another directory it will recurcivly obtain the new file path and continue on
        Function will also call outputData to output the data as it is being read
*/
void assign_root_attributes(unsigned int *fat, char * ptr, int DIR_OFFSET,char * path, char *dir){
    DataPtr elem_ptr = (DataPtr)calloc(sizeof(data_t),ONEBLOCK);
    int i, rover,stop;
    if(DIR_OFFSET == RT_OFFSET){
        stop = ROOT_LEN;
    }
    else{
        //New directories have length 512 instead of 6656
        stop = BLOCK_WIDTH;
    }
    for(i = 0; i < stop;i += LINE_WIDTH){
        rover = 0;
        memcpy(elem_ptr->filename,&ptr[i + DIR_OFFSET + rover],CP_8_BYTES);
        if(*elem_ptr->filename == '.' || strcmp((const char *)elem_ptr->filename,"..")== 0 || strcmp((const char *)elem_ptr->filename,"") == 0){
            //skip if invalid filename
            continue;
        }
        //Remove whitespace from filename
        for(int a = 0; a < 9;a++){
            if(elem_ptr->filename[a] == ATTRIBUTE_FILE || a == strlen((const char *)elem_ptr->filename)){
                elem_ptr->filename[a] = '\0';
                break;
            }
            
        }
        //replace with _
        if(elem_ptr->filename[0] == DEL_CHAR){
            elem_ptr->filename[0] = '_';
            elem_ptr->deleted = true;
        }
        else{
            elem_ptr->deleted = false;
        }
        //change rover for every new part of the directory table
        rover = EXT_LOC;
        memcpy(elem_ptr->extension,&ptr[i + DIR_OFFSET + rover],CP_3_BYTES);
        rover = ATT_LOC;
        memcpy(elem_ptr->attributes,&ptr[i + DIR_OFFSET + rover],CP_1_BYTE);
        rover = FLC_LOC;
        memcpy(&elem_ptr->first_clust,&ptr[i + DIR_OFFSET + rover],CP_2_BYTES);
        rover = FILE_SZ_LOC;
        memcpy(&elem_ptr->file_size,&ptr[i + DIR_OFFSET + rover],CP_4_BYTES);
        if (*elem_ptr->attributes == ATTRIBUTE_DIR){
            //its a directory, restart the function
            int new_dir_offset = find_data(fat,elem_ptr->first_clust);
            char * new_path = calloc(sizeof(char), ALLOCATE_SIZE);
            strcpy(new_path,path);
            strcat(new_path,(const char *)elem_ptr->filename);
            int size = strlen(new_path);
            new_path[size] = '/';
            assign_root_attributes(fat, ptr, new_dir_offset,new_path,dir);
        }
        else{
            if(elem_ptr->deleted){
                outputdata(dir,elem_ptr,ptr,fat, elem_ptr->first_clust);
                fprintf(stdout,"FILE\tDELETED\t%s%s.%s\t%ld\n",path,elem_ptr->filename,elem_ptr->extension,elem_ptr->file_size);
            }
            else{
                outputdata(dir,elem_ptr,ptr,fat, elem_ptr->first_clust);
                fprintf(stdout,"FILE\tNORMAL\t%s%s.%s\t%ld\n",path,elem_ptr->filename,elem_ptr->extension,elem_ptr->file_size);
            }
        }
        
    }
}
/*
Function: decodeFAT
Inputs: image_ptr
Ouptut: ret_ptr [fat decoded]
Purpose: Decode the fat with the bit shifting and masking methods presented like this:
       original ab cd ef --> fat 0xdab 0xefc
*/
unsigned int * decodeFAT(char * ptr){
    unsigned int * ret_ptr;
    ret_ptr = malloc(FAT_LEN * sizeof(int));
    unsigned int tempA = 0;
    unsigned int  tempB = 0;
    int j = 0;
    //j is here becasue every new entry is 3 bytes over
    for(int i = 0; i < FAT_LEN; i += NEXTNIBBLE){
        //Mask off the inner and outer nibble
        tempA = ptr[BLOCK_WIDTH + j + NEXTENTRY] & 0x0F;
        tempB = ptr[BLOCK_WIDTH + j + NEXTENTRY] & 0xF0;
        //Shift tempB one byte over to the left so that it can be maksed correctly: b4 0xf; after 0x0f
        tempB = tempB >> 4;
        //Shift tempA 2 bytest to the left to make maksing easier below: b4 0xf; after 0xf00
        tempA = tempA << 8;
        //obtain the correct nibble, mask off everything except the last 2 bytes then OR it with the shifted over 2 byte tempA
        ret_ptr[i] = (ptr[j + BLOCK_WIDTH] & 0x000000ff )| tempA;
        //Mask off the same amout but shift the result over by one byte so that tempB can be easily OR with it
        ret_ptr[i + NEXTENTRY] = ((ptr[BLOCK_WIDTH + j + NEXTNIBBLE] & 0x000000ff) << 4) | tempB;
        j += NEXT_FAT_ENTRY;
    }
return ret_ptr;
}


int main(int argc, char** argv){
    char * directory_name = malloc(sizeof(char) * ALLOCATE_SIZE);
    strcpy(directory_name,argv[DIRECTORY]);
    strcat(directory_name,"/");
	int fd = open(argv[FILENAME],O_RDWR, S_IRUSR | S_IWUSR);
	struct stat sb;
    fstat(fd,&sb);
	char *filemappedpage = mmap(0, sb.st_size,PROT_READ,MAP_SHARED, fd, 0);
    unsigned int * fat_table = decodeFAT(filemappedpage);
    char * file_path = malloc(sizeof(char) * ALLOCATE_SIZE);
    strcpy(file_path,"/");
    assign_root_attributes(fat_table, filemappedpage,RT_OFFSET,file_path,directory_name);
	munmap(filemappedpage, sb.st_size);
	close(fd);
}
