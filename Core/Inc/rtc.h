#ifndef __RTC_H_
#define __RTC_H_

#define JULIAN_DATE_BASE     2440588   //���� ������ ������� �� ��������� ���� (UnixTime = 00:00:00 01.01.1970 = JD0 = 2440588)
#define INIT_RTC_VALUE    1325376000   //�������� ��� ��������� �������������

//��������� �������� ���� � ������� 
typedef struct
{
	uint16_t year;          //���     (xxxx)
	uint8_t month;          //�����   (1-12)
	uint8_t day;            //����    (1-31)
	uint8_t hour;           //����    (0-23)
	uint8_t min;            //������  (0-59)
	uint8_t sec;            //������� (0-59)
  uint8_t dow;            //���� ������ 0 = �����������, 1 = �����������, .... 6 = �������.
} _type_datetime;
    
  extern const uint16_t week_day[];

  uint8_t rtc_init(void);                               //������������� RTC

  void rtc_get_dt(_type_datetime *dt);          //�������� ������� ���� � �����
  void rtc_set_dt(_type_datetime *dt);          //���������� ������� ���� � �����
  uint32_t rtc_get_cnt(void);                   //�������� ������� ���� � ����� � �������� (RTC_Tick)
  void rtc_set_cnt(uint32_t cnt);               //���������� ���� � ����� � �������� (RTC_Tick)

  void rtc_decode(uint32_t value, _type_datetime * dt);      //������������� ������� (RTC_Tick) � ���� � �����
  uint32_t rtc_code(_type_datetime *dt);                     //������������� ���� � ����� � ������� (RTC_Tick)
  uint8_t rtc_dow(uint16_t year, uint8_t month, uint8_t day);//�������� ���� ������
  uint8_t rtc_check_leap(uint16_t year);                     //��������� ��� �� ����������

#endif