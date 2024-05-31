
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_timer.h"

#define LED_GPIO    GPIO_NUM_2
#define LDR_GPIO    GPIO_NUM_34

bool ldr_state = false;
SemaphoreHandle_t xSemaphore = NULL;
adc_oneshot_unit_handle_t adc1_handle;
long long old_time = 0;

void IRAM_ATTR ldr_isr_handler(void* arg) 
{
    long long current_time = esp_timer_get_time();
    if(current_time - old_time >= 300000){
        ldr_state = !ldr_state;
        xSemaphoreGiveFromISR(xSemaphore, NULL);
        old_time = esp_timer_get_time();
    }
}

void configure_led(void)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
}

void configure_interrupt(void)
{   
    gpio_set_direction(LDR_GPIO, GPIO_MODE_INPUT);
    gpio_set_intr_type(LDR_GPIO, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(LDR_GPIO, ldr_isr_handler, NULL);
}

void configure_adc(void)
{
    adc_oneshot_unit_init_cfg_t init_config1 = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_chan_cfg_t config = { .bitwidth = ADC_BITWIDTH_DEFAULT, .atten = ADC_ATTEN_DB_12 };

    adc_oneshot_new_unit(&init_config1, &adc1_handle);    
    adc_oneshot_config_channel(adc1_handle, LDR_GPIO, &config);
}

void app_main() 
{
    configure_led();
    configure_interrupt();

    xSemaphore = xSemaphoreCreateBinary();

    while (1) 
    {
        if(xSemaphoreTake(xSemaphore,portMAX_DELAY) == pdTRUE) {
            if(ldr_state){
                gpio_set_level(LED_GPIO, 0);
                ESP_LOGI("TEST", "LED DESLIGADO");
            }
            else{
                gpio_set_level(LED_GPIO, 1);
                ESP_LOGI("TEST", "LED LIGADO");
            }
        }

        ESP_LOGI("TEST", "Seguindo");
    }
}