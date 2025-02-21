#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <stdio.h>

#include <bme680_reg.h>

#define BME680_ADDR 0x77
#define I2C_LABEL i2c0

#define SLEEP_TIME_MS 3000

uint32_t read_temp(const struct device *i2c_bus)
{
    uint8_t raw_msb, raw_lsb, raw_xlsb;
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_MSB, &raw_msb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_LSB, &raw_lsb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_XLSB, &raw_xlsb);
    
    printf("Raw Temp Registers: %02X %02X %02X\n", raw_msb, raw_lsb, raw_xlsb);

    uint32_t temp_adc = ((uint32_t)raw_msb << 12) | ((uint32_t)raw_lsb << 4) | (raw_xlsb >> 4);
    printf("temp_adc: %u\n", temp_adc);

    return temp_adc;
}

void get_par12(const struct device *i2c_bus, uint16_t *par_t1, uint16_t *par_t2)
{
    uint8_t par_t1_lsb, par_t1_msb;
    uint8_t par_t2_lsb, par_t2_msb;

    i2c_reg_read_byte(i2c_bus, BME680_ADDR, 0xE9, &par_t1_lsb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, 0xEA, &par_t1_msb);
    *par_t1 = ((uint16_t)par_t1_msb << 8) | par_t1_lsb;

    i2c_reg_read_byte(i2c_bus, BME680_ADDR, 0x8A, &par_t2_lsb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, 0x8B, &par_t2_msb);
    *par_t2 = ((uint16_t)par_t2_msb << 8) | par_t2_lsb;
}

int main(void)
{
    const struct device *i2c_bus = DEVICE_DT_GET(DT_NODELABEL(I2C_LABEL));

    if (!device_is_ready(i2c_bus))
    {
        printf("I2C device not ready\n");
        return -1;
    }

    /* Read chip ID */
    uint8_t id;
    if (i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_ID, &id) < 0)
    {
        printf("Could not communicate with sensor.\n");
        return -1;
    }

    if (id != 0x61)
    {
        printf("Invalid sensor ID: 0x%X\n", id);
        return -1;
    }

    while (1)
    {
        // Set forced mode for a new measurement
        i2c_reg_write_byte(i2c_bus, BME680_ADDR, BME680_CTRL_MEAS, 0x21); // Forced mode, x1 oversampling
        k_msleep(10); 

        uint32_t temp_adc = read_temp(i2c_bus);

        uint16_t par_t1, par_t2;
        get_par12(i2c_bus, &par_t1, &par_t2);

        uint8_t par_t3;
        i2c_reg_read_byte(i2c_bus, BME680_ADDR, 0x8C, &par_t3);

        // Compensate temperature
        double var1, var2, t_fine, temp_comp;
        var1 = (((double)temp_adc / 16384.0) - ((double)par_t1 / 1024.0)) * (double)par_t2;
        var2 = ((((double)temp_adc / 131072.0) - ((double)par_t1 / 8192.0)) *
                (((double)temp_adc / 131072.0) - ((double)par_t1 / 8192.0))) * ((double)par_t3 * 16.0);
        t_fine = var1 + var2;
        temp_comp = t_fine / 5120.0;

        printf("temp_comp: %.2f °C\n", temp_comp);

        k_msleep(SLEEP_TIME_MS);
    }
}
