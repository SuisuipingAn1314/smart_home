#include "main.h"
#include "i2c.h"
#include "sht3x.h"

// ==========================================================
// 硬件配置：你的 CBT6 板子上 DHT11 接在 PA8
// ==========================================================
#define DHT11_PORT GPIOA
#define DHT11_PIN  GPIO_PIN_8

// 微秒级延时函数 (基于 SysTick)
static void delay_us(uint32_t us)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;

    ticks = us * (SystemCoreClock / 1000000);
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told) tcnt += told - tnow;
            else tcnt += reload - tnow + told;
            told = tnow;
            if (tcnt >= ticks) break;
        }
    }
}

// 配置 PA8 为输出
static void DHT11_Set_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

// 配置 PA8 为输入
static void DHT11_Set_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

// SHT3X 初始化 (直接复用为 DHT11 初始化)
void SHT3X_Init(void)
{
    DHT11_Set_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
}

// 控制传感器采集温湿度数据，采集成功返回 0，失败返回非零值
uint8_t SHT3X_Get(float* temp, float* humi)
{
    uint8_t buf[5] = {0};
    uint8_t i, j;

    // 1. 主机发送开始信号
    DHT11_Set_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(20); // 拉低至少 18ms
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    delay_us(30);
    DHT11_Set_Input();

    // 2. 检测 DHT11 响应
    if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET)
    {
        // 等待 DHT11 拉低 80us 结束
        while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET); 
        // 等待 DHT11 拉高 80us 结束
        while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET);   

        // 3. 读取 40 位数据
        for (i = 0; i < 5; i++)
        {
            for (j = 0; j < 8; j++)
            {
                // 等待 50us 低电平结束
                while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET); 
                delay_us(40); // 延时 40us 判断高低电平
                
                if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
                {
                    buf[i] |= (1 << (7 - j));
                    // 等待高电平结束
                    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET); 
                }
            }
        }

        // 4. 校验数据
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            if(humi != NULL) *humi = (float)buf[0] + (float)buf[1] / 10.0;
            if(temp != NULL) *temp = (float)buf[2] + (float)buf[3] / 10.0;
            return 0; // 成功
        }
    }
    return 1; // 失败
}