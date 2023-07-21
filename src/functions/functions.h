#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void start_wifi();
void print_free_heap();
void free_memory_if_over(std::vector<String>& base64_vector, int safe_heap);
void capture_pics(std::vector<String>& base64_vector, const int camera_signal, const int camera_power);
void send_email(std::vector<String>& base64_vector, const bool reset, const int boot_count);
bool check_threshold(const int threshold_duration_s, timeval& sleep_time);
void check_wifi_or_reset(std::vector<String>& base64_vector, const int reset_count, const int wifi_reset_s);
void set_new_time(timeval& sleep_time);
void sleep();

#endif