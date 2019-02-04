#include <i2c.h>
#include <init.h>
#include <gpio.h>
#include <string.h>
#include <device.h>
#include <zephyr.h>
#include <flash.h>
#include "eeprom_24FC256.h"

#define EEPROM_ADDRESS 0x54

static int eeprom_read_data(struct device *dev, off_t offset, void *data, size_t len)
{
	uint8_t addr[2];
	struct eeprom_data *drv_data = dev->driver_data;
	addr[0] = (offset>>8)&0xFF;
	addr[1] = (offset&0xFF);
	if(i2c_write(drv_data->i2c, addr, sizeof(addr), EEPROM_ADDRESS))
	{
		printk("I2C Address Error\n");
	}
	
	return i2c_read(drv_data->i2c, data, len, EEPROM_ADDRESS);
}


static int eeprom_write_data(struct device *dev, off_t offset, const void *data, size_t len)
{
	uint8_t addr[2];
	struct eeprom_data *drv_data = dev->driver_data;
	addr[0] = (offset>>8)&0xFF;
	addr[1] = (offset&0xFF);
	if(i2c_write(drv_data->i2c, data, len+2, EEPROM_ADDRESS))
	{
		printk("I2C Write Error\n");
	}
	k_sleep(1);
	return 0;
}


static const struct flash_driver_api eeprom_api = {
	.read = eeprom_read_data,
	.write = eeprom_write_data,
};

static int eeprom_init(struct device *dev)
{
	struct eeprom_data *drv_data = dev->driver_data;
	
	drv_data->gpio_driver = device_get_binding(CONFIG_GPIO_PCAL9535A_2_DEV_NAME);
	gpio_pin_write(drv_data->gpio_driver, 12, 0);
	gpio_pin_configure(drv_data->gpio_driver, 12, GPIO_DIR_OUT);
	drv_data->i2c = device_get_binding(CONFIG_GPIO_PCAL9535A_1_I2C_MASTER_DEV_NAME);
	i2c_configure(drv_data->i2c, (I2C_SPEED_FAST<<1)|I2C_MODE_MASTER);
	dev->driver_api = &eeprom_api;
	return 0;
}

static struct eeprom_data eeprom_24fc256_data;
DEVICE_INIT(EEPROM, CONFIG_EEPROM_24FC256_NAME, eeprom_init, &eeprom_24fc256_data, NULL, POST_KERNEL, CONFIG_EEPROM_24FC256_INIT_PRIORITY);


