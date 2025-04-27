#include "hx711.h"
#include "main.h"

void HX711_Init(HX711 *hx, GPIO_TypeDef* gpioSck, uint16_t pinSck, 
                GPIO_TypeDef* gpioData, uint16_t pinData) {
    hx->gpioSck = gpioSck;
    hx->pinSck = pinSck;
    hx->gpioData = gpioData;
    hx->pinData = pinData;
    hx->offset = 0;
    hx->scale = 1.0f;
    
    HAL_GPIO_WritePin(hx->gpioSck, hx->pinSck, GPIO_PIN_RESET);
}

uint8_t HX711_IsReady(HX711 *hx) {
    return (HAL_GPIO_ReadPin(hx->gpioData, hx->pinData) == GPIO_PIN_RESET);
}

int32_t HX711_Read(HX711 *hx) {
    //等待HX711准备好
    while (!HX711_IsReady(hx));
    
    int32_t data = 0;
    
    //读取24数据
    for (uint8_t i = 0; i < 24; i++) {
        HAL_GPIO_WritePin(hx->gpioSck, hx->pinSck, GPIO_PIN_SET);
        data <<= 1;
        if (HAL_GPIO_ReadPin(hx->gpioData, hx->pinData)) {
            data++;
        }
        HAL_GPIO_WritePin(hx->gpioSck, hx->pinSck, GPIO_PIN_RESET);
    }
    
    // 设置通到和增益(通道A,增益128)
    for (uint8_t i = 0; i < 1; i++) {
        HAL_GPIO_WritePin(hx->gpioSck, hx->pinSck, GPIO_PIN_SET);
        HAL_GPIO_WritePin(hx->gpioSck, hx->pinSck, GPIO_PIN_RESET);
    }
    
    // 扩展符号到32位
    if (data & 0x800000) {
        data |= 0xFF000000;
    }
    
    return data;
}

int32_t HX711_ReadAverage(HX711 *hx, uint8_t times) {
    int64_t sum = 0;
    for (uint8_t i = 0; i < times; i++) {
        sum += HX711_Read(hx);
    }
    return sum / times;
}

float HX711_GetWeight(HX711 *hx, uint8_t times) {
    int32_t val = HX711_ReadAverage(hx, times) - hx->offset;
    return (float)val / hx->scale;
}

void HX711_Tare(HX711 *hx, uint8_t times) {
    int64_t sum = HX711_ReadAverage(hx, times);
    hx->offset = sum;
}

void HX711_SetScale(HX711 *hx, float scale) {
    hx->scale = scale;
}

float HX711_GetScale(HX711 *hx) {
    return hx->scale;
}

void HX711_SetOffset(HX711 *hx, int32_t offset) {
    hx->offset = offset;
}

int32_t HX711_GetOffset(HX711 *hx) {
    return hx->offset;
}

void HX711_PowerDown(HX711 *hx) {
    HAL_GPIO_WritePin(hx->gpioSck, hx->pinSck, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hx->gpioSck, hx->pinSck, GPIO_PIN_SET);
}

void HX711_PowerUp(HX711 *hx) {
    HAL_GPIO_WritePin(hx->gpioSck, hx->pinSck, GPIO_PIN_RESET);
}