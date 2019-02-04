
#define _SENSOR_HCSR04
#include <device.h>
#include <i2c.h>
#include <gpio.h>
#include <misc/byteorder.h>
#include <misc/util.h>
#include <kernel.h>
#include <sensor.h>
#include <misc/__assert.h>
#include <logging/sys_log.h>

struct hcsr04_data {
	
	struct gpio_callback input_pin_callback;
	struct device *input_gpio_pinmux;
	struct device *input_pin_gpio_device;
    struct device *output_pin_gpio_device;
    struct device *output_pin_gpio_pinmux;
    struct device *dev_pointer;
    u32_t distance;
    u32_t end_time;
};

#define EDGE    (GPIO_INT_EDGE|GPIO_INT_ACTIVE_HIGH)
