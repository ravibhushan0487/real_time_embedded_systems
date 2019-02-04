//Including Appropriate Headers
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

//Declaring Sample Sizes to record Measurements
#define CONTEXT_SWITCH_OVERHEAD_SAMPLE_SIZE 500
#define INTERRUPT_LATENCY_SAMPLE_SIZE 500
#define INTERRUPT_LATENCY_BG_SAMPLE_SIZE 500
#define MY_SHELL_MODULE "Samples"
u64_t context_switch_samples[CONTEXT_SWITCH_OVERHEAD_SAMPLE_SIZE];
u64_t interrupt_latency_samples[INTERRUPT_LATENCY_SAMPLE_SIZE];
u64_t interrupt_latency_bg_samples[INTERRUPT_LATENCY_BG_SAMPLE_SIZE];
u64_t start_time=0;
u64_t end_time=0;
int gpio_output_count = 0;
int pwm_callback = 0;
static struct gpio_callback input_pin_callback;

//Interrupt Variables
#define EDGE    (GPIO_INT_EDGE | GPIO_INT_ACTIVE_HIGH)


//Context Switch Variables
#define THREADSTACK 1000
int countext_switch_counter = 0;
int less_priority_thread_running = 0;
K_MUTEX_DEFINE(context_mutex);
char context_thread_stacks[4][THREADSTACK];
static struct k_thread context_threads[4];

//Context Switch Time Variables
u64_t context_start;
u64_t context_end;
u64_t context_cycle;

//Mutex Lock Time Vatiables
u64_t mutex_unlock_start;
u64_t mutex_unlock_stop;
u64_t mutex_cycle;

static int interupt_latency_no_background_task(int argc, char *argv[])
{
   for(int counter=0;counter<INTERRUPT_LATENCY_SAMPLE_SIZE;counter++){
        printk("%llu\t",interrupt_latency_samples[counter]);
        if((counter%20) == 0) {
            printk("\n");
        }
    }
    printk("\n");
    return 0;
}


static int interupt_latency_background_task(int argc, char *argv[])
{
    for(int counter=0;counter<INTERRUPT_LATENCY_BG_SAMPLE_SIZE;counter++){
        printk("%llu\t",interrupt_latency_bg_samples[counter]);
        if((counter%20) == 0) {
            printk("\n");
        }
    }
    printk("\n");
    return 0;
}

static int context_switch_time(int argc, char *argv[])
{
   for(int counter=0;counter<CONTEXT_SWITCH_OVERHEAD_SAMPLE_SIZE;counter++)
   {
        printk("%llu\t",context_switch_samples[counter]);
        if((counter%20) == 0) {
            printk("\n");
        }
    }
    printk("\n");
    return 0;
}


// Below is a static structure which denotes the options available to user in the registered shell module
// Option 1 will output 500 sample measurements related to Interupt Latency
// Option 2 will output Interrupt Latency Overhead Measurements Without Background Tasks
// OPtion 3 will output Context Switching Overhead Measurements

static struct shell_cmd commands[] = {
    { "1",interupt_latency_no_background_task, "Show Interupt Latency Overhead Measurements" },
     { "2",interupt_latency_background_task, "Show Interupt Latency Overhead Measurements With Background Task" },
      { "3",context_switch_time, "Show Context Switching Overhead Measurements" },
    { NULL, NULL, NULL }
};


void input_pin_callback_function(struct device *gpiob, struct gpio_callback *cb,
            u32_t pins)
{
    //u64_t temp;
    //start = _tsc_read();
    end_time = _tsc_read();
    if(pwm_callback) 
    {
        interrupt_latency_bg_samples[gpio_output_count] = SYS_CLOCK_HW_CYCLES_TO_NS(end_time - start_time);
        gpio_output_count = gpio_output_count+1; 
    }
    else 
    {
        interrupt_latency_samples[gpio_output_count] = SYS_CLOCK_HW_CYCLES_TO_NS(end_time - start_time);
        gpio_output_count = gpio_output_count+1; 
    }

}


//Thread 2 Callback Function
void context_thread_two(void) {
    while (countext_switch_counter < CONTEXT_SWITCH_OVERHEAD_SAMPLE_SIZE) 
    {
        k_mutex_lock(&context_mutex, K_FOREVER);
        if(less_priority_thread_running) 
        {
            context_end = _tsc_read();
           
            k_mutex_lock(&context_mutex, K_FOREVER);
            mutex_unlock_start = _tsc_read();
            k_mutex_unlock(&context_mutex);
            
            mutex_unlock_stop = _tsc_read();
            mutex_cycle = mutex_unlock_stop - mutex_unlock_start;
            context_cycle = context_end - context_start - mutex_cycle;
            context_switch_samples[countext_switch_counter] = SYS_CLOCK_HW_CYCLES_TO_NS(context_cycle);
            
            countext_switch_counter++;
        }
        k_mutex_unlock(&context_mutex);
        k_sleep(400);
    }
    
}

//Thread 1 Callback Function
void context_thread_one(void) {
    while (countext_switch_counter < CONTEXT_SWITCH_OVERHEAD_SAMPLE_SIZE) 
    {
        less_priority_thread_running = 1;
        k_mutex_lock(&context_mutex, K_FOREVER);
        k_sleep(300);
        context_start = _tsc_read();
        k_mutex_unlock(&context_mutex);
        less_priority_thread_running = 0;
    }
    
}

//Implementation function of compute_and_record_context_switching_overhead declared above
void compute_and_record_context_switching_overhead(void) {
    
    k_thread_create(&context_threads[2], &context_thread_stacks[2][0], THREADSTACK, context_thread_one, NULL, NULL, NULL, 5, 0, 0);
    k_thread_create(&context_threads[3], &context_thread_stacks[3][0], THREADSTACK, context_thread_two, NULL, NULL, NULL, 7, 0, 0);
    k_sleep(1000000);
    k_thread_abort(&context_threads[2]);
    k_thread_abort(&context_threads[3]);
}

struct data_item_type {
    u32_t pal1;
    u32_t pal2;
    u32_t pal3;
}yts;

K_MSGQ_DEFINE(queue, sizeof(yts), 10, 4);

void MSend(void)
{
    while (1){
        struct data_item_type *sendy;
        sendy->pal1 = 21;
        sendy->pal2 = 22;
        sendy->pal3 = 23;

        while (k_msgq_put(&queue, &sendy, K_NO_WAIT) != 0) 
            {
        k_msgq_purge(&queue);}  
        k_sleep(100);
}}

void MReceive(void){
    struct data_item_type *receivey;
    while (1) {
        k_msgq_get(&queue, &receivey, K_FOREVER);
       
        k_sleep(100);
}}




//Implementation function of compute_and_record_interrupt_latency declared above
void compute_and_record_interrupt_latency() {
    
    struct device *input_pin_gpio_device;
    struct device *input_gpio_pinmux;
    struct device *output_pin_pwm;
    struct device *output_pin_gpio_device;
    struct device *output_pin_gpio_pinmux;
  
  
    // INPUT PIN - BINDING AS PER AN INPUT
    input_gpio_pinmux = device_get_binding(CONFIG_PINMUX_NAME);
    if(!input_gpio_pinmux)    {
        printk("Cannot find PinMux!\n");
        return;
    }

    if(pinmux_pin_set(input_gpio_pinmux, 5, PINMUX_FUNC_B))    {
        printk("Failed to Set Pinmux");
        return;
    }


    input_pin_gpio_device = device_get_binding(PINMUX_GALILEO_GPIO_INTEL_CW_NAME);
    if (!input_pin_gpio_device)     {
        printk("Error: Binding\n");
        return;
    }

    //OUTPUT PIN - CONFIGURED AS GPIO
    output_pin_gpio_pinmux = device_get_binding(CONFIG_PINMUX_NAME);
    if(!output_pin_gpio_pinmux)
    {        printk("Cannot find PinMux2!\n");
        return;}

    if(pinmux_pin_set(output_pin_gpio_pinmux, 6, PINMUX_FUNC_A))
    {   printk("Failed to Set Pinmux2 ");
        return;}

    output_pin_gpio_device = device_get_binding(PINMUX_GALILEO_GPIO_INTEL_CW_NAME);
    if (!output_pin_gpio_device) {printk("Error: Binding\n");
        return;}
    
    
    gpio_pin_configure(input_pin_gpio_device, 0, GPIO_DIR_IN | GPIO_INT | EDGE);
    gpio_init_callback(&input_pin_callback, input_pin_callback_function, BIT(0));
    gpio_add_callback(input_pin_gpio_device, &input_pin_callback);
    gpio_pin_enable_callback(input_pin_gpio_device, 0);


    
    gpio_pin_configure(output_pin_gpio_device, 1, GPIO_DIR_OUT);
    if(gpio_pin_write(output_pin_gpio_device, 1, 0))
        {printk("Output Write Failed");}
    
    
    while(gpio_output_count<INTERRUPT_LATENCY_SAMPLE_SIZE)
    {    
        start_time = _tsc_read();
        if(gpio_pin_write(output_pin_gpio_device, 1, 0))
        {printk("Output Failed");}
        if(gpio_pin_write(output_pin_gpio_device, 1, 1))
        {printk("Output Failed");}
    }
    

    //OUTPUT PIN - CONFIGURED AS PWM
    if(pinmux_pin_set(output_pin_gpio_pinmux, 6, PINMUX_FUNC_C))
    {
        printk("Failed to Set Pinmux2 Pin");
        return;
    }

    output_pin_pwm = device_get_binding(PINMUX_GALILEO_PWM0_NAME);
    if (!output_pin_pwm) {
        printk("Error: Get Binding\n");
        return;
    }



    k_thread_create(&context_threads[0], &context_thread_stacks[0][0], THREADSTACK, MSend, NULL, NULL, NULL, 5, 0, 0);
    k_thread_create(&context_threads[1], &context_thread_stacks[1][0], THREADSTACK, MReceive, NULL, NULL, NULL, 7, 0, 0);
    //k_sleep(1000000);

    gpio_output_count=0;
    pwm_callback = 1;
    int disable_callback = 0;

    while(!disable_callback)
    {   
        start_time = _tsc_read();
        if(gpio_pin_write(output_pin_gpio_device, 1, 0))
        {printk("Output Failed");}
        if(gpio_pin_write(output_pin_gpio_device, 1, 1))
        {printk("Output Failed");}
        //pwm_pin_set_cycles(output_pin_pwm, 5,4095,2000);
        if(gpio_output_count>=INTERRUPT_LATENCY_BG_SAMPLE_SIZE)
        {
           disable_callback = 1;
            gpio_pin_disable_callback(input_pin_gpio_device, 0);
        }
           
    }


    k_thread_abort(&context_threads[0]);
    k_thread_abort(&context_threads[1]);
}



void main(void) {

    printk("Task Executing.Please wait or press Enter and then type Help for intermediate data.\n");
   //Invoking the function to calculate Interrupt Latency with and without background tasks
    compute_and_record_interrupt_latency();

  //Invoking the function to calculate Context Switching Overhead 
   compute_and_record_context_switching_overhead();

   //Register Shell Module
   SHELL_REGISTER(MY_SHELL_MODULE, commands);
   
   printk("\nComputation Completed\nPlease press enter for Shell Prompt");
   // Sleeping the main thread so that it does not interfere with the ongoing computations
   k_sleep(15000);
}
