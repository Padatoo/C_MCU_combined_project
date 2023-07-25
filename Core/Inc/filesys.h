#include "stdint.h"


void savedata(uint8_t num,uint32_t count,uint8_t hour,uint8_t min,uint8_t sec,uint16_t ms,uint8_t dat,uint8_t month,uint16_t year,float lat,float lng, float alt,float rel_altt,float roll, float pitch,float yaw);
void initfile (void);
void closefile(void);
void generate_fname (uint32_t session);
void initfilesystem(void);
void next_fname (uint32_t session);
extern uint32_t getcapacity (void);
