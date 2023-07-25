#include "rtc.h"

//const uint8_t month_day[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

const uint16_t week_day[] = { 0x4263, 0xA8BD, 0x42BF, 0x4370, 0xABBF, 0xA8BF, 0x43B2};


//------------------------------------------------------------ ������������� RTC
//uint8_t rtc_init(void)
//{
//  uint8_t result = 0;
//  RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;  //��������� ������������ ������� ���������� �������� � ����������� ��������� ��������
//  PWR->CR |= PWR_CR_DBP;                              //��������� ������ � ������� ��������� ������
//  if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN) //���� ���� ��������� - �������������� ��
//  {    
//    RCC->BDCR |=  RCC_BDCR_BDRST;          //��������� ����� ������� ��������� ������
//    RCC->BDCR &= ~RCC_BDCR_BDRST;    
//    RCC->BDCR |=  RCC_BDCR_RTCSEL_LSE;     //������� ����� 32768
//    RCC->BDCR |=  RCC_BDCR_RTCEN;          //������ ������������
//    RTC->CRL  |=  RTC_CRL_CNF;             //��������� ��������� ��������� RTC
//    RTC->PRLL  =  0x7FFF;                  //��������� ������� ��������
//    RTC->CRL  &= ~RTC_CRL_CNF;             //��������� ��������� ��������� RTC
//      RCC->BDCR |= RCC_BDCR_LSEON;         //�������� ������� ��������� 32768
//      while ((RCC->BDCR & RCC_BDCR_LSEON) != RCC_BDCR_LSEON); //��������� ����� ����������     
//    result = 1;
//  }
//  RTC->CRL &= (uint16_t)~RTC_CRL_RSF;
//  while((RTC->CRL & RTC_CRL_RSF) != RTC_CRL_RSF);
//    if(result) //���� ���� ��� ������ ����������������...
//    {
//      rtc_set_cnt(INIT_RTC_VALUE); //������������� ��������� ��������
//    }  
//  return result; // result: 0 - ���� ��� ���� ���������������, 1 - ������������� ���������
//}
// 
//-------------------------------------- ���������� ���������� �������� ��������
uint32_t rtc_get_cnt(void)
{
  return  (uint32_t)((RTC->CNTH << 16) | RTC->CNTL);
}

//----------------------------------- ���������� ������� ���� � ����� � ��������
void rtc_set_cnt(uint32_t cnt)
{  
  PWR->CR |= PWR_CR_DBP;                        // ��������� ������ � ��������� RTC
  while ((RTC->CRL & RTC_CRL_RTOFF) == 0);      // ������� ��������� ������
  RTC->CRL |=  RTC_CRL_CNF;                     // �������� ����� ����������������
    RTC->CNTH = ((cnt)>>16) & 0xFFFF;           // �������� �������� HIGH
    RTC->CNTL = ((cnt)    ) & 0xFFFF;           // �������� �������� LOW 
  RTC->CRL &= ~RTC_CRL_CNF;                     // ��������� ����������������
  while ((RTC->CRL & RTC_CRL_RTOFF) == 0);      // ������� ��������� ������
  PWR->CR &= ~PWR_CR_DBP;                       // ��������� ������ ��� ������ � ��������� RTC
}

//----------------------------------------- ���������� ������� ���� � ����� � dt
void rtc_get_dt(_type_datetime *dt)
{
  uint32_t value = (uint32_t)((RTC->CNTH << 16) | RTC->CNTL);
  rtc_decode(value, dt);
}

//---------------------------------------- ���������� ������� ���� � ����� �� dt
void rtc_set_dt(_type_datetime *dt)
{
  uint32_t value = rtc_code(dt);
  rtc_set_cnt(value);
}

//-----------------------------------------------------------------------------
uint32_t rtc_code(_type_datetime *dt)
{  
  uint8_t a;
  uint16_t y;
  uint8_t m;
  uint32_t JDN;

// ���������� ����������� �������������
    a=(14-dt->month)/12;
    y=dt->year+4800-a;
    m=dt->month+(12*a)-3;
// ��������� �������� �������� ���������� ���
    JDN=dt->day;
    JDN+=(153*m+2)/5;
    JDN+=365*y;
    JDN+=y/4;
    JDN+=-y/100;
    JDN+=y/400;
    JDN = JDN -32045;
    JDN = JDN - JULIAN_DATE_BASE; //������ �� ������� ����
    JDN*=86400;     // ��������� ��� � �������
    JDN+=(dt->hour*3600); // � ��������� ��� �������� �������� ���
    JDN+=(dt->min*60);
    JDN+=(dt->sec);
  return JDN;
  /*
  
  uint32_t tmp = 0;
  uint16_t year = 0;    
  uint8_t i = 0;
  
  year = dt.year - YEAR_BASE;
	tmp = year * (365 * 24 * 3600) + (year / 4) * 24 * 3600;

    for (i = 0; i < (dt.month - 1); i++)
    {
      tmp += 24 * 3600 * month_day[i];
    }

    tmp += (dt.day - 1) * 24 * 3600;
    tmp += dt.hour * 3600;
    tmp += dt.min * 60;

	return (tmp + dt.sec);
  */
}

//-----------------------------------------------------------------------------
void rtc_decode(uint32_t value, _type_datetime * dt)
{
	
  unsigned long time;
  unsigned long t1;
  unsigned long a;
  unsigned long b;
  unsigned long c;
  unsigned long d;
  unsigned long e;
  unsigned long m;
  int year = 0;
  int mon = 0;
  int wday = 0;  
  int mday = 0;  
  int hour = 0;
  int min = 0;
  int sec = 0;
  uint64_t jd = 0;;
  uint64_t jdn = 0;
 
        jd = ((value+43200)/(86400>>1)) + (2440587<<1) + 1;
        jdn = jd>>1;
 
        time = value;   t1 = time/60;    sec = time - t1*60;
        time = t1;         t1 = time/60;    min = time - t1*60;
        time = t1;         t1 = time/24;    hour = time - t1*24;
 
        wday = jdn%7;
 
        a = jdn + 32044;
        b = (4*a+3)/146097;
        c = a - (146097*b)/4;
        d = (4*c+3)/1461;
        e = c - (1461*d)/4;
        m = (5*e+2)/153;
        mday = e - (153*m+2)/5 + 1;
        mon = m + 3 - 12*(m/10);
        year = 100*b + d - 4800 + (m/10);
        
        dt->year = year;
        dt->month = mon;
        dt->day = mday;
        dt->hour = hour;
        dt->min = min;
        dt->sec = sec;
        dt->dow = wday;
        
  
  /*
  int16_t tmp = 0;
  int8_t year = 0;
  int8_t month = 0;
  int8_t day = 0;
//���������� ���
	year = (value / (365 * 24 * 3600));  
//��������� ���������� ���
	tmp = ((value % (365 * 24 * 3600)) / (24 * 3600)) - (year / 4);
	if ((tmp <= 0) && (year > 3))
	{
		year--;        
		if (rtc_check_leap(year + YEAR_BASE))
		{
			tmp += 366;
		}
		else
		{
			tmp += 365;
		}
	}

  dt->year = year + YEAR_BASE;   //����������� ���
    
	while (tmp >= 0) //������� ��� � ������
	{
		day = tmp;
		if ((month++) == 2)
		{
			if (rtc_check_leap(dt->year))
				tmp -= (month_day[month - 1] + 1);
			else
				tmp -= month_day[month - 1];
		}
		else
		{
			tmp -= month_day[month - 1];
		}    
	}

	//dt->dow = rtc_dow(dt->year, dt->month, dt->day);
    
	dt->hour = ((value % (365 * 24 * 3600)) % (24 * 3600)) / 3600;
	dt->min = (((value % (365 * 24 * 3600)) % (24 * 3600)) % 3600) / 60;
	dt->sec = (((value % (365 * 24 * 3600)) % (24 * 3600)) % 3600) % 60;
  
  dt->month = month;
  dt->day = day + 1;
  
  */
}

//--------------------------------------------------------- �������� ���� ������
uint8_t rtc_dow(uint16_t year, uint8_t month, uint8_t day)
{
	uint16_t tmp1, tmp2, tmp3, tmp4, week_day;
    if (month < 3)
    {
      month = month + 12;
      year = year - 1;
    }
	tmp1 = (6 * (month + 1)) / 10;
	tmp2 = year / 4;
	tmp3 = year / 100;
	tmp4 = year / 400;
	week_day = day + (2 * month) + tmp1 + year + tmp2 - tmp3 + tmp4 + 1;
	week_day = week_day % 7;
  return (week_day);
}

//-------------------------------------------------- ��������� ��� �� ����������
uint8_t rtc_check_leap(uint16_t year)
{
	if ((year % 400) == 0)
	{
		return TRUE; //����������
	}
	else if ((year % 100) == 0)
	{
		return FALSE;
	}
	else if ((year % 4) == 0)
	{
		return TRUE; //����������
	}
	else
	{
		return FALSE;
	}
}

