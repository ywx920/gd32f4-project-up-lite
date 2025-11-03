#include "CLRD125.h"
#include "LT1258.h"  
#include <string.h>

// CLRD125配置表格（D[0]~D[79]，原始表格值，全局数组）
uint8_t D[80] = {
    0x04,0x00,0x01,0x02,0x02,0xFF,0x08,0x00,0x0C,0xA0,
    0xA0,0x00,0x00,0xAA,0x0C,0x03,0x00,0x00,0x00,0x00,
    0x00,0x0E,0x03,0x05,0x02,0x00,0x00,0x00,0x0C,0x03,
    0x00,0x00,0x00,0x00,0x00,0x0E,0x03,0x05,0x02,0x00,
    0x22,0x00,0x00,0x0E,0x00,0x05,0x02,0x00,0x00,0x00,
    0x0C,0x00,0x05,0x02,0x00,0x00,0x00,0x0E,0x00,0x05,
    0x02,0x00,0x00,0x00,0x0C,0x00,0x05,0x02,0x55,0xFA,
    0xFF,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x07,0xFC
};

// 静态辅助函数
static ERR_STA clrd125I2cHandler(uint8_t regAddr, uint8_t *pData, uint8_t isWrite)
{
    uint8_t retry = RETRY_CNT;  
    ERR_STA sta = STA_ERR;

    while (retry--)
    {
        if (isWrite)
        {
            sta = clrd125Write(regAddr, *pData);
        }
        else
        {
            sta = clrd125Read(regAddr, pData);
        }
        if (STA_OK == sta)
        {
            break;
        }
    }
    return sta;
}

// static ERR_STA clrd125ConfigInputEn(void)
// {
//     uint8_t data = CLRD125_INPUT_EN_VAL;  // 宏从表格提取
//     return clrd125I2cHandler(CLRD125_REG_INPUT_EN, &data, 1);
// }

/* 启用扇出 */
// 对地址79（0x4F）写入0xFC，表示使能通道、启用扇出（合并操作）
static ERR_STA clrd125ConfigInputEn(void)
{
    uint8_t data = 0;
    // 配置input_en<1:0>=11（地址79的bit1~bit0），启用1:2扇出
    data = CLRD125_INPUT_EN_VAL;  // 0x03
    return clrd125I2cHandler(0x79, &data, 1);  // 直接操作地址79的input_en位
}

// static ERR_STA clrd125ConfigSel0Sel1(void)
// {
//     uint8_t data = 0;
//     ERR_STA sta = STA_ERR;

//     data = CLRD125_SEL0_VAL;  // 宏从表格提取
//     sta = clrd125I2cHandler(CLRD125_REG_SEL0, &data, 1);
//     if (STA_OK != sta) return sta;

//     data = CLRD125_SEL1_VAL;  // 宏从表格提取
//     sta = clrd125I2cHandler(CLRD125_REG_SEL1, &data, 1);
//     return sta;
// }

/* 修改支持双通道 */
static ERR_STA clrd125ConfigSel0Sel1(void)
{
    uint8_t data = 0;
    ERR_STA sta = STA_ERR;

    // 配置SEL0（通道0：S_INA0±→S_OUTA0±）
    data = CLRD125_SEL0_VAL;  // 0x03（sel0<1:0>=11）
    sta = clrd125I2cHandler(0x79, &data, 1);  // 直接操作地址79的sel0位
    if (STA_OK != sta) return sta;

    // 配置SEL1（通道1：S_INA1±→S_OUTA1±）
    data = (CLRD125_SEL1_VAL << 2);  // sel1<1:0>占地址79的bit3~bit2，左移2位
    sta = clrd125I2cHandler(0x79, &data, 1);
    return sta;
}

// static ERR_STA clrd125ConfigEq(void)
// {
//     uint8_t data = CLRD125_EQ_VAL;  // 宏从表格提取
//     ERR_STA sta = STA_ERR;

//     sta = clrd125I2cHandler(CLRD125_REG_EQ_D0, &data, 1);
//     if (STA_OK != sta) return sta;
//     sta = clrd125I2cHandler(CLRD125_REG_EQ_D1, &data, 1);
//     if (STA_OK != sta) return sta;

//     sta = clrd125I2cHandler(CLRD125_REG_EQ_S0, &data, 1);
//     if (STA_OK != sta) return sta;
//     sta = clrd125I2cHandler(CLRD125_REG_EQ_S1, &data, 1);
//     return sta;
// }

// static ERR_STA clrd125ConfigDeempSwing(void)
// {
//     uint8_t data = 0;
//     ERR_STA sta = STA_ERR;

//     data = CLRD125_VOD_VAL;  // 宏从表格提取
//     sta = clrd125I2cHandler(CLRD125_REG_VOD, &data, 1);
//     if (STA_OK != sta) return sta;

//     data = CLRD125_DEEMP_VAL;  // 宏从表格提取
//     sta = clrd125I2cHandler(CLRD125_REG_DEM_D0, &data, 1);
//     if (STA_OK != sta) return sta;
//     sta = clrd125I2cHandler(CLRD125_REG_DEM_D1, &data, 1);
//     if (STA_OK != sta) return sta;

//     sta = clrd125I2cHandler(CLRD125_REG_DEM_S0, &data, 1);
//     if (STA_OK != sta) return sta;
//     sta = clrd125I2cHandler(CLRD125_REG_DEM_S1, &data, 1);
//     return sta;
// }

// 两通道一样：通道1地址15，通道2地址22，写入的值皆为0x03
// 均衡配置：按通道配置ch0/ch1，模拟LT1258的四通道独立调理
static ERR_STA clrd125ConfigEq(void)
{
    uint8_t data = CLRD125_EQ_VAL;  // 0x0F（30dB）
    ERR_STA sta = STA_ERR;

    // 通道0均衡（ctle_ctrl_ch0，地址15）
    sta = clrd125I2cHandler(0x0F, &data, 1);  // 地址15=ctle_ctrl_ch0
    if (STA_OK != sta) return sta;
    // 通道1均衡（ctle_ctrl_ch1，地址22）
    sta = clrd125I2cHandler(0x16, &data, 1);  // 地址22=ctle_ctrl_ch1
    return sta;
}

// 对地址24、地址38写入0x01（-3.5dB），对地址23、地址37写入0x04（1.0V）
// 去加重+摆幅配置（按通道配置）
static ERR_STA clrd125ConfigDeempSwing(void)
{
    uint8_t dem_data = CLRD125_DEEMP_VAL;  // 0x01（-3.5dB）
    uint8_t vod_data = CLRD125_VOD_VAL;    // 0x04（1.0Vp-p）
    ERR_STA sta = STA_ERR;

    // 通道0：去加重（dem_ch0，地址24）+ 摆幅（vod_ch0，地址23）
    sta = clrd125I2cHandler(0x18, &dem_data, 1);  // 地址24=dem_ch1（对应通道0）
    if (STA_OK != sta) return sta;
    sta = clrd125I2cHandler(0x17, &vod_data, 1);  // 地址23=vod_ch1（对应通道0）
    if (STA_OK != sta) return sta;

    // 通道1：去加重（dem_ch1，地址38）+ 摆幅（vod_ch1，地址37）
    sta = clrd125I2cHandler(0x26, &dem_data, 1);  // 地址38=dem_ch3（对应通道1）
    if (STA_OK != sta) return sta;
    sta = clrd125I2cHandler(0x25, &vod_data, 1);  // 地址37=vod_ch3（对应通道1）
    return sta;
}

// 公开接口（保留原逻辑）
ERR_STA clrd125Init(void)
{
    ERR_STA sta = STA_ERR;

    sta = clrd125ConfigInputEn();
    if (STA_OK != sta) return sta;

    sta = clrd125ConfigSel0Sel1();
    if (STA_OK != sta) return sta;

    sta = clrd125ConfigEq();
    if (STA_OK != sta) return sta;

    sta = clrd125ConfigDeempSwing();
    return sta;
}

ERR_STA clrd125Read(uint8_t regAddr, uint8_t *val)
{
    uint8_t buf[1] = {0};
    ERR_STA sta = STA_ERR;

    sta = gpioI2cRead(LT1258_I2C, CLRD125_DEVICE_ADDR, regAddr, buf, 1);
    if (STA_OK == sta)
    {
        *val = buf[0];
    }
    return sta;
}

ERR_STA clrd125Write(uint8_t regAddr, uint8_t val)
{
    uint8_t buf[1] = {0};  
    buf[0] = val & 0xFF;  

    return gpioI2cWrite(LT1258_I2C, CLRD125_DEVICE_ADDR, regAddr, buf, 1) ? STA_ERR : STA_OK;
}

ERR_STA clrd125InitWithRetry(void)
{
    int8_t retry = 3; 
    ERR_STA initSta = STA_ERR;

    while (retry-- > 0)
    {
        initSta = clrd125Init(); 
        if (STA_OK == initSta)
        {
            printf("CLRD125Init success (remaining retry: %d)\r\n", retry);
            break;
        }
        else
        {
            printf("CLRD125Init failed, retry after 100ms (remaining retry: %d)\r\n", retry);
            delay_1ms(100);
        }
    }

    if (STA_ERR == initSta)
    {
        printf("CLRD125Init failed after 3 retries\r\n");
    }

    return initSta;
}

