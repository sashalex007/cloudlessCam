#include <LittleFS.h>
#include <WiFi.h>
#include <memorysaver.h>
#include <sys/time.h>
#include <vector>

#include "email/email.h"
#include "camera/camera.h"
#include "gpio/gpio.h"
#include "private_variables.h"
#include "read_battery/read_battery.h"
#include "reset/reset.h"

using namespace std;
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

void ram_status() {
    Serial.printf( "heap: %d free\n", esp_get_free_heap_size());
}

void check_memory(vector<String>& base64_vector) {
    ram_status();
    while (esp_get_free_heap_size() < safe_heap) {
        base64_vector.pop_back();
        Serial.println("Image removed");
    }
    ram_status();
}

void capture_pics(vector<String>& base64_vector) {
    Camera cam(camera_signal, camera_power);
    int img_count = 0;
    for (String& base64_string : base64_vector) {
        cam.capture(base64_string);
        ++img_count;
        if (img_count < base64_vector.size()) {
            delay(100);
        }
    }
    cam.power_down(camera_power);
}

void send_email(vector<String>& base64_vector, bool reset) {
    Email email;
    int img_count = 0;
    for (String& base64_string : base64_vector) {
        if (base64_string != "") {
            email.add_attachment(base64_string, img_count);
        }
        ++img_count;
    }
    email.send(reset, boot_count, 0);
}

bool check_threshold() {
    int duration_s = threshold_duration_s;
    timeval duration;  // compute duration since last boot
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        timeval timeNow;
        gettimeofday(&timeNow, NULL);
        timersub(&timeNow, &sleep_time, &duration);
        Serial.printf("Duration: %" PRIu64 "s\n", (duration.tv_sec * (uint64_t)1));
    }
    return (duration.tv_sec * (uint64_t)1) > (time_t)duration_s || duration.tv_sec * (uint64_t)1 < (time_t)0;
}

void check_wifi(vector<String>& base64_vector, int reset_count) {
    int connection_timeout = wifi_reset_s;
    int timeout_counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(200);
        timeout_counter++;
        if (timeout_counter >= connection_timeout * 5) {
            Serial.println("Wifi failure, saving data...");
            save_reset(base64_vector, reset_count);
            Serial.println("Reset initiated");
            ESP.restart();
        }
    }
    Serial.println(WiFi.localIP());
}

void set_new_time() {
    gettimeofday(&sleep_time, NULL);
}

void sleep() {
    Serial.println("Going to sleep now");
    delay(100);
    esp_deep_sleep_start();
}
 
void setup() {
    init_pir(pir_signal, pir_power); 
    Serial.begin(9600);
    vector<String> img_container = {"", "", "", ""};

    int reset_count = get_reset_count();
    bool reset = reset_count > -1;

    if ((check_threshold() || reset) && reset_count < max_reset_tries) {  
        WiFi.begin(ssid, password);
        if (reset) {
            open_reset(img_container);
        } else {
            ++boot_count;
            ram_status();
            capture_pics(img_container);
        }
        check_wifi(img_container, reset_count);
        check_memory(img_container);
        send_email(img_container, reset);
        set_new_time();
    } else {
        if (reset_count >= max_reset_tries) {
            Serial.println("Reset failed");
            erase_reset();
        } else {
            Serial.println("Duration below threshold");
        }
        delay(2000);
    }
    sleep();
}

void loop() {}