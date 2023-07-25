/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * Copyright (c) 2017 Ascam Ltd
  *
  *
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "filesys.h"
#include "stdio.h"
#include "tm_stm32_fatfs.h"
#include "ringfs.h"
#include "StorageMode.h"
#include "ringfs.h"

/* Fatfs structure */
FATFS FS;
FIL fil;
FRESULT fres;

TM_FATFS_Size_t CardSize;
char buffer[96];
char filename[15];
uint32_t session=1;

void generate_fname (uint32_t ses){

  sprintf(filename,"SD:session%u.txt",ses);
  
}

uint8_t transformed_buffer_to_uint8_t[96];
//extern struct ringfs fs;
void savedata (uint8_t num,uint32_t count,uint8_t hour,uint8_t min,uint8_t sec,uint16_t ms,uint8_t dat,uint8_t month,uint16_t year,float lat,float lng,float alt, float rel_altt, float roll, float pitch,float yaw){
  
		if   (rel_altt<0) {
			rel_altt = abs(rel_altt); 
		}
		// пишем маркер события
    if (num==1){
		sprintf(buffer,"A%05u    %02u:%02u:%04u   %02u:%02u:%02u:%04u     %.7f    %.7f     %.4f        %.4f \n",count,dat,month,year,hour,min,sec,ms,lat,lng,alt,rel_altt);
		}
    if (num==2){
		sprintf(buffer,"B%05u    %02u:%02u:%04u   %02u:%02u:%02u:%04u     %.7f    %.7f     %.4f        %.4f \n",count,dat,month,year,hour,min,sec,ms,lat,lng,alt,rel_altt);
		}	
    // пишем в файл и закрываем файл
    f_puts(buffer, &fil);
		//uint8_t transformed_buffer_to_uint8_t[96];
		for (int i = 0; i < 96; i++) {
			transformed_buffer_to_uint8_t[i] = (uint8_t)buffer[i];
		}
		
		ringfs_append(&ring_structure, transformed_buffer_to_uint8_t, 90, 30);
		
		
     
}

extern uint32_t getcapacity (void){
 
 return CardSize.Total;
  
}

void initfilesystem(void){

 if (f_mount(&FS, "SD:", 1) == FR_OK) {}
	else{
   f_mount(NULL, "SD:", 1);}
   
}

void initfile (void){
   
      // монтируем карту к файловой системе
	     if (f_mount(&FS, "SD:", 1) == FR_OK) {
	    // создаем файл
        if ((fres = f_open(&fil, filename, FA_OPEN_ALWAYS | FA_READ | FA_WRITE)) == FR_OK) {
		
      // создаем шапку файла
          sprintf(buffer,"Событие     Дата         Время UTC        Широта        Долгота	    Высота WGS84  Высота над точкой старта\n");
			// пишем в файл 
          f_puts(buffer, &fil);
        }
      }
}

void closefile (void){
   f_close(&fil);
  // f_mount(NULL, "SD:", 1);
   session++;generate_fname (session);
}
