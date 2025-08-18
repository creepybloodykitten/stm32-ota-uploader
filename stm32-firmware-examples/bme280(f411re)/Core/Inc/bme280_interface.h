#ifndef BME280_INTERFACE_H_
#define BME280_INTERFACE_H_

#include "stm32f4xx_hal.h" 
#include "bme280.h"

struct bme280_intf_link 
	{
    I2C_HandleTypeDef *handle;
    uint8_t dev_id;
};

void user_delay_us(uint32_t period, void *intf_ptr);
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t user_i2c_write(uint8_t reg_addr,const uint8_t *reg_data, uint32_t len, void *intf_ptr);

int8_t bme280_interface_init(struct bme280_dev *dev, struct bme280_intf_link *intf);

#endif 