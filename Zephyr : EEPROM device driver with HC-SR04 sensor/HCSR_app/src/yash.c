//################################################################################################
//
// Program     : Assignment 4 Measurement in Zephyr RTOS
// Source file : main.c
// Authors     : Yash Shah and Ravi Bhushan Team #3
// Date        : 30th March 2018
//
//################################################################################################


#include <nanokernel.h>
#include <board.h>
#include <device.h>
#include <init.h>
#include <misc/shell.h>
#include <zephyr.h>
#include <i2c.h>
#include <gpio.h>
#include <pinmux.h>
#include <hcsr04.h>
#include <string.h>
#include <flash.h>




#define I2C_ADDRESS	0x54
#define SIZE_OF_BUFFER 64
#define STACKSIZE 2000
#define SLEEP  100
#define CPU_TICKS (SLEEP * sys_clock_ticks_per_sec / 1000)

#if defined(CONFIG_STDOUT_CONSOLE)
  #include <stdio.h>
  #define PRINT           printf
  #else
  #include <misc/printk.h>
  #define PRINT           printk
#endif

struct device *gpio;
struct device *gpio_sus;
struct device *i2c_dev, *i2c;
struct device *pinmux;
struct device *exp0;
struct device *hcsr04;

struct nano_sem fiber_hcsr;
struct nano_sem fiber_eeprom;
char __stack fiberStack[STACKSIZE];

uint8_t hbyte_addr = 0x00;
uint8_t lbyte_addr = 0x00;
uint32_t buff_A[SIZE_OF_BUFFER];
uint32_t buff_B[SIZE_OF_BUFFER];
uint32_t write_buff[SIZE_OF_BUFFER];
uint32_t read_buff[SIZE_OF_BUFFER];
uint32_t temp_buff[SIZE_OF_BUFFER];
int write_count1 =0 ;
int set_flag_write = 0;
int read_count1 =0 ;
int set_flag_read= 0;
int count_buff_1 =0;
int current_page =0;
int w_page;
int r_page1;
int r_page2;
int set_get =0;
/* String to integer conversion */

int conversion(char *string)
{	
	int  i, len;
	int result=0;
	len = strlen(string);
	for(i=0; i<len; i++){
		result = result * 10 + ( string[i] - '0' );
	}
	return result;	
}

/* Page write address calculation */
 
void calc_addr(int page_num)
{
	int count = 0;
	int temp_byte;
	int addr_bytes;
	hbyte_addr = 0x00;
	lbyte_addr = 0x00;
	while(count != page_num)
	{
		addr_bytes = hbyte_addr;
		temp_byte = lbyte_addr;
		addr_bytes = (addr_bytes << 8 ) | (temp_byte);
		addr_bytes = addr_bytes + 64;
		lbyte_addr = (addr_bytes & 0xFF);	//Low Byte
		hbyte_addr = (addr_bytes & 0xFF00) >> 8 ;	//High Byte
		count+=1;
	}
}

/* EEPROM write function */

int write_function(uint32_t *data_in_buffer, int count)  //count is number of pages to write
{
	int pagecount = 0;
	uint8_t buff[66];
	uint8_t data[64];
	int bufindex, temp;
	int i;

	memcpy(data , (uint8_t*)data_in_buffer , 64);
		
	for(pagecount=0;pagecount < count; pagecount++)
	{
		//struct i2c_msg msgs[2];
		calc_addr(current_page);
		buff[0] = hbyte_addr;
		buff[1] = lbyte_addr;
		PRINT("------->Setting current position to page %d address = 0x%02x%02x \n", current_page, hbyte_addr, lbyte_addr);
		bufindex = 2;
		for(i=pagecount*64; i<(pagecount*64)+64; i++)
		{
			buff[bufindex] = data[i];
			bufindex++;
		}
		
		temp=i2c_write(i2c_dev, buff, 66, I2C_ADDRESS );
		if(!temp) PRINT("return i2c write %d\n", temp);
		else PRINT("return i2c write %d\n", temp);
		fiber_sleep(CPU_TICKS);
		if(current_page == 511)
			current_page = 0;
		else
			current_page++;
	}
	return 0;
}

/* EEPROM read function */

int eeprom_read(uint32_t *read_data_in_buffer, int count)
{
	int pagecount = 0;
	int temp ;
	uint8_t buff[5];
	int i;	
	
	for(pagecount=0;pagecount < count; pagecount++)
	{
		calc_addr(r_page1++);
		buff[0] = hbyte_addr;
		buff[1] = lbyte_addr;
		PRINT("-------> page %d address = 0x%02x%02x \n", current_page, hbyte_addr, lbyte_addr);
		temp=i2c_write(i2c_dev,buff,2, I2C_ADDRESS );
		if(!temp) PRINT("return i2c write %d\n", temp);
		else PRINT("return i2c write %d\n", temp);
	
		fiber_sleep(CPU_TICKS);
	  //  PRINT("Test \n");
		temp=i2c_read(i2c_dev,(uint8_t *)read_data_in_buffer, 64,I2C_ADDRESS );
		if(!temp) PRINT("return i2c read %d\n", temp);
		else PRINT("return i2c read %d\n", temp);

		fiber_sleep(CPU_TICKS);
	
		if(current_page == 511)
			current_page = 0;
		else
			current_page++;
		for(i=0; i< 8; i=i+2)
			PRINT("read = %d    timestamp = %d\n", read_data_in_buffer[i] , read_data_in_buffer[i+1]);	
		PRINT("\n\n" );	

	}
	
	return 0;
}



static int shell_cmd_enable(int argc, char *argv[])
{
	int enable = conversion(argv[1]);
	PRINT("enable = %d\n", enable);
	if(enable==0)
	{
		PRINT("None of the devices are enabled\n");
		enable=0;
	}
	else if(enable==1)
	{
		PRINT("Device HCSR0 is enabled\n");
		enable=1;
		hcsr04 = device_get_binding(CONFIG_HCSR04_DW_0_NAME);
		if(!hcsr04)
		PRINT("not binding\n");
	}
	else if(enable==2)
	{
		PRINT("Device HCSR1 is enabled\n");
		enable=2;
		hcsr04 = device_get_binding(CONFIG_HCSR04_DW_1_NAME);
		if(!hcsr04)
		PRINT("not binding\n");
	}
	else
		PRINT("Enter either 0,1 or 2\n");
return 0;
}

void eeprom_w(void)
{

PRINT("eeprom_w entered 1\n");
int i,ret;
	for(i=0; i<w_page; i++){
	
	if(set_flag_read ==0)
	{
		PRINT("blocked 1\n");
		nano_fiber_sem_take(&fiber_hcsr, TICKS_UNLIMITED);
		PRINT("blocked released 1\n");
		ret = write_function(write_buff, 1);
		if(ret == 0)
		{set_flag_read =1;
			while(set_get!=1);
		nano_fiber_sem_give(&fiber_eeprom);
		}
	}

	}	
	

}

static int shell_cmd_start(int argc, char *argv[])
{
	
	 w_page= conversion(argv[1]);
PRINT("start  %d\n",w_page);
	
	

nano_sem_init(&fiber_hcsr);
nano_sem_init(&fiber_eeprom);
fiber_fiber_start(&fiberStack[0], STACKSIZE,
			(nano_fiber_entry_t) eeprom_w, 0, 0, 7, 0);

uint32_t  tsc1;
uint32_t  tsc2;
int i,temp;
uint32_t val;
int k;

for(k =0; k< w_page; k++){	
	for(i=0; i<8; i++){
		PRINT("for enterred\n");
tsc1 = sys_cycle_get_32();
			temp = hcsr04_write(hcsr04, CONFIG_GPIO_INT_PIN_1);
			if (temp != 0) 
				PRINT("hc-sr04_write config error %d!!\n", temp);
				
			fiber_sleep(20);
			
			temp = hcsr04_read(hcsr04, 7, &val);
tsc2 = sys_cycle_get_32();
			if (temp != 0)
     				PRINT("hc-sr04_read config error %d!!\n", temp);
 			else
			PRINT("val = %d\n", val);


			
			if(write_count1 < 16)
				{
	  
					write_buff[write_count1] = val;
					write_buff[write_count1+1] = tsc2-tsc1;
					write_count1 = write_count1 +2;
				}
			
			if(write_count1 == 16)
			{
				
				set_flag_write = 1;
			 	
				
			
				nano_fiber_sem_give(&fiber_hcsr);
				
				
				nano_fiber_sem_take(&fiber_eeprom, 20 );
				set_get =1;
				
				if((set_flag_write == 1) && (set_flag_read ==1))
				{
				
					memcpy(temp_buff, write_buff, sizeof(write_buff));
					memcpy(write_buff, read_buff, sizeof(read_buff));
					memcpy(write_buff, temp_buff, sizeof(temp_buff));
					set_flag_write =0; set_flag_read = 0; write_count1 = 0;set_get =0;
				
				}
			}
			
			
		}
write_count1 = 0;
nano_sem_init(&fiber_hcsr);
nano_sem_init(&fiber_eeprom);
}
return 0;
	
}

static int shell_cmd_p1p2(int argc, char *argv[])
{
	PRINT("dump\n");
	r_page1=conversion(argv[1]);
	r_page2=conversion(argv[2]);
	uint32_t read_data_in_buffer[16];	
	eeprom_read(read_data_in_buffer, r_page2-r_page1);
return 0;
}


const struct shell_cmd commands[] = {
	{ "enable", shell_cmd_enable },
	{ "start", shell_cmd_start },
	{ "dump", shell_cmd_p1p2 },
	{ NULL, NULL }
};




void main(void)
{
	int temp;
	gpio = device_get_binding(CONFIG_GPIO_PCAL9535A_2_DEV_NAME);
	i2c = device_get_binding("I2C_0");
	if (!i2c) {
		PRINT("I2C not found!!\n");
	}
	i2c_dev = device_get_binding(CONFIG_GPIO_PCAL9535A_1_I2C_MASTER_DEV_NAME);
		
	if (!gpio) {
		PRINT("GPIO not found!!\n");
	} else {
		temp = gpio_pin_write(gpio, 12, 0);  	
		if (temp != 0){ 
			PRINT("GPIO config error %d!!\n", temp);
		}
		temp = gpio_pin_configure(gpio, 12, GPIO_DIR_OUT);  	
		if (temp != 0){ 
			PRINT("GPIO config error %d!!\n", temp);
		}
	}
  	
	if (!i2c_dev) {
		PRINT("I2C not found!!\n");
	} else {
		temp = i2c_configure(i2c_dev, (I2C_SPEED_FAST << 1) | I2C_MODE_MASTER);
		if (temp != 0) {
			PRINT("I2C configuration error: %d\n", temp);
		}
		else {
			PRINT("I2C configuration : %d\n", temp);
		}
	}

	memset(buff_A, 0, sizeof(buff_A));
	memset(buff_B, 0, sizeof(buff_B));
	memcpy(write_buff, buff_A, sizeof(buff_A));  
	memcpy(read_buff, buff_B, sizeof(buff_A));  

	shell_init("shell> ", commands);

	}