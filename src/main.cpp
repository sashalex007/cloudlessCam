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

RTC_DATA_ATTR timeval sleep_time;
RTC_DATA_ATTR int boot_count = 0;

// pins
const int pir_signal = 39;
const int pir_power = 25;
const int camera_signal = 15;
const int camera_power = 13;

void capture_pics(vector<String>& base64_vector) {
    Camera cam(camera_signal, camera_power);
    int img_count = 0;
    for (String& base64_string : base64_vector) {
        if (ESP.getFreeHeap() > 150000) {
            cam.capture(base64_string);
        }
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
    int duration_s = 30;

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
    vector<String> img_container = {"", "", "", ""};
    Serial.begin(9600);

    pair<bool, int> reset_result = is_reset();
    bool reset = reset_result.first;
    int reset_count = reset_result.second;

    if ((check_threshold() || reset) && reset_count < 4) {  // check if over time threshold or reset
        WiFi.begin(ssid, password);
        if (reset) {
            open_reset(img_container);
        } else {
            ++boot_count;
            capture_pics(img_container);
        }
        check_wifi(img_container, reset_count);
        send_email(img_container, reset);
        set_new_time();
    } else {
        if (reset_count >= 4) {
            Serial.println("Reset failed");
        } else {
            Serial.println("Duration below threshold");
        }
        delay(2000);
    }
    sleep();
}

void loop() {}