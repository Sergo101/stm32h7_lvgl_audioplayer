#include "gt911.h"
#include <stdio.h>
#include "string.h" 
#include "main.h"

//INT���Ÿ���
static void set_float(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Pin = TP_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(TP_INT_GPIO_Port, &GPIO_InitStruct);
}

static void set_int_out(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Pin = TP_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TP_INT_GPIO_Port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(TP_INT_GPIO_Port, TP_INT_Pin, GPIO_PIN_RESET);
}

//д�Ĵ���
static void GT911_WR_Reg_One_byte(uint16_t reg,uint8_t data)
{
	uint8_t temp[3]={0};
	temp[0]=reg>>8;
	temp[1]=reg&0xff;
	temp[2]=data;
	if(HAL_I2C_Master_Transmit(&hi2c4,GT_CMD_WR,temp,3,0xff)!=HAL_OK)
	{
		printf("write reg one byte error\r\n");
	}
}

//��GT911����һ������		  
static uint8_t GT911_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t res = 0;
	uint8_t temp[2]={0};
	temp[1]=reg&0xff;
	temp[0]=reg>>8;
	res = HAL_I2C_Master_Transmit(&hi2c4,GT_CMD_WR,temp,2,20);
	res += HAL_I2C_Master_Receive(&hi2c4,GT_CMD_RD,buf,len,20);
	return res;
}

//��ʼ��
void gt911_init(void)
{
	uint8_t temp[5]={0};
	
	temp[4]=0;
	
	for(uint16_t i = 0; i < 100; i++)
	{
		HAL_GPIO_WritePin(TP_RST_GPIO_Port,TP_RST_Pin,GPIO_PIN_RESET);
		HAL_Delay(5);
		/* INT pin need to be set as output, because when reset is settet, gt911 choose i2c address according INT pin */
		set_int_out();
		HAL_Delay(5);
		HAL_GPIO_WritePin(TP_RST_GPIO_Port,TP_RST_Pin,GPIO_PIN_SET);  
		HAL_Delay(5);
		set_float();
		if(GT911_RD_Reg(GT_PID_REG,temp,4) == HAL_OK)
		{
			break;
		}
	}

	printf("CTP ID:%s\r\n",temp);
	temp[0]=0X02;			
	GT911_WR_Reg_One_byte(GT_CTRL_REG,temp[0]);//����λGT9147
	GT911_RD_Reg(GT_CFGS_REG,temp,1);//��ȡGT_CFGS_REG�Ĵ���
	printf("TP firmware version: %d\r\n",temp[0]);
	
}

//��ȡ����
void gt911_get_state(TS_StateTypeDef* state)
{
	uint8_t flag=0;
	uint8_t buf[4]={0};
	GT911_RD_Reg(GT_GSTID_REG,&flag,1);
	if(flag&0x80)																		//�����Ƿ�׼����
	{
		//���־
		GT911_WR_Reg_One_byte(GT_GSTID_REG,0);
		if(flag&0xf)																	//�����ݵ�
		{
			//ֻ���һ����
			GT911_RD_Reg(GT_TP1_REG,buf,4);
			state->X=(uint16_t)((buf[1]<<8)|buf[0]);
			state->Y=(uint16_t)((buf[3]<<8)|buf[2]);
			state->TouchDetected=1;
		}
		else
			state->TouchDetected=0;
	}
	else
		state->TouchDetected=0;
}

