//################################################################################################
//
// Program     : Assignment 4 Measurement in Zephyr RTOS
// Source file : main.c
// Authors     : Yash Shah and Ravi Bhushan Team #3
// Date        : 30th March 2018
//
//################################################################################################

//Including Appropriate Headers
#include <zephyr.h>
#include <board.h>
#include <device.h>
#include <pinmux.h>
#include "pinmux_galileo.h"
#include <gpio.h>
#include <misc/printk.h>
#include <string.h>
#include <sensor.h>
#include <flash.h>
#include <shell/shell.h>


#define MY_SHELL_MODULE "Sensors"
#define INITIAL_ADDRESS 0x00
#define INITIAL_PAGE 0
#define NEXT_PAGE 64

struct device *dev;
struct device *eeprom;
u8_t read_buffer[64], write_buffer[66];
u32_t initialize_time = 0;
u32_t initialize_distance = 0;
int sensor_enabled = 0;

int conversion(char *string) {   
    int  i, len;
    int result=0;
    len = strlen(string);
    for(i=0; i<len; i++){
        result = result * 10 + ( string[i] - '0' );
    }
    return result;  
}

static int enable_sensor(int argc, char *argv[]) {

    int choice = conversion(argv[1]);
    if(choice ==0){
        sensor_enabled = 0;
        printk("No Sensors Enabled\n");
    } else if(choice ==1){
        sensor_enabled = 1;
        dev = device_get_binding("HCSR04_0");
        
        __ASSERT(dev != NULL, "Failed to get device binding\n");
        __ASSERT(eeprom != NULL, "Failed to get device binding\n");
        printk("device is %p, name is %s\n", dev, dev->config->name);
        printk("device is %p, name is %s\n", eeprom, eeprom->config->name);
    } else if(choice ==2){
        sensor_enabled = 1;
        dev = device_get_binding("HCSR04_1");

        __ASSERT(dev != NULL, "Failed to get device binding\n");
        __ASSERT(eeprom != NULL, "Failed to get device binding\n");
        printk("device is %p, name is %s\n", dev, dev->config->name);
        printk("device is %p, name is %s\n", eeprom, eeprom->config->name);
    }
    return 0;
}

static int start_till_page(int argc, char *argv[]) {
    int choice = conversion(argv[1]);
    if(!sensor_enabled) {
        printk("Please Enable Sensor by typing Enable [1,2]\n");
    }else {
        if(choice > 512){
            printk("Please enter value less than 512\n");
        }else {
            printk("Fetching Sensor Values\n");
            //Initializing pages to 0
            
            
            u16_t page_address = INITIAL_PAGE;
            for(int page = 0;page < choice;page++){
                write_buffer[0] = (page_address>>8)&0xFF;
                write_buffer[1] = (page_address&0xFF);
                for(int sample = 0;sample < 8; sample++){
                    int ret;
                    u32_t start_recording = _tsc_read();
                    ret = sensor_sample_fetch(dev);
                    if (ret) {
                        printk("sensor_sample_fetch failed ret %d\n", ret);
                        return 0;
                    }
                    struct sensor_value temp_value;
                    ret = sensor_channel_get(dev, SENSOR_CHAN_PROX, &temp_value);
                    if (ret) {
                        printk("sensor_channel_get failed ret %d\n", ret);
                        return 0;
                    }
                    u32_t distance = temp_value.val1;
                    u32_t timestamp = SYS_CLOCK_HW_CYCLES_TO_NS(temp_value.val2 - start_recording);
                    printk("distance::%d and time::%d ns\n",distance,timestamp);
                    write_buffer[sample*8 + 2] = timestamp&0xFF;
                    write_buffer[sample*8 + 3] = (timestamp>>8)&0xFF;
                    write_buffer[sample*8 + 4] = (timestamp>>16)&0xFF;
                    write_buffer[sample*8 + 5] = (timestamp>>24)&0xFF;
                    write_buffer[sample*8 + 6] = distance&0xFF;
                    write_buffer[sample*8 + 7] = (distance>>8)&0xFF;
                    write_buffer[sample*8 + 8] = (distance>>16)&0xFF;
                    write_buffer[sample*8 + 9] = (distance>>24)&0xFF;
                }
               
                flash_write(eeprom, INITIAL_ADDRESS, write_buffer, 66);
                page_address = page_address + NEXT_PAGE;
            }
            
            printk("Sensor Values Fetch Complete\n");
        }
    }
    return 0;
}

static int dump_eeprom_data(int argc, char *argv[]) {
    
    int choice1 = conversion(argv[1]);
    int choice2 = conversion(argv[2]);

    if(!sensor_enabled) {
        printk("Please Enable Sensor by typing Enable [1,2]\n");
    }else {
        if(choice1>choice2) {
            printk("No Pages showed as first page is greater than second page\n");
        }
        u16_t page_address_read = INITIAL_PAGE;
        for(int p=0;p<choice1;p++) {
            page_address_read = page_address_read + 0x0040;
        }
            for(int page = 0;page <= choice2-choice1;page++) {
                flash_read(eeprom, page_address_read, read_buffer, 64);
                for(int sample = 0;sample < 8; sample++){
                    u32_t time = ((u32_t)read_buffer[sample*8 + 0]<<24)|((u32_t)read_buffer[sample*8 + 1]<<16)|((u32_t)read_buffer[sample*8 + 2]<<8)|((u32_t)read_buffer[sample*8 + 3]);
                    u32_t distance = ((u32_t)read_buffer[sample*8 + 4]<<24)|((u32_t)read_buffer[sample*8 + 5]<<16)|((u32_t)read_buffer[sample*8 + 6]<<8)|((u32_t)read_buffer[sample*8 + 7]);
                    printk("time:%d",time);
                    printk("distance:%d\n",distance);
                }
                
                page_address_read = page_address_read + NEXT_PAGE;
            }

    }
    return 0;
}


static struct shell_cmd commands[] = {
    { "Enable",enable_sensor, "Enable 0 to Enable None, 1 to Enable HCSR0, 2 to Enable HCSR1" },
     { "Start",start_till_page, " Start p : Collect Data Till P page. p<=512" },
      { "Dump",dump_eeprom_data, "Dump p1 p2 : Print data from p1 to p2. p1<p2" },
    { NULL, NULL, NULL }
};

void main(void) {

    eeprom = device_get_binding(CONFIG_EEPROM_24FC256_NAME);
    SHELL_REGISTER(MY_SHELL_MODULE, commands);
    
}