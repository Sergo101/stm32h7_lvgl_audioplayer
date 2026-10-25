#ifndef __GT911_H
#define __GT911_H

#ifdef __cplusplus
extern "C"
{
#endif
	
#include "stm32h7xx.h"
#include "i2c.h"


#define GT_CMD_WR     0XBA
#define GT_CMD_RD     0XBB
  
#define GT_CTRL_REG 	0X8040
#define GT_CFGS_REG 	0X8047
#define GT_Module_Switch1 	0X804D
#define GT_CHECK_REG 	0X80FF
#define GT_PID_REG 		0X8140
#define GT_GSTID_REG 	0X814E
#define GT_TP1_REG 		0X8150
#define GT_TP2_REG 		0X8158
#define GT_TP3_REG 		0X8160
#define GT_TP4_REG 		0X8168
#define GT_TP5_REG 		0X8170
	
typedef struct
{
  uint16_t TouchDetected;
  uint16_t X;
  uint16_t Y;
} TS_StateTypeDef;

void gt911_init(void);
void gt911_get_state(TS_StateTypeDef* state);
void gt911_DataRecievedCallback (struct __I2C_HandleTypeDef *hi2c);
void gt911_StartConvesion(void);

#ifdef __cplusplus
}
#endif

#endif

