#ifndef __CLRD125_H
#define __CLRD125_H

#include "gd32f4xx.h"
#include <stdint.h>
#include <stdio.h>
#include "common.h"
#include "LT1258.h"
#include "systick.h"

// ###########################################################################
// 1. 新增：CLRD125配置表格（D[0]~D[79]，用户提供的表格值，头文件声明避免多定义）
// 注：extern声明全局数组，实际定义在CLRD125.c中，防止头文件重复包含导致多定义错误
// ###########################################################################
extern uint8_t D[80];

// ###########################################################################
// 2. CLRD125设备地址
// ###########################################################################
#define CLRD125_DEVICE_ADDR    0x30  

// ###########################################################################
// 3. CLRD125 SMBus寄存器定义
// ###########################################################################
#define CLRD125_REG_INPUT_EN   0x01  // INPUT_EN控制（引脚22，SMBus模式）
#define CLRD125_REG_SEL0       0x02  // 通道0路径选择（引脚23）
#define CLRD125_REG_SEL1       0x03  // 通道1路径选择（引脚26）
#define CLRD125_REG_EQ_D0      0x04  // D侧均衡控制0（EQ_D0，引脚46）
#define CLRD125_REG_EQ_D1      0x05  // D侧均衡控制1（EQ_D1，引脚47）
#define CLRD125_REG_EQ_S0      0x06  // S侧均衡控制0（EQ_S0，引脚20）
#define CLRD125_REG_EQ_S1      0x07  // S侧均衡控制1（EQ_S1，引脚19）
#define CLRD125_REG_DEM_D0     0x08  // D侧去加重控制0（DEM_D0，引脚49）
#define CLRD125_REG_DEM_D1     0x09  // D侧去加重控制1（DEM_D1，引脚54）
#define CLRD125_REG_DEM_S0     0x0A  // S侧去加重控制0（DEM_S0，引脚50）
#define CLRD125_REG_DEM_S1     0x0B  // S侧去加重控制1（DEM_S1，引脚53）
#define CLRD125_REG_VOD        0x0C  // 输出摆幅控制（600~1300mVp-p）

// ###########################################################################
// 4. 关键修改：信号调理参数从表格提取（替换原固定值，确保与原功能一致）
// 映射逻辑基于CLRD125寄存器位宽（EQ/DEM为2位控制，INPUT_EN为1位控制）
// ###########################################################################
// INPUT_EN=1（扇出启用）：D[6]=0x08（二进制00001000）→ 第3位=1，取低1位有效（与原CLRD125_INPUT_EN_VAL=0x01一致）
// #define CLRD125_INPUT_EN_VAL   ((D[6] >> 3) & 0x01)  
// // SEL0=1（S_INA0±→S_OUTA0±）：D[5]=0xFF（11111111）→ 低1位=1（与原CLRD125_SEL0_VAL=0x01一致）
// #define CLRD125_SEL0_VAL       (D[5] & 0x01)          
// // SEL1=1（S_INA1±→S_OUTA1±）：D[5]低2位=11→右移1位取第2位=1（与原CLRD125_SEL1_VAL=0x01一致）
// #define CLRD125_SEL1_VAL       ((D[5] >> 1) & 0x01)   
// // EQ=30dB（EQ_D[1:0]/EQ_S[1:0]=11）：D[8]=0x0C（00001100）→ 低2位=11（与原CLRD125_EQ_VAL=0x03一致）
// #define CLRD125_EQ_VAL         (D[8] & 0x03)          
// // 去加重=-12dB（DEM_D[1:0]/DEM_S[1:0]=10）：D[3]=0x02（00000010）→ 低2位=10（与原CLRD125_DEEMP_VAL=0x02一致）
// #define CLRD125_DEEMP_VAL      (D[3] & 0x03)          
// // 输出摆幅=1000mVp-p：D[23]=0x05（00000101）→ 低4位=05（与原CLRD125_VOD_VAL=0x05一致）
// #define CLRD125_VOD_VAL        (D[23] & 0x0F)         

// 新宏定义（替换后，匹配LT1258功能）
#define CLRD125_EQ_VAL         0x0F  // 30dB均衡（接近LT1258的25dB，补偿能力更强）
#define CLRD125_DEEMP_VAL      0x01  // -3.5dB去加重（与LT1258的-3.2dB效果一致）
#define CLRD125_VOD_VAL        0x04  // 1.0Vp-p摆幅（与LT1258的0.9Vp-p接近）
#define CLRD125_INPUT_EN_VAL   0x03  // input_en=11，启用1:2扇出（与LT1258扇出功能一致）
#define CLRD125_SEL0_VAL       0x03  // sel0=11，S_INA0±→S_OUTA0±（模拟LT1258路由选择）
#define CLRD125_SEL1_VAL       0x03  // sel1=11，S_INA1±→S_OUTA1±（模拟LT1258路由选择）

// ###########################################################################
// 5. 通用状态定义
// ###########################################################################
#define STA_OK                 0
#define STA_ERR                1

// ###########################################################################
// 6. 公开接口声明
// ###########################################################################
ERR_STA clrd125Init(void);                  
ERR_STA clrd125Read(uint8_t regAddr, uint8_t *val);  
ERR_STA clrd125Write(uint8_t regAddr, uint8_t val);   
ERR_STA clrd125InitWithRetry(void);

#endif

