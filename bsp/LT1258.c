#include "gd32f4xx.h"
#include "LT1258.h"
#include <string.h>
#include "stdio.h"



// ************************** 静态辅助函数 **************************
static ERR_STA i2cReadWriteHandler(uint8_t deviceAddr, uint8_t regAddr, uint8_t *pData, uint8_t isWrite)
{
    uint8_t retry;  // C89: 变量声明移到块开头
    ERR_STA sta = STA_ERR;
    retry = RETRY_CNT;

    while (retry--)
    {
        if (isWrite)
        {
            sta = lt1258Write(deviceAddr, regAddr, *pData);
        }
        else
        {
            sta = lt1258Read(deviceAddr, regAddr, pData);
        }

        if (STA_OK == sta)
        {
            break;
        }
    }

    return sta;
}

static ERR_STA switchChipBank(uint8_t deviceAddr, uint8_t bankVal)
{
    return i2cReadWriteHandler(deviceAddr, REG_BANK_SEL, &bankVal, 1);
}

static ERR_STA configInen(uint8_t deviceAddr)
{
    uint8_t data = 0;
    ERR_STA sta = STA_ERR;

    sta = i2cReadWriteHandler(deviceAddr, REG_INEN_SEL_EN, &data, 0);
    if (STA_OK != sta) return sta;

    data |= BIT_INEN_EN;
    sta = i2cReadWriteHandler(deviceAddr, REG_INEN_SEL_EN, &data, 1);
    if (STA_OK != sta) return sta;

    sta = i2cReadWriteHandler(deviceAddr, REG_CH_PARAM, &data, 0);
    if (STA_OK != sta) return sta;

    data = (data & ~BIT5) | BIT4;
    sta = i2cReadWriteHandler(deviceAddr, REG_CH_PARAM, &data, 1);

    return sta;
}

static ERR_STA configSel0Sel1(uint8_t deviceAddr)
{
    uint8_t data = 0;
    ERR_STA sta = STA_ERR;

    sta = i2cReadWriteHandler(deviceAddr, REG_INEN_SEL_EN, &data, 0);
    if (STA_OK != sta) return sta;
    data |= BIT_SEL0_EN;
    sta = i2cReadWriteHandler(deviceAddr, REG_INEN_SEL_EN, &data, 1);
    if (STA_OK != sta) return sta;

    sta = i2cReadWriteHandler(deviceAddr, REG_CH_PARAM, &data, 0);
    if (STA_OK != sta) return sta;
    data = (data & ~BIT1) | BIT0;
    sta = i2cReadWriteHandler(deviceAddr, REG_CH_PARAM, &data, 1);
    if (STA_OK != sta) return sta;

    sta = i2cReadWriteHandler(deviceAddr, REG_INEN_SEL_EN, &data, 0);
    if (STA_OK != sta) return sta;
    data |= BIT_SEL1_EN;
    sta = i2cReadWriteHandler(deviceAddr, REG_INEN_SEL_EN, &data, 1);
    if (STA_OK != sta) return sta;

    sta = i2cReadWriteHandler(deviceAddr, REG_CH_PARAM, &data, 0);
    if (STA_OK != sta) return sta;
    data = (data & ~BIT3) | BIT2;
    sta = i2cReadWriteHandler(deviceAddr, REG_CH_PARAM, &data, 1);

    return sta;
}

static ERR_STA configEqAllCh(uint8_t deviceAddr)
{
    uint8_t data = EQ_CTRL_EN_VAL;
    uint8_t eqOffsets[] = {CH_EQ_OFFSET_0, CH_EQ_OFFSET_1, CH_EQ_OFFSET_2, CH_EQ_OFFSET_3};
    uint8_t chNum = sizeof(eqOffsets)/sizeof(eqOffsets[0]);
    ERR_STA sta = STA_ERR;
    uint8_t i;  // C89: 循环变量在循环外声明

    for (i = 0; i < chNum; i++)
    {
        sta = i2cReadWriteHandler(deviceAddr, REG_EQ_CTRL_BASE + eqOffsets[i], &data, 1);
        if (STA_OK != sta) return sta;

        data = EQ_PARAM_VAL;
        sta = i2cReadWriteHandler(deviceAddr, REG_EQ_PARAM_BASE + eqOffsets[i], &data, 1);
        if (STA_OK != sta) return sta;

        data = EQ_CTRL_EN_VAL;
    }

    return sta;
}

static ERR_STA configDeempSwingAllCh(uint8_t deviceAddr)
{
    uint8_t data = DEEMP_CTRL_EN_VAL;
    uint8_t deempOffsets[] = {CH_EQ_OFFSET_0, CH_EQ_OFFSET_1, CH_EQ_OFFSET_2, CH_EQ_OFFSET_3};
    uint8_t chNum = sizeof(deempOffsets)/sizeof(deempOffsets[0]);
    ERR_STA sta = STA_ERR;
    uint8_t i;  // C89: 循环变量在循环外声明

    for (i = 0; i < chNum; i++)
    {
        sta = i2cReadWriteHandler(deviceAddr, REG_DEEMP_CTRL_BASE + deempOffsets[i], &data, 1);
        if (STA_OK != sta) return sta;

        data = DEEMP_PARAM_VAL;
        sta = i2cReadWriteHandler(deviceAddr, REG_DEEMP_PARAM_BASE + deempOffsets[i], &data, 1);
        if (STA_OK != sta) return sta;

        data = SWING_PARAM_VAL;
        sta = i2cReadWriteHandler(deviceAddr, REG_SWING_PARAM_BASE + deempOffsets[i], &data, 1);
        if (STA_OK != sta) return sta;

        data = DEEMP_CTRL_EN_VAL;
    }

    return sta;
}

static ERR_STA lt1258SingleChipConfig(uint8_t deviceAddr)
{
    ERR_STA sta = STA_ERR;

    sta = switchChipBank(deviceAddr, BANK_81);
    if (STA_OK != sta) goto config_fail;

    sta = configInen(deviceAddr);
    if (STA_OK != sta) goto config_fail;

    sta = configSel0Sel1(deviceAddr);
    if (STA_OK != sta) goto config_fail;

    sta = switchChipBank(deviceAddr, BANK_80);
    if (STA_OK != sta) goto config_fail;

    sta = configEqAllCh(deviceAddr);
    if (STA_OK != sta) goto config_fail;

    sta = configDeempSwingAllCh(deviceAddr);
    if (STA_OK != sta) goto config_fail;

config_fail:
    return sta;
}

// ************************** 公开接口 **************************
ERR_STA lt1258Init(void)
{
    ERR_STA sta = STA_OK;
    uint8_t deviceAddrs[] = LT1258_DEVICE_ADDRS;
    uint8_t i;  // C89: 循环变量在循环外声明

    gpioI2cInit(LT1258_I2C);

    for (i = 0; i < LT1258_DEVICE_NUM; i++)
    {
        sta |= lt1258SingleChipConfig(deviceAddrs[i]);
    }

    return (STA_OK == sta) ? STA_OK : STA_ERR;
}

ERR_STA lt1258Read(uint8_t deviceAddr, uint8_t regAddr, uint8_t *val)
{
    uint8_t buf[1];  // C89: 数组初始化不使用变量
    ERR_STA sta = STA_ERR;

    memset(buf, 0, sizeof(buf));  // 显式初始化
    sta = gpioI2cRead(LT1258_I2C, deviceAddr, regAddr, buf, 1);
    if (STA_OK == sta)
    {
        *val = buf[0];
    }

    return sta;
}

ERR_STA lt1258Write(uint8_t deviceAddr, uint8_t regAddr, uint8_t val)
{
    uint8_t buf[1];  // C89: 数组初始化不使用变量，改为赋值
    buf[0] = val & 0xFF;  // 赋值代替初始化

    return gpioI2cWrite(LT1258_I2C, deviceAddr, regAddr, buf, 1) ? STA_ERR : STA_OK;
}

