#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include <esp32-hal.h>
#include "gpio.h"

void init_pir() {
    rtc_gpio_hold_dis(GPIO_NUM_25); //interrupt pir power
    rtc_gpio_init(GPIO_NUM_25);
    rtc_gpio_set_direction(GPIO_NUM_25, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_set_level(GPIO_NUM_25, 0);
    delay(10);
    rtc_gpio_set_level(GPIO_NUM_25, 1);
    rtc_gpio_hold_en(GPIO_NUM_25);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 1);  // enable sensor wake
}