#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <stdio.h>

#include <bme680_reg.h>

#define BME680_ADDR     0x77

#define I2C_LABEL       i2c0

int main(void)
{
    const struct device *i2c_bus = DEVICE_DT_GET(DT_NODELABEL(I2C_LABEL));

    if (i2c_bus == NULL)
    {
        printf("No device found; did initialization fail?\n");
        return -1;
    } 
    
    /* Try and read chip id */
    uint8_t id;
    if (i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_ID, &id) < 0){
        printf("Could not communicate with sensor.\n"); 
        return -1; 
    }
    
    if (id != 0x61)
    {
        printf("Sensor ID could not be read from I2C device.\n");
        return -1;
    }

}
