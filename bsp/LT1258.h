/*!
    \file  LT1258.c
    \brief
*/

#ifndef LT1258_H
#define LT1258_H

#include "gd32f4xx.h"
#include "gpio_i2c.h"

// 芯片ID与版本信息（补充文档性说明）
// 芯片ID寄存器：0x8100=0x20, 0x8101=0x02；版本寄存器0x8102=0xE1
#define LT1258_I2C GPIO_I2C1  // 使能I2C通道1

/******************* 硬件地址定义 *******************/
// LT1258设备I2C地址（7位地址，已右移1位）
#define LT1258_DEVICE_ADDR_1 0x18  // 对应0x30 >> 1
#define LT1258_DEVICE_ADDR_2 0x1C  // 对应0x38 >> 1
#define LT1258_DEVICE_ADDR_3 0x20  // 对应0x40 >> 1
#define LT1258_DEVICE_ADDR_4 0x24  // 对应0x48 >> 1

/******************* 位定义 *******************/
#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

// ************************** 全局宏定义 **************************
#define LT1258_DEVICE_ADDRS  {LT1258_DEVICE_ADDR_1, LT1258_DEVICE_ADDR_2, LT1258_DEVICE_ADDR_3, LT1258_DEVICE_ADDR_4}
#define LT1258_DEVICE_NUM    4  

#define REG_BANK_SEL         0xFF    
#define BANK_81              0x81    
#define BANK_80              0x80    

#define REG_INEN_SEL_EN      0x60    
#define REG_CH_PARAM         0x63    
#define BIT_INEN_EN          BIT2    
#define BIT_SEL0_EN          BIT0    
#define BIT_SEL1_EN          BIT1    

#define REG_EQ_CTRL_BASE     0x38    
#define REG_EQ_PARAM_BASE    0x39    
#define EQ_CTRL_EN_VAL       0xC0    
#define EQ_PARAM_VAL         0x70    
#define CH_EQ_OFFSET_0       0x00    
#define CH_EQ_OFFSET_1       0x40    
#define CH_EQ_OFFSET_2       0x60    
#define CH_EQ_OFFSET_3       0xA0    

#define REG_DEEMP_CTRL_BASE  0x3A    
#define REG_DEEMP_PARAM_BASE 0x3B    
#define REG_SWING_PARAM_BASE 0x3C    
#define DEEMP_CTRL_EN_VAL    0x03    
#define DEEMP_PARAM_VAL      0x70    
#define SWING_PARAM_VAL      0x20    

#define RETRY_CNT            3      

/******************* 函数声明 *******************/
/*!
    \brief      初始化LT1258芯片（批量初始化所有4个芯片）
    \param      void
    \retval     ERR_STA: STA_OK=成功，STA_ERR=失败
*/
extern ERR_STA lt1258Init(void);

/*!
    \brief      向LT1258写入数据
    \param      deviceAddr: 设备地址（LT1258_DEVICE_ADDR_1~4）
    \param      regAddr: 寄存器地址
    \param      val: 要写入的数值
    \retval     ERR_STA: STA_OK=成功，STA_ERR=失败
*/
extern ERR_STA lt1258Write(uint8_t deviceAddr, uint8_t regAddr, uint8_t val);

/*!
    \brief      从LT1258读取数据
    \param      deviceAddr: 设备地址（LT1258_DEVICE_ADDR_1~4）
    \param      regAddr: 寄存器地址
    \param      val: 存储读取结果的指针
    \retval     ERR_STA: STA_OK=成功，STA_ERR=失败
*/
extern ERR_STA lt1258Read(uint8_t deviceAddr, uint8_t regAddr, uint8_t *val);

#endif /* LT1258_H */

