#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
//句柄
static QueueHandle_t gpio_evt_queue = NULL;
//任务队列
static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}
//中断函数
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void app_main(void)
{
    //配置结构体
    gpio_config_t io_conf = {
        //设置中断 （下降沿）
        .intr_type = GPIO_INTR_NEGEDGE,
        //设置输出和输入模式（输入）
        .mode = GPIO_MODE_INPUT,
        //设置引脚 9
        .pin_bit_mask = 1ULL<<GPIO_NUM_9,
        //是否打开下拉模式
        .pull_down_en = 0,
        //是否打开上拉模式
        .pull_up_en = 1
    };
    //对结构体进行配置
    gpio_config(&io_conf);

    //创建队列
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //添加中断 
    gpio_install_isr_service(0);
    //中断函数 引脚 函数名称 参数
    gpio_isr_handler_add(GPIO_NUM_9, gpio_isr_handler, (void*) GPIO_NUM_9);
}
