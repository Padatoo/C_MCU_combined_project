#ifndef __RTC_H_
#define __RTC_H_

#define JULIAN_DATE_BASE     2440588   //день начала отсчета по Юлианской дате (UnixTime = 00:00:00 01.01.1970 = JD0 = 2440588)
#define INIT_RTC_VALUE    1325376000   //значение для первичной инициализации

//структура хранения даты и времени 
typedef struct
{
	uint16_t year;          //год     (xxxx)
	uint8_t month;          //месяц   (1-12)
	uint8_t day;            //день    (1-31)
	uint8_t hour;           //часы    (0-23)
	uint8_t min;            //минуты  (0-59)
	uint8_t sec;            //секунды (0-59)
  uint8_t dow;            //день недели 0 = воскресенье, 1 = понедельник, .... 6 = суббота.
} _type_datetime;
    
  extern const uint16_t week_day[];

  uint8_t rtc_init(void);                               //Инициализация RTC

  void rtc_get_dt(_type_datetime *dt);          //Получить текущие дату и время
  void rtc_set_dt(_type_datetime *dt);          //Установить текущие дату и время
  uint32_t rtc_get_cnt(void);                   //Получить текущие дату и время в секундах (RTC_Tick)
  void rtc_set_cnt(uint32_t cnt);               //Установить дату и время в секундах (RTC_Tick)

  void rtc_decode(uint32_t value, _type_datetime * dt);      //преобразовать секунды (RTC_Tick) в дату и время
  uint32_t rtc_code(_type_datetime *dt);                     //преобразовать дату и время в секунды (RTC_Tick)
  uint8_t rtc_dow(uint16_t year, uint8_t month, uint8_t day);//получить день недели
  uint8_t rtc_check_leap(uint16_t year);                     //проверить год на високосный

#endif