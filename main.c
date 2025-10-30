/*!
    \file    main.c
    \brief   with systick
*/

#include "gd32f4xx.h"
#include "systick.h"
#include "bsp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Version_MSG.h"
#include "serial.h"
#include "GN1406.h"
#include "LT1258.h"
#include "CLRD125.h"

// 任务周期宏定义（基于系统滴答定时器）
#define LED_TASK_CYCLE (SYS_CLK_TICK / 4)  // 0.25s
#define TEST_TASK_CYCLE (SYS_CLK_TICK * 1) // 1s
#define MAIN_LOOP_INTERVAL 10              // 主循环间隔(ms)，避免CPU空转

// 状态帧结构定义
typedef struct
{
    uint8_t head;       // 帧头：0x7E
    uint8_t type;       // 类型：0xF1
    uint8_t time_h;     // 时间高位
    uint8_t time_l;     // 时间低位
    uint8_t los_state;  // LOS状态
    uint8_t ch1_status; // 通道1状态
    uint8_t ch1_val;
    uint8_t ch2_status; // 通道2状态
    uint8_t ch2_val;
    uint8_t ch3_status; // 通道3状态
    uint8_t ch3_val;
    uint8_t ch4_status; // 通道4状态
    uint8_t ch4_val;
    uint8_t temp1;         // 温度1
    uint8_t temp2;         // 温度2
    uint8_t temp_internal; // 内部温度
    uint8_t reserved[14];  // 预留填充（确保总长度31字节，加校验和共32）
    uint8_t checksum;      // 校验和
} StatusFrame;

// 全局变量
static uint64_t g_led_last_tick = 0;  // LED任务计时
static uint64_t g_test_last_tick = 0; // 测试任务计时

/************************** 工具函数优化 **************************/
// 通用：将8位整数转为二进制字符串并打印
static void dec2bin_common(uint8_t n, const char *prefix, uint8_t min_digits)
{
    int result = 0, k = 1, i, temp = n;
    while (temp)
    {
        i = temp % 2;
        result = k * i + result;
        k *= 10;
        temp /= 2;
    }
    printf("%s:%0*d\r\n", prefix, min_digits, result);
}

// 保留原函数名，内部调用通用函数
void Dec2Bin(uint8_t n)
{
    dec2bin_common(n, "read_los SFP_LOS", 8);
}
void Dec2Bin_RA(uint8_t n)
{
    dec2bin_common(n, "The Read_RA val", 4);
}
void Dec2Bin_GA(uint8_t n)
{
    dec2bin_common(n, "The Read_GA val", 6);
}
void Dec2Bin_LOS(uint8_t n)
{
    dec2bin_common(n, "The LOS_val", 8);
}

/************************** 硬件操作封装 **************************/
// LED控制封装
static void led_control(FLED_NUM led, uint8_t state)
{
    fLedCtl(led, state);
}

// 传感器读取封装
static ERR_STA sensor_read_temp(uint8_t *temp)
{
    ERR_STA sta;
    sta = ReadTemp((char *)temp); // 强制类型转换
    if (STA_ERR == sta)
    {
        temp[0] = temp[1] = temp[2] = 0xFF;
        printf("ReadCB2991Temp fail\r\n");
    }
    return sta;
}

static ERR_STA sensor_read_vol(uint16_t *vol)
{
    ERR_STA sta = ReadVol(vol);
    if (STA_ERR == sta)
    {
        vol[0] = vol[1] = vol[2] = vol[3] = 0xFFFF;
        printf("ReadCB2991vol fail\r\n");
    }
    return sta;
}

// 串口发送封装
static void uart_send(uint32_t usart, uint8_t *data, uint16_t len)
{
    serialSend(usart, data, len);
}

/************************** 任务拆分 **************************/
// 处理LOS状态与LED控制（testTask）
static void process_los_status(uint8_t los_val, StatusFrame *frame)
{
    // 通道1状态（对应LED_FL3）
    if ((los_val & 0x03) == 0x00)
    {
        led_control(LED_FL3, LED_GREEN);
        frame->ch1_status = 0x01;
        frame->ch1_val = 0xE1;
    }
    else
    {
        led_control(LED_FL3, LED_RED);
        frame->ch1_status = 0x01;
        frame->ch1_val = 0xE0;
    }

    // 通道2状态（对应LED_FL4）
    if ((los_val & 0x0C) == 0x00)
    {
        led_control(LED_FL4, LED_GREEN);
        frame->ch2_status = 0x01;
        frame->ch2_val = 0xE1;
    }
    else
    {
        led_control(LED_FL4, LED_RED);
        frame->ch2_status = 0x01;
        frame->ch2_val = 0xE0;
    }

    // 通道3状态（对应LED_FL5）
    if ((los_val & 0x30) == 0x00)
    {
        led_control(LED_FL5, LED_GREEN);
        frame->ch3_status = 0x01;
        frame->ch3_val = 0xE1;
    }
    else
    {
        led_control(LED_FL5, LED_RED);
        frame->ch3_status = 0x01;
        frame->ch3_val = 0xE0;
    }

    // 通道4状态（对应LED_FL6）
    if ((los_val & 0xC0) == 0x00)
    {
        led_control(LED_FL6, LED_GREEN);
        frame->ch4_status = 0x01;
        frame->ch4_val = 0xE1;
    }
    else
    {
        led_control(LED_FL6, LED_RED);
        frame->ch4_status = 0x01;
        frame->ch4_val = 0xE0;
    }
}

// 构建状态报告帧（testTask）
static void build_status_frame(StatusFrame *frame, uint8_t los_val, uint8_t *temp)
{
    uint8_t *frame_ptr;
    uint8_t i;

    frame->head = 0x7E;
    frame->type = 0xF1;
    frame->time_h = (sec >> 8) & 0xFF;
    frame->time_l = sec & 0xFF;
    frame->los_state = los_val & 0xFF;

    frame->temp1 = temp[0];
    frame->temp2 = temp[1];
    frame->temp_internal = temp[2];

    memset(frame->reserved, 0x00, sizeof(frame->reserved));

    frame->checksum = 0;
    frame_ptr = (uint8_t *)frame;
    for (i = 1; i < sizeof(StatusFrame) - 1; i++)
    {
        frame->checksum += frame_ptr[i];
        frame->checksum &= 0xFF;
    }
}

/************************** 任务函数 **************************/
void ledTask(void)
{
    if (systick - g_led_last_tick > LED_TASK_CYCLE)
    {
        g_led_last_tick = systick;
        dbgLedCtl(LED_D0, LED_TOG);
    }
}

void testTask(void)
{
    uint8_t los_val;
    uint8_t temp[3];
    uint16_t vol[4];
    StatusFrame frame;

    if (systick - g_test_last_tick > TEST_TASK_CYCLE)
    {
        g_test_last_tick = systick;
        sec = systick / 1000;

        los_val = read_los();
        Dec2Bin_LOS(los_val);

        memset(temp, 0, sizeof(temp));
        sensor_read_temp(temp);
        memset(vol, 0, sizeof(vol));
        sensor_read_vol(vol);
        printf("\r\nReadCB2991Temp:%d %d %d\r\n", temp[0], temp[1], temp[2]);
        printf("ReadCB2991vol:%dmv %dmv %dmv %dmv \r\n", vol[0], vol[1], vol[2], vol[3]);

        memset(&frame, 0, sizeof(StatusFrame));
        process_los_status(los_val, &frame);
        build_status_frame(&frame, los_val, temp);

        uart_send(USART2, (uint8_t *)&frame, sizeof(StatusFrame));
    }
}

void BBXX_ouput(void)
{
    printf("\r\nZX2103 UpperTerminalARMV%d.%02d.%03d,%s,%s\r\n",
           VERSION_A, VERSION_CD, VERSION_EFG, __DATE__, __TIME__);
}

/************************** 初始化与主函数 **************************/
static void hardware_init(void)
{
    int8_t retry; 
    ERR_STA sta = STA_OK;

    systick_config();
    rcu_config();
    bspGpioInit();
    other_io_init();
    uartInit();
    BBXX_ouput();

    // LT1258 初始化
    retry = 3;
    while (retry-- && STA_ERR == lt1258Init())
    {
        delay_1ms(100);
    }
    if (retry < 0)
    {
        printf("LT1258Init failed after 3 retries\r\n");
    }

    // CLRD125 初始化
    if (STA_OK == sta) // 在 LT1258 初始化成功后，再初始化 CLRD125（避免总线冲突）
    {
        sta = clrd125InitWithRetry();
    }

    if (STA_OK == sta)
    {
        printf("All devices (3x LT1258 + 1x CLRD125) init success\r\n");
    }
    else
    {
        printf("Some device init failed\r\n");
    }

    // 温度传感器初始化
    retry = 3;
    while (retry-- && STA_ERR == cb2991TempInit())
    {
        delay_1ms(100);
    }
    if (retry < 0)
    {
        printf("cb2991TempInit failed after 3 retries\r\n");
    }
}

int main(void)
{
    uint8_t RA_val;
    uint8_t GA_val;

    hardware_init();

    while (1)
    {
        RA_val = read_RA();
        Dec2Bin_RA(RA_val);
        GA_val = read_GA();
        Dec2Bin_GA(GA_val);

        ledTask();
        testTask();

        delay_1ms(MAIN_LOOP_INTERVAL);
    }
}
