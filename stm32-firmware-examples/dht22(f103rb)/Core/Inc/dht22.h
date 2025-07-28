
#ifndef DHT22_H
#define DHT22_H

#include "stm32f1xx_it.h"
#include "stm32f1xx_hal.h"

#define DHT22_PORT GPIOA
#define DHT22_PIN GPIO_PIN_1


typedef struct {
    float temperature;
    float humidity;
} DHT22_Data;

uint8_t DHT22_Start(void);
uint8_t DHT22_Read(DHT22_Data *data);


#endif // DHT22_H