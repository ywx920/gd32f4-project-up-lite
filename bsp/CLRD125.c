#include "CLRD125.h"
#include "LT1258.h"  // 引用LT1258的I2C底层（复用I2C硬件）
#include <string.h>

// ************************** 静态辅助函数（CLRD125） **************************
/**
 * @brief  CLRD125 I2C读写封装
 * @param  regAddr：CLRD125寄存器地址
 * @param  pData：数据缓冲区
 * @param  isWrite：1=写，0=读
 * @retval 状态：STA_OK/STA_ERR
 */
static ERR_STA clrd125I2cHandler(uint8_t regAddr, uint8_t *pData, uint8_t isWrite)
{
    uint8_t retry = RETRY_CNT;  // 复用LT1258的RETRY_CNT宏
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

/**
 * @brief  配置CLRD125 INPUT_EN（工作模式：扇出使能/禁用）
 * @param  无
 * @retval 状态：STA_OK/STA_ERR
 */
static ERR_STA clrd125ConfigInputEn(void)
{
    uint8_t data = CLRD125_INPUT_EN_VAL;
    return clrd125I2cHandler(CLRD125_REG_INPUT_EN, &data, 1);  // 写使能值
}

/**
 * @brief  配置CLRD125 SEL0/SEL1（通道路径选择）
 * @param  无
 * @retval 状态：STA_OK/STA_ERR
 */
static ERR_STA clrd125ConfigSel0Sel1(void)
{
    uint8_t data = 0;
    ERR_STA sta = STA_ERR;

    // 配置SEL0（通道0路径）
    data = CLRD125_SEL0_VAL;
    sta = clrd125I2cHandler(CLRD125_REG_SEL0, &data, 1);
    if (STA_OK != sta) return sta;

    // 配置SEL1（通道1路径）
    data = CLRD125_SEL1_VAL;
    sta = clrd125I2cHandler(CLRD125_REG_SEL1, &data, 1);
    return sta;
}

/**
 * @brief  配置CLRD125所有通道均衡（D侧+S侧）
 * @param  无
 * @retval 状态：STA_OK/STA_ERR
 */
static ERR_STA clrd125ConfigEq(void)
{
    uint8_t data = CLRD125_EQ_VAL;
    ERR_STA sta = STA_ERR;

    // 1. 配置D侧均衡（D_IN0±/D_IN1±，对应EQ_D0/EQ_D1）
    sta = clrd125I2cHandler(CLRD125_REG_EQ_D0, &data, 1);
    if (STA_OK != sta) return sta;
    sta = clrd125I2cHandler(CLRD125_REG_EQ_D1, &data, 1);
    if (STA_OK != sta) return sta;

    // 2. 配置S侧均衡（S_INA0±/S_INA1±，对应EQ_S0/EQ_S1）
    sta = clrd125I2cHandler(CLRD125_REG_EQ_S0, &data, 1);
    if (STA_OK != sta) return sta;
    sta = clrd125I2cHandler(CLRD125_REG_EQ_S1, &data, 1);
    return sta;
}

/**
 * @brief  配置CLRD125所有通道去加重+输出摆幅
 * @param  无
 * @retval 状态：STA_OK/STA_ERR
 */
static ERR_STA clrd125ConfigDeempSwing(void)
{
    uint8_t data = 0;
    ERR_STA sta = STA_ERR;

    // 1. 配置输出摆幅（全局控制，600~1300mVp-p）
    data = CLRD125_VOD_VAL;
    sta = clrd125I2cHandler(CLRD125_REG_VOD, &data, 1);
    if (STA_OK != sta) return sta;

    // 2. 配置D侧去加重（D_OUT0±/D_OUT1±，对应DEM_D0/DEM_D1）
    data = CLRD125_DEEMP_VAL;
    sta = clrd125I2cHandler(CLRD125_REG_DEM_D0, &data, 1);
    if (STA_OK != sta) return sta;
    sta = clrd125I2cHandler(CLRD125_REG_DEM_D1, &data, 1);
    if (STA_OK != sta) return sta;

    // 3. 配置S侧去加重（S_OUTA0±/S_OUTA1±，对应DEM_S0/DEM_S1）
    sta = clrd125I2cHandler(CLRD125_REG_DEM_S0, &data, 1);
    if (STA_OK != sta) return sta;
    sta = clrd125I2cHandler(CLRD125_REG_DEM_S1, &data, 1);
    return sta;
}

// ************************** 公开接口（CLRD125） **************************
/**
 * @brief  CLRD125初始化
 * @param  无
 * @retval 状态：STA_OK/STA_ERR
 */
ERR_STA clrd125Init(void)
{
    ERR_STA sta = STA_ERR;

    // 初始化流程：INPUT_EN→SEL0/SEL1→均衡→去加重+摆幅（基于CLRD125功能优先级）
    sta = clrd125ConfigInputEn();
    if (STA_OK != sta) return sta;

    sta = clrd125ConfigSel0Sel1();
    if (STA_OK != sta) return sta;

    sta = clrd125ConfigEq();
    if (STA_OK != sta) return sta;

    sta = clrd125ConfigDeempSwing();
    return sta;
}

/**
 * @brief  CLRD125寄存器读（复用LT1258的I2C底层，仅指定CLRD125地址）
 * @param  regAddr：CLRD125寄存器地址
 * @param  val：读取到的数据
 * @retval 状态：STA_OK/STA_ERR
 */
ERR_STA clrd125Read(uint8_t regAddr, uint8_t *val)
{
    uint8_t buf[1] = {0};
    ERR_STA sta = STA_ERR;

    // 复用LT1258的gpioI2cRead函数
    sta = gpioI2cRead(LT1258_I2C, CLRD125_DEVICE_ADDR, regAddr, buf, 1);
    if (STA_OK == sta)
    {
        *val = buf[0];
    }
    return sta;
}

/**
 * @brief  CLRD125寄存器写（复用LT1258的I2C底层，仅指定CLRD125地址）
 * @param  regAddr：CLRD125寄存器地址
 * @param  val：要写入的数据
 * @retval 状态：STA_OK/STA_ERR
 */
ERR_STA clrd125Write(uint8_t regAddr, uint8_t val)
{
    // 1. 先定义空数组（常量初始化），再赋值8位数据（符合SMBus 8位格式）
    uint8_t buf[1] = {0};  
    buf[0] = val & 0xFF;  // 变量赋值合法，且确保8位数据（SMBus特性）

    // 2. 复用LT1258的I2C写函数（硬件一致），地址为CLRD125的SMBus从地址
    return gpioI2cWrite(LT1258_I2C, CLRD125_DEVICE_ADDR, regAddr, buf, 1) ? STA_ERR : STA_OK;
}

/**
 * @brief  CLRD125 带重试的初始化（3次重试，失败延时100ms）
 * @note   基于 CLRD125 特性：无BANK切换，仅初始化1片被替换器件，失败后重试避免偶发I2C通信问题
 * @param  无
 * @retval ERR_STA：STA_OK（初始化成功）/STA_ERR（3次重试后仍失败）
 */
ERR_STA clrd125InitWithRetry(void)
{
    int8_t retry = 3; // 最多重试3次
    ERR_STA initSta = STA_ERR;

    // 循环重试：每次失败延时100ms后重新初始化
    while (retry-- > 0)
    {
        initSta = clrd125Init(); // 调用之前实现的 CLRD125 基础初始化函数（配置INPUT_EN、EQ、去加重等）

        if (STA_OK == initSta)
        {
            printf("CLRD125Init success (remaining retry: %d)\r\n", retry);
            break; // 初始化成功，退出重试循环
        }
        else
        {
            printf("CLRD125Init failed, retry after 100ms (remaining retry: %d)\r\n", retry);
            delay_1ms(100); // 失败后延时100ms，避免频繁重试导致I2C总线拥堵
        }
    }

    // 3次重试后仍失败，打印最终错误信息
    if (STA_ERR == initSta)
    {
        printf("CLRD125Init failed after 3 retries\r\n");
    }

    return initSta;
}

