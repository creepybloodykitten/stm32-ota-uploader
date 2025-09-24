#include "dht22.h"

extern TIM_HandleTypeDef htim1; 

static void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < us);
}

static void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

static void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

uint8_t DHT22_Start(void) {
    uint8_t response = 0;
    Set_Pin_Output(DHT22_PORT, DHT22_PIN);
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_RESET);
    delay_us(1200);
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);
    delay_us(30);
    Set_Pin_Input(DHT22_PORT, DHT22_PIN);
    delay_us(40);
    if (!(HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))) {
        delay_us(80);
        if ((HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))) response = 1;
    }
    while ((HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN)));
    return response;
}

uint8_t DHT22_Read(DHT22_Data *data) {
    uint8_t i, j;
    uint8_t raw_data[5] = {0, 0, 0, 0, 0};

    if (!DHT22_Start()) return 0;

    for (j = 0; j < 5; j++) {
        for (i = 0; i < 8; i++) {
            while (!(HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN)));
            delay_us(40);
            if (!(HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))) {
                raw_data[j] &= ~(1 << (7 - i));
            } else {
                raw_data[j] |= (1 << (7 - i));
            }
            while ((HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN)));
        }
    }

    if ((raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3]) == raw_data[4]) {
        data->humidity = ((uint16_t)raw_data[0] << 8 | raw_data[1]) / 10.0f;
        data->temperature = ((uint16_t)raw_data[2] << 8 | raw_data[3]) / 10.0f;
        if (raw_data[2] & 0x80) {
            data->temperature *= -1;
        }
        return 1;
    }

    return 0;
}