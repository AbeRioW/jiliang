#ifndef HX711_H
#define HX711_H

#include "stm32f1xx_hal.h"

typedef struct {
    GPIO_TypeDef* gpioSck;
    uint16_t pinSck;
    GPIO_TypeDef* gpioData;
    uint16_t pinData;
    int32_t offset;
    float scale;
} HX711;

void HX711_Init(HX711 *hx, GPIO_TypeDef* gpioSck, uint16_t pinSck, 
                GPIO_TypeDef* gpioData, uint16_t pinData);
uint8_t HX711_IsReady(HX711 *hx);
int32_t HX711_Read(HX711 *hx);
int32_t HX711_ReadAverage(HX711 *hx, uint8_t times);
float HX711_GetWeight(HX711 *hx, uint8_t times);
void HX711_Tare(HX711 *hx, uint8_t times);
void HX711_SetScale(HX711 *hx, float scale);
float HX711_GetScale(HX711 *hx);
void HX711_SetOffset(HX711 *hx, int32_t offset);
int32_t HX711_GetOffset(HX711 *hx);
void HX711_PowerDown(HX711 *hx);
void HX711_PowerUp(HX711 *hx);

#endif