/**
  ******************************************************************************
  * File Name          : camera.c
  * Description        : code4drive fotocamera
  ******************************************************************************
  *
  * Copyright (c) 2017 Ascam Ltd
  *
  *
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "camera.h"
#include "usart.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "filesys.h"
#include "math.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////
const float M_PI =3.14159265358979323846264338328f;
const float EARTH_RADIUS =6372795.0f;

uint8_t RXBuffer[32];


volatile uint32_t shut_int_f1=2; // по умолчанию интервал 2 секунды
volatile uint32_t shut_int_f2=2;
volatile bool start_shoot=false;
volatile bool pwm_shoot=false;
volatile bool foto_status =false;
volatile float intDist=0.0f;
volatile float last_intDist1=0.0f;
volatile float last_intDist2=0.0f;

float Lat =0.0f;
float Lng =0.0f;
float lastLat, lastLng;
float decLat,decLng;
float Alt =0.0f;
float rel_altt=0.0f;
float roll =0.0f;
float pitch =0.0f;
float yaw =0.0f;


uint32_t hour =0;
uint32_t min =0;
uint32_t sec =0;
uint64_t ms =0;
uint64_t utc_time=0; 
uint32_t date=0;
uint32_t dat=0;
uint32_t month=0;
uint32_t year=0;
uint64_t jd=0;
uint64_t jdn=0;

uint32_t pdat;

static uint32_t last_timef1;
static uint32_t last_timef2;
static uint32_t htime;
static uint32_t ltime;


volatile uint32_t count_timef1=0;
volatile uint32_t count_timef2=0;
volatile uint32_t shoot1_time=0;
volatile uint32_t shoot2_time=0;


//////////////////////////////////////////////////////////////////////////////////////////////////////////
void recive_cmd(void){ 
  // обрабатываем новые данные и/или команды от автопилота
  if (!HAL_UART_Receive_IT(&huart1,RXBuffer,32)){
      
    parse((float*)&RXBuffer,8);
    for (uint8_t i=0;i<32;i++){
        RXBuffer[i]=0;
    }
  }
  // запускаем интервальную съёмку первого фотоаппарата по времени
  if (start_shoot && (shut_int_f1<10)){
      if  (((HAL_GetTick() - last_timef1) > (shut_int_f1*1000))&&shut_int_f1) {
			shoot1();
			last_timef1=HAL_GetTick();
			}
      
      if ((shoot1_time>2)&&(HAL_GPIO_ReadPin(shtorka_A_GPIO_Port,shtorka_A_Pin)==GPIO_PIN_RESET)) {
        HAL_GPIO_WritePin(EVENTA_GPIO_Port,EVENTA_Pin,GPIO_PIN_RESET);
        shoot1_time=0;
				savedata(1,count_timef1,hour,min,sec,ms,date,month,year,Lat,Lng,Alt,rel_altt,roll,pitch,yaw);
        HAL_GPIO_WritePin(EVENTA_GPIO_Port,EVENTA_Pin,GPIO_PIN_SET);
				feedback_shoot(1,count_timef1,foto_status); foto_status=false;
				count_timef1++;
      }
  }
	
  // запускаем интервальную съёмку второго фотоаппарата по времени
	 if (start_shoot && (shut_int_f2<10)){
       if  (((HAL_GetTick() - last_timef2) > (shut_int_f2*1000))&&shut_int_f2) {
	     shoot2();
	     last_timef2=HAL_GetTick();
	 }
	
       if ((shoot2_time>2)&&(HAL_GPIO_ReadPin(shtorka_B_GPIO_Port,shtorka_B_Pin)==GPIO_PIN_RESET)) {
         HAL_GPIO_WritePin(EVENTB_GPIO_Port,EVENTB_Pin,GPIO_PIN_RESET);
         shoot2_time=0;
         savedata(2,count_timef2,hour,min,sec,ms,date,month,year,Lat,Lng,Alt,rel_altt,roll,pitch,yaw);
         HAL_GPIO_WritePin(EVENTB_GPIO_Port,EVENTB_Pin,GPIO_PIN_SET);
				 feedback_shoot(2,count_timef2,foto_status);foto_status=false;
				 count_timef2++;
      }  
  } 
	 
	// запускаем интервальную съёмку первого фотоаппарата по расстоянию
	if (start_shoot && (shut_int_f1>9)){
      if  ((intDist - last_intDist1) > shut_int_f1) {
	     shoot1();
	     last_intDist1=intDist;
			}		
			if ((shoot1_time>2)&&(HAL_GPIO_ReadPin(shtorka_A_GPIO_Port,shtorka_A_Pin)==GPIO_PIN_RESET)) {
        HAL_GPIO_WritePin(EVENTA_GPIO_Port,EVENTA_Pin,GPIO_PIN_RESET);
        shoot1_time=0;
				savedata(1,count_timef1,hour,min,sec,ms,date,month,year,Lat,Lng,Alt,rel_altt,roll,pitch,yaw);
        HAL_GPIO_WritePin(EVENTA_GPIO_Port,EVENTA_Pin,GPIO_PIN_SET);
				feedback_shoot(1,count_timef1,foto_status); foto_status=false;
				count_timef1++;
      }
	}
	
	// запускаем интервальную съёмку второго фотоаппарата по расстоянию
	if (start_shoot && (shut_int_f2>9)){
		  if  ((intDist - last_intDist2) > shut_int_f2) {
	     shoot2();
	     last_intDist2=intDist;
			}		
			if ((shoot2_time>2)&&(HAL_GPIO_ReadPin(shtorka_B_GPIO_Port,shtorka_B_Pin)==GPIO_PIN_RESET)) {
         HAL_GPIO_WritePin(EVENTB_GPIO_Port,EVENTB_Pin,GPIO_PIN_RESET);
         shoot2_time=0;
         savedata(2,count_timef2,hour,min,sec,ms,date,month,year,Lat,Lng,Alt,rel_altt,roll,pitch,yaw);
         HAL_GPIO_WritePin(EVENTB_GPIO_Port,EVENTB_Pin,GPIO_PIN_SET);
				 feedback_shoot(2,count_timef2,foto_status);foto_status=false;
				 count_timef2++;
      }
	}	
	
	// отпускаем кнопку спуска по таймауту

  if (shoot1_time<1) {
		HAL_GPIO_WritePin(shot_A_GPIO_Port,shot_A_Pin,GPIO_PIN_SET);
	}
  if (shoot2_time<1) {
		HAL_GPIO_WritePin(shot_B_GPIO_Port,shot_B_Pin,GPIO_PIN_SET);
	}
	
}  

void shoot1 (void){
 	HAL_GPIO_WritePin(shot_A_GPIO_Port,shot_A_Pin,GPIO_PIN_RESET);
  shoot1_time=1000;
}

void shoot2 (void){
  HAL_GPIO_WritePin(shot_B_GPIO_Port,shot_B_Pin,GPIO_PIN_RESET);
  shoot2_time=1000;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void parse (float *pData, uint8_t pSize ){
  

	// 100Гц данные от автопилота
  if (pData[0]==1111){
     lastLat=Lat;
     lastLng=Lng;
     Lat=pData[1];
     Lng=pData[2];
     decLat=Lat-lastLat;
     decLng=Lng-lastLng;
     Alt=pData[3];
     htime=(uint32_t)pData[4];
     ltime=(uint32_t)pData[5];
     rel_altt=pData[6];
		 utc_time =htime* 4294967296+ltime;
		 ms = utc_time%1000;
     utc_time  /= 1000000;
		 unixtime_to_datetime ( utc_time,&year,&month,&date,&dat,
                             &hour,&min,&sec,
                             &jd,&jdn );
		 if (lastLat && lastLng) {
		   intDist +=calcDistance (lastLat, lastLng , Lat, Lng);
		 }
	   return;
     
  }
	// получение интервала
  if (pData[0]==3333){
		if (pData[5]==1){shut_int_f1=(uint8_t)pData[2];};
    if (pData[5]==2){shut_int_f2=(uint8_t)pData[2];};  
    if (pData[5]==3){ 
	                  if 	(start_shoot) {foto_status=true;}
										    else {feedback_state((uint8_t)shut_int_f1,(uint8_t)shut_int_f2,0);}
		}
    return;
  }
	// запуск фотоаппарата(-ов)
  if (pData[0]==2222){
    if (pData[5]==1){
      shoot1();
    }
    if (pData[5]==2){
      shoot2();
    }  
    if (pData[5]==3){
              if (start_shoot==true) {
                           shoot1_time=0;
													 shoot2_time=0;
													 start_shoot=false;
													 closefile();
													 
              } else {
              initfile (); 
              start_shoot=true;
							}	
    } 
    if (pData[5]==4){ 
			start_shoot=false; closefile(); 
		} 
  } 
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void camera_systick(void){

if (shoot1_time>0) shoot1_time--;
if (shoot2_time>0) shoot2_time--;  
}

void feedback_state(uint8_t in1,uint8_t in2,uint8_t f_st){
	char tx_buffer[6];
	uint8_t ver=3;
	sprintf(tx_buffer,"S%01u%01u%01u%01u%01u",ver,in1,in2,f_st,0);
  HAL_UART_Transmit(&huart1, (uint8_t*)tx_buffer,6, 20);
}

void feedback_shoot (uint8_t num,uint32_t count, bool ft_status){
		
	char tx_buffer[6];
	if (!ft_status) {
		  if (num==1){
		           sprintf(tx_buffer,"A%05u",count);
		           HAL_UART_Transmit(&huart1, (uint8_t*)tx_buffer,6, 20);
		}
    if (num==2){
		           sprintf(tx_buffer,"B%05u",count);
		           HAL_UART_Transmit(&huart1, (uint8_t*)tx_buffer,6, 20);
		}	
		return;
	}
	uint8_t f_st;
	if (ft_status) {f_st =1;} else {f_st=0;}
	sprintf(tx_buffer,"S%01u%01u%01u%01u%01u",3,(uint8_t)shut_int_f1,(uint8_t)shut_int_f1,f_st,0);
  HAL_UART_Transmit(&huart1, (uint8_t*)tx_buffer,6, 20);
}

void  unixtime_to_datetime ( uint64_t unixtime,
                             uint32_t *year, uint32_t *mon, uint32_t *mday, uint32_t *wday,
                             uint32_t *hour, uint32_t *min, uint32_t *sec,
                             uint64_t *jd, uint64_t *jdn )
{
        uint64_t time;
        uint64_t t1;
        uint64_t a;
        uint64_t b;
        uint64_t c;
        uint64_t d;
        uint64_t e;
        uint64_t m;
	
        *jd  = ((unixtime+43200)/(86400>>1)) + (2440587<<1) + 1;
        *jdn = *jd>>1;

        time = unixtime;   t1 = time/60;    *sec  = time - t1*60;
        time = t1;         t1 = time/60;    *min  = time - t1*60;
        time = t1;         t1 = time/24;    *hour = time - t1*24;

        *wday = *jdn%7;

        a = *jdn + 32044;
        b = (4*a+3)/146097;
        c = a - (146097*b)/4;
        d = (4*c+3)/1461;
        e = c - (1461*d)/4;
        m = (5*e+2)/153;
        *mday = e - (153*m+2)/5 + 1;
        *mon  = m + 3 - 12*(m/10);
        *year = 100*b + d - 4800 + (m/10);
}


float calcDistance (float latA, float lonA, float latB, float lonB) {
  
// перевести координаты в радианы
float lat1 = latA * M_PI / 180;
float lat2 = latB * M_PI / 180;
float lon1 = lonA * M_PI / 180;
float lon2 = lonB * M_PI / 180;
  
// косинусы и синусы широт и разницы долгот
float cl1 = cos(lat1);
float cl2 = cos(lat2);
float sl1 = sin(lat1);
float sl2 = sin(lat2);
float delta = lon2 - lon1;
float cdelta = cos(delta);
float sdelta = sin(delta);
  
// вычисления длины большого круга
float y = sqrt(pow(cl2 * delta, 2) + pow(cl1 * sl2 - sl1 * cl2 * cdelta, 2));
float x = sl1 * sl2 + cl1 * cl2 * cdelta;
  
//
float ad = atan2(y, x);
float dist = ad * EARTH_RADIUS;
  
return dist;
}
