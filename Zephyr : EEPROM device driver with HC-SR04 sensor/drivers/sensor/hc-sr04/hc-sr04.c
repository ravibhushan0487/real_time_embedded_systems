#include <zephyr.h>
#include <board.h>
#include <device.h>
#include <pinmux.h>
#include "pinmux_galileo.h"
#include <gpio.h>
#include <misc/printk.h>
#include <string.h>
#include <pwm.h>
#include <shell/shell.h>
#include <i2c.h>
#include <sensor.h>
#include <flash.h>
#include <sys_io.h>
#include <init.h>
#include <misc/util.h>
#include <misc/__assert.h>
#include <clock_control.h>
#include <misc/util.h>
#include <sys_clock.h>
#include <limits.h>
#include <kernel.h>
#include "hc-sr04.h"

void callback_function_0(struct device *gpiob, struct gpio_callback *cb, u32_t pins)
{
    uint32_t echo_pulse = 0;
    uint32_t start_time, end_time;
    uint32_t pulse_time;
    uint32_t distance;
    struct hcsr04_data *drv_data =CONTAINER_OF(cb, struct hcsr04_data, input_pin_callback);
    start_time = _tsc_read();
    gpio_pin_disable_callback(drv_data->input_pin_gpio_device, 2);
    do
    {
        gpio_pin_read(drv_data->input_pin_gpio_device,2,&echo_pulse);
        end_time = _tsc_read();
    }while(echo_pulse==1);
    pulse_time = end_time - start_time;
    distance = (end_time - start_time)/(400*58);
    drv_data->distance = distance;
    drv_data->end_time = end_time;
}

void callback_function_1(struct device *gpiob, struct gpio_callback *cb, u32_t pins)
{
    uint32_t echo_pulse = 0;
    uint64_t start_time, end_time;
    uint32_t pulse_time;
    uint32_t distance;
    struct hcsr04_data *drv_data =CONTAINER_OF(cb, struct hcsr04_data, input_pin_callback);
    start_time = _tsc_read();
    gpio_pin_disable_callback(drv_data->input_pin_gpio_device, 7);
    do
    {
        gpio_pin_read(drv_data->input_pin_gpio_device,7,&echo_pulse);
        end_time = _tsc_read();
    }while(echo_pulse==1);
    pulse_time = end_time - start_time;
    distance = (end_time - start_time)/(400*58);
    drv_data->distance = distance;
    drv_data->end_time = end_time;
}

static int hcsr04_sample_fetch_0(struct device *dev, enum sensor_channel chan)
{
    struct hcsr04_data *drv_data = dev->driver_data;
    gpio_pin_enable_callback(drv_data->input_pin_gpio_device, 2);
    
    gpio_pin_write(drv_data->output_pin_gpio_device,6,1);
    k_sleep(K_MSEC(1));
    gpio_pin_write(drv_data->output_pin_gpio_device,6,0);
    k_sleep(K_MSEC(1));
    return 0;
}


static int hcsr04_channel_get_0(struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
    struct hcsr04_data *drv_data = dev->driver_data;

    __ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL || chan == SENSOR_CHAN_DISTANCE);
    val->val1 = drv_data->distance;
    val->val2 = drv_data->end_time;
    return 0;
}

static int hcsr04_sample_fetch_1(struct device *dev, enum sensor_channel chan)
{
    struct hcsr04_data *drv_data = dev->driver_data;
    gpio_pin_enable_callback(drv_data->input_pin_gpio_device, 7);
    
    gpio_pin_write(drv_data->output_pin_gpio_device,5,1);
    k_sleep(K_MSEC(1));
    gpio_pin_write(drv_data->output_pin_gpio_device,5,0);
    k_sleep(K_MSEC(1));
    return 0;
}


static int hcsr04_channel_get_1(struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
    struct hcsr04_data *drv_data = dev->driver_data;

    __ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL || chan == SENSOR_CHAN_DISTANCE);
    val->val1 = drv_data->distance;
    val->val2 = drv_data->end_time;
    return 0;
}

static const struct sensor_driver_api hcsr04_driver_api_0 = {
    .sample_fetch = hcsr04_sample_fetch_0,
    .channel_get = hcsr04_channel_get_0,
};

static const struct sensor_driver_api hcsr04_driver_api_1 = {
    .sample_fetch = hcsr04_sample_fetch_1,
    .channel_get = hcsr04_channel_get_1,
};

static int hcsr04_init_0(struct device *dev)
{
    struct hcsr04_data *drv_data = dev->driver_data;

    //IO10 -- Output pin -> ECHO PIN
    drv_data->input_gpio_pinmux = device_get_binding(CONFIG_PINMUX_NAME);
    pinmux_pin_set(drv_data->input_gpio_pinmux, 10, PINMUX_FUNC_A);
    drv_data->input_pin_gpio_device = device_get_binding(CONFIG_GPIO_DW_0_NAME);
    gpio_pin_configure(drv_data->input_pin_gpio_device, 2, GPIO_DIR_OUT);
    //setting 0
    if(gpio_pin_write(drv_data->input_pin_gpio_device, 2, 0))
    {printk("Output Failed");}


    //IO10 -- Input pin -> ECHO PIN
    drv_data->input_gpio_pinmux = device_get_binding(CONFIG_PINMUX_NAME);
    pinmux_pin_set(drv_data->input_gpio_pinmux, 10, PINMUX_FUNC_B);
    drv_data->input_pin_gpio_device = device_get_binding(CONFIG_GPIO_DW_0_NAME);
    gpio_pin_configure(drv_data->input_pin_gpio_device, 2, GPIO_DIR_IN | GPIO_INT | EDGE);
    gpio_init_callback(&(drv_data->input_pin_callback), callback_function_0, BIT(2));
    gpio_add_callback(drv_data->input_pin_gpio_device, &(drv_data->input_pin_callback));
    
    //IO3 -- Output pin -> TRIGGER PIN
    drv_data->output_pin_gpio_device = device_get_binding(CONFIG_GPIO_DW_0_NAME);
    drv_data->output_pin_gpio_pinmux = device_get_binding(CONFIG_PINMUX_NAME);
    pinmux_pin_set(drv_data->output_pin_gpio_pinmux, 3, PINMUX_FUNC_A);
    gpio_pin_configure(drv_data->output_pin_gpio_device, 6, GPIO_DIR_OUT);
    
    //IO3 -- Setting as 0
    if(gpio_pin_write(drv_data->output_pin_gpio_device, 6, 0))
    {printk("Output Failed");}

    return 0;
}

static int hcsr04_init_1(struct device *dev)
{
    struct hcsr04_data *drv_data = dev->driver_data;




    //IO12 -- Output pin -> ECHO PIN
    drv_data->input_gpio_pinmux = device_get_binding(CONFIG_PINMUX_NAME);
    pinmux_pin_set(drv_data->input_gpio_pinmux, 12, PINMUX_FUNC_A);//
    drv_data->input_pin_gpio_device = device_get_binding(CONFIG_GPIO_DW_0_NAME);
    gpio_pin_configure(drv_data->input_pin_gpio_device, 7, GPIO_DIR_OUT);
    //setting 0
    if(gpio_pin_write(drv_data->input_pin_gpio_device, 7, 0))
    {printk("Output Failed");}


    //IO12 -- Input pin -> ECHO PIN
    drv_data->input_gpio_pinmux = device_get_binding(CONFIG_PINMUX_NAME);
    pinmux_pin_set(drv_data->input_gpio_pinmux, 12, PINMUX_FUNC_B);
    drv_data->input_pin_gpio_device = device_get_binding(CONFIG_GPIO_DW_0_NAME);
    gpio_pin_configure(drv_data->input_pin_gpio_device, 7, GPIO_DIR_IN | GPIO_INT | EDGE);
    gpio_init_callback(&(drv_data->input_pin_callback), callback_function_1, BIT(7));
    gpio_add_callback(drv_data->input_pin_gpio_device, &(drv_data->input_pin_callback));
    
    //IO2 -- Output pin -> TRIGGER PIN
    drv_data->output_pin_gpio_device = device_get_binding(CONFIG_GPIO_DW_0_NAME);
    drv_data->output_pin_gpio_pinmux = device_get_binding(CONFIG_PINMUX_NAME);
    pinmux_pin_set(drv_data->output_pin_gpio_pinmux, 2, PINMUX_FUNC_A);
    gpio_pin_configure(drv_data->output_pin_gpio_device, 5, GPIO_DIR_OUT);
    
    //IO2 -- Setting as 0
    if(gpio_pin_write(drv_data->output_pin_gpio_device, 5, 0))
    {printk("Output Failed");}

    return 0;
}

struct hcsr04_data hcsr04_driver_0;
struct hcsr04_data hcsr04_driver_1;

DEVICE_AND_API_INIT(HCSR_0, CONFIG_HCSR04_0_NAME, hcsr04_init_0, &hcsr04_driver_0, NULL, POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &hcsr04_driver_api_0);
DEVICE_AND_API_INIT(HCSR_1, CONFIG_HCSR04_1_NAME, hcsr04_init_1, &hcsr04_driver_1, NULL, POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &hcsr04_driver_api_1);