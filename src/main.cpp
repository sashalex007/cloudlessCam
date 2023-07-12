#include <WiFi.h>
#include <sys/time.h>
#include <LittleFS.h>
#include <memorysaver.h>
#include <vector>

#include "email/email.h"
#include "camera/camera.h"

#include "gpio/gpio.h"
#include "read_battery/read_battery.h"
#include "reset/reset.h"
#include "private_variables.h"

RTC_DATA_ATTR timeval sleep_time;
RTC_DATA_ATTR int boot_count = 0;
int reset_count = 0; //prevents reset bootloop. keep global
bool is_reset = false;
//pins
const int pir_signal = 39;
const int pir_power = 25;
const int camera_signal = 15;
const int camera_power = 13;

Email email;
std::vector<String> base64_vector = {"","","",""};

void start() {
    Camera cam(camera_signal, camera_power);
    int max_img = 4;
    int img_count = 0;

    while (img_count < max_img) {
        if (!is_reset && ESP.getFreeHeap() > 150000) {
            cam.capture(base64_vector[img_count]);
        }

        if (base64_vector[img_count] != "") { //skip empty string
            email.add_attachment(base64_vector[img_count], img_count);
            img_count++;
            if (img_count < max_img) {
                delay(100); //delay between captures
            }
        } else {
            Serial.println("Skipped"); 
            img_count++;
        }
    }
}

void check_reset() {
    LittleFS.begin();
    if (LittleFS.exists("/0.txt")) {  // check if boot is for is_reset
        is_reset = true;
        open_reset(base64_vector, reset_count);
    }
    if (reset_count >= 3) {
        Serial.println("Reset Failed...going to sleep now");
        delay(100);
        esp_deep_sleep_start();
    }
}

bool check_threshold() {
    int duration_s = 30;

    timeval duration;  // compute duration since last boot
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        timeval timeNow;
        gettimeofday(&timeNow, NULL);
        timersub(&timeNow, &sleep_time, &duration);
        Serial.printf("Duration: %" PRIu64 "s\n", (duration.tv_sec * (uint64_t)1));
    }
    return (duration.tv_sec * (uint64_t)1) > (time_t)duration_s || duration.tv_sec * (uint64_t)1 < (time_t)0 || is_reset;
}

void check_wifi() {
    int connection_timeout = 10;
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

void power_down_cam() {
    digitalWrite(camera_power, LOW);
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
    init_pir();  // init pir sensor power interrupt and wake pin
    Serial.begin(9600);
    check_reset(); // check if is_reset boot

    if (check_threshold()) { //check if over time threshold
        if (!is_reset) {
            boot_count++;
        }
        WiFi.begin(ssid, password);
        start();
        check_wifi();
        email.send(is_reset, boot_count, 0);
        power_down_cam();
        set_new_time();
    } else {
        Serial.println("Duration below threshold");
        delay(2000);
    }
    sleep();
}

void loop() {}