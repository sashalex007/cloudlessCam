#include "email/email.h"
#include "camera/camera.h"
#include "reset/reset.h"
#include "./private_variables/private_variables.h"
#include "functions.h"

using namespace std;

void start_wifi() {
    WiFi.begin(ssid, password);
}

void print_free_heap() {
    Serial.printf( "heap: %d free\n", esp_get_free_heap_size());
}

void free_memory_if_over(vector<String>& base64_vector, const int safe_heap) {
    print_free_heap();
    while (esp_get_free_heap_size() < safe_heap) {
        base64_vector.pop_back();
        Serial.println("Image removed");
        print_free_heap();
    }
}

void capture_pics(vector<String>& base64_vector, const int camera_signal, const int camera_power) {
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

void send_email(vector<String>& base64_vector, const bool reset, const int boot_count) {
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

bool check_threshold(const int threshold_duration_s, timeval& sleep_time) {
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

void check_wifi_or_reset(vector<String>& base64_vector, const int reset_count, const int wifi_reset_s) {
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

void set_new_time(timeval& sleep_time) {
    gettimeofday(&sleep_time, NULL);
}

void sleep() {
    Serial.println("Going to sleep now");
    delay(100);
    esp_deep_sleep_start();
}