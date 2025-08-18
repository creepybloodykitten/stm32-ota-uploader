#include "bme280_interface.h"

// Microsecond delay function
// HAL_Delay has 1 ms accuracy, which is sufficient for most tasks.
// For the BME280, this accuracy is enough.
void user_delay_us(uint32_t period, void *intf_ptr) {
    HAL_Delay(period / 1000);
}

// I2C read function (new signature)
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    // Retrieve our structure from the generic pointer
    struct bme280_intf_link *dev_link = (struct bme280_intf_link *)intf_ptr;

    // Use dev_id and handle from our structure
    if (HAL_I2C_Mem_Read(dev_link->handle, (dev_link->dev_id << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, len, 1000) == HAL_OK) {
        return BME280_OK;
    } else {
        return BME280_E_COMM_FAIL;
    }
}

// I2C write function (new signature)
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) 
	{
    struct bme280_intf_link *dev_link = (struct bme280_intf_link *)intf_ptr;
    if (HAL_I2C_Mem_Write(dev_link->handle, (dev_link->dev_id << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)reg_data, len, 1000) == HAL_OK) {
        return BME280_OK;
    } else {
        return BME280_E_COMM_FAIL;
    }
}

// Convenient initializer function
int8_t bme280_interface_init(struct bme280_dev *dev, struct bme280_intf_link *intf) {
    dev->intf = BME280_I2C_INTF;     // Specify that the interface is I2C
    dev->read = user_i2c_read;       // Bind our read function
    dev->write = user_i2c_write;     // Bind our write function
    dev->delay_us = user_delay_us;   // Bind our delay function (in microseconds!)

    // Store pointer to our structure so it's accessible in read/write
    dev->intf_ptr = intf;

    // Initialize the sensor itself
    int8_t rslt = bme280_init(dev);
    return rslt;
}