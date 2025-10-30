#ifndef __CLRD125_H
#define __CLRD125_H

#include "gd32f4xx.h"
#include <stdint.h>
#include <stdio.h>
#include "common.h"
#include "LT1258.h"
#include "systick.h"


// 1. CLRD125设备地址（仅设置1片，与3片LT1258地址区分，需按硬件AD0-AD3接线调整）
#define CLRD125_DEVICE_ADDR    0x30  // 自定义地址，需与LT1258的B0h~CEh地址不冲突

// 2. CLRD125 SMBus寄存器定义（无BANK切换）
#define CLRD125_REG_INPUT_EN   0x01  // INPUT_EN控制（对应引脚22，SMBus模式）
#define CLRD125_REG_SEL0       0x02  // 通道0路径选择（对应引脚23）
#define CLRD125_REG_SEL1       0x03  // 通道1路径选择（对应引脚26）
#define CLRD125_REG_EQ_D0      0x04  // D侧（D_INn±）均衡控制0（EQ_D0，引脚46）
#define CLRD125_REG_EQ_D1      0x05  // D侧均衡控制1（EQ_D1，引脚47）
#define CLRD125_REG_EQ_S0      0x06  // S侧（S_INAn±）均衡控制0（EQ_S0，引脚20）
#define CLRD125_REG_EQ_S1      0x07  // S侧均衡控制1（EQ_S1，引脚19）
#define CLRD125_REG_DEM_D0     0x08  // D侧（D_OUTn±）去加重控制0（DEM_D0，引脚49）
#define CLRD125_REG_DEM_D1     0x09  // D侧去加重控制1（DEM_D1，引脚54）
#define CLRD125_REG_DEM_S0     0x0A  // S侧（S_OUTn±）去加重控制0（DEM_S0，引脚50）
#define CLRD125_REG_DEM_S1     0x0B  // S侧去加重控制1（DEM_S1，引脚53）
#define CLRD125_REG_VOD        0x0C  // 输出摆幅控制（600~1300mVp-p，对应发送输出电压控制）

// 3. 信号调理参数（基于CLRD125特性）
#define CLRD125_EQ_VAL         0x03  // 均衡等级：30dB（6.25GHz，EQ_D[1:0]/EQ_S[1:0]=11）
#define CLRD125_DEEMP_VAL      0x02  // 去加重等级：-12dB（DEM_D[1:0]/DEM_S[1:0]=10）
#define CLRD125_VOD_VAL        0x05  // 输出摆幅：1000mVp-p（600~1300mVp-p中间值）
#define CLRD125_INPUT_EN_VAL   0x01  // INPUT_EN=1：扇出启用（两S_OUT均开启，输入50Ω始终使能）
#define CLRD125_SEL0_VAL       0x01  // 通道0：S_INA0±输入→S_OUTA0±输出（表3：SEL0=1）
#define CLRD125_SEL1_VAL       0x01  // 通道1：S_INA1±输入→S_OUTA1±输出（表3：SEL1=1）

// 4. 通用状态定义
#define STA_OK                 0
#define STA_ERR                1

// #ifndef ERR_STA
// typedef uint8_t ERR_STA;
// #endif

// 5. 公开接口声明
ERR_STA clrd125Init(void);                  // CLRD125初始化（仅设置为初始化1片）
ERR_STA clrd125Read(uint8_t regAddr, uint8_t *val);  // CLRD125寄存器读
ERR_STA clrd125Write(uint8_t regAddr, uint8_t val);   // CLRD125寄存器写
ERR_STA clrd125InitWithRetry(void);

#endif

