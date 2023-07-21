#include <Arduino.h>
#include <memorysaver.h>
#include <sys/time.h>
#include <vector>

#include "functions/functions.h"
#include "gpio/gpio.h"
#include "reset/reset.h"

//persistant values
RTC_DATA_ATTR timeval sleep_time;
RTC_DATA_ATTR int boot_count = 0;
// pir pins
const gpio_num_t pir_signal = GPIO_NUM_39;
const gpio_num_t pir_power = GPIO_NUM_25;
//cam pins
const int camera_signal = 15;
const int camera_power = 13;
//static values
const int threshold_duration_s = 30;
const int max_reset_tries = 3;
const int wifi_reset_s = 10;
const int safe_heap = 75000;

void setup() {
    init_pir_sensor(pir_signal, pir_power); 
    Serial.begin(9600);
    std::vector<String> img_container = {"", "", "", ""};

    int reset_count = get_reset_count();
    bool reset = (reset_count > -1) && (reset_count < max_reset_tries);

    if (check_threshold(threshold_duration_s, sleep_time) || reset) {  
        start_wifi();
        if (reset) {
            load_pics_from_file(img_container);
        } else {
            ++boot_count;
            print_free_heap();
            capture_pics(img_container, camera_signal, camera_power);
        }
        check_wifi_or_reset(img_container, reset_count, wifi_reset_s);
        free_memory_if_over(img_container, safe_heap);
        send_email(img_container, reset, boot_count);
        set_new_time(sleep_time);
    } else {
        if (reset_count >= max_reset_tries) {
            Serial.println("Reset failed");
            erase_pics_file();
        } else {
            Serial.println("Duration below threshold");
        }
        delay(2000); //allow PIR sensor to stabilize
    }
    sleep();
}

void loop() {}