/////////////// фотоаппарат ///////////////////////

#include "stm32f1xx_hal.h"
#include <stdbool.h>

void receive_cmd(void);
void parse (float *pData, uint8_t pSize);
void shoot1 (void);
void shoot2 (void);
void camera_systick(void);
void feedback_state(uint8_t in1,uint8_t in2,uint8_t st);
void feedback_shoot (uint8_t num,uint32_t count,bool foto_status);
float calcDistance (float latA, float lonA, float latB, float lonB);
void  unixtime_to_datetime ( uint64_t unixtime,
                             uint32_t *year, uint32_t *mon, uint32_t *mday, uint32_t *wday,
                             uint32_t *hour, uint32_t *min, uint32_t *sec,
                             uint64_t *jd, uint64_t *jdn );

///////////////////////////////////////////////////
