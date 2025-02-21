#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>


#define SLEEP_TIME_MS 3000

int main(void) {
    const struct device *dev = DEVICE_DT_GET_ANY(bosch_bme680);

    if (!device_is_ready(dev)) {
        printf("BME680 device is not ready\n");
        return;
    }

    while (1) {
        struct sensor_value temp;

        if (sensor_sample_fetch(dev) < 0) {
            printf("Failed to fetch BME680 sample\n");
            continue;
        }

        if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
            printf("Failed to get temperature data\n");
            continue;
        }

        printf("Temperature: %d.%d°C\n", temp.val1, temp.val2);

        k_msleep(SLEEP_TIME_MS);
    }
}