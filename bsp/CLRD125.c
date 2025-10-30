#include "CLRD125.h"
#include "LT1258.h"  
#include <string.h>

// ###########################################################################
// 新增：CLRD125配置表格（D[0]~D[79]，用户提供的原始表格值，C文件定义全局数组）
// ###########################################################################
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

// ###########################################################################
// 静态辅助函数（完全保留原逻辑，无任何修改）
// ###########################################################################
static ERR_STA clrd125I2cHandler(uint8_t regAddr, uint8_t *pData, uint8_t isWrite)
{
    uint8_t retry = RETRY_CNT;  // 复用LT1258的RETRY_CNT宏（原逻辑不变）
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

static ERR_STA clrd125ConfigInputEn(void)
{
    uint8_t data = CLRD125_INPUT_EN_VAL;  // 宏已从表格提取，无需改函数
    return clrd125I2cHandler(CLRD125_REG_INPUT_EN, &data, 1);
}

static ERR_STA clrd125ConfigSel0Sel1(void)
{
    uint8_t data = 0;
    ERR_STA sta = STA_ERR;

    data = CLRD125_SEL0_VAL;  // 宏从表格提取
    sta = clrd125I2cHandler(CLRD125_REG_SEL0, &data, 1);
    if (STA_OK != sta) return sta;

    data = CLRD125_SEL1_VAL;  // 宏从表格提取
    sta = clrd125I2cHandler(CLRD125_REG_SEL1, &data, 1);
    return sta;
}

static ERR_STA clrd125ConfigEq(void)
{
    uint8_t data = CLRD125_EQ_VAL;  // 宏从表格提取
    ERR_STA sta = STA_ERR;

    sta = clrd125I2cHandler(CLRD125_REG_EQ_D0, &data, 1);
    if (STA_OK != sta) return sta;
    sta = clrd125I2cHandler(CLRD125_REG_EQ_D1, &data, 1);
    if (STA_OK != sta) return sta;

    sta = clrd125I2cHandler(CLRD125_REG_EQ_S0, &data, 1);
    if (STA_OK != sta) return sta;
    sta = clrd125I2cHandler(CLRD125_REG_EQ_S1, &data, 1);
    return sta;
}

static ERR_STA clrd125ConfigDeempSwing(void)
{
    uint8_t data = 0;
    ERR_STA sta = STA_ERR;

    data = CLRD125_VOD_VAL;  // 宏从表格提取
    sta = clrd125I2cHandler(CLRD125_REG_VOD, &data, 1);
    if (STA_OK != sta) return sta;

    data = CLRD125_DEEMP_VAL;  // 宏从表格提取
    sta = clrd125I2cHandler(CLRD125_REG_DEM_D0, &data, 1);
    if (STA_OK != sta) return sta;
    sta = clrd125I2cHandler(CLRD125_REG_DEM_D1, &data, 1);
    if (STA_OK != sta) return sta;

    sta = clrd125I2cHandler(CLRD125_REG_DEM_S0, &data, 1);
    if (STA_OK != sta) return sta;
    sta = clrd125I2cHandler(CLRD125_REG_DEM_S1, &data, 1);
    return sta;
}

// ###########################################################################
// 公开接口（完全保留原逻辑，无任何修改）
// ###########################################################################
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

