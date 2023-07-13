#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include <esp32-hal.h>
#include "gpio.h"

void init_pir(const int pir_signal, const int pir_power) {
    rtc_gpio_hold_dis(pir_power); //interrupt pir power
    rtc_gpio_init(pir_power);
    rtc_gpio_set_direction(pir_power, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_set_level(pir_power, 0);
    delay(10);
    rtc_gpio_set_level(pir_power, 1);
    rtc_gpio_hold_en(pir_power);
    esp_sleep_enable_ext0_wakeup(pir_signal, 1);  // enable sensor wake
}