#include <vector>
#include <WString.h>
#include <LittleFS.h>
#include "reset.h"

void save_reset(std::vector<String>& base64_vector, int reset_count) {
    File file;
    const char *files[4] = {"/0.txt", "/1.txt", "/2.txt", "/3.txt"};
    for (int i = 0; i < 4; i++) {
        file = LittleFS.open(files[i], "w");
        file.print(base64_vector[i]);
        delay(1);
        file.close();
    }

    file = LittleFS.open("/reset_count.txt", "w");
    file.print(reset_count + 1);
    delay(1);
    file.close();
}

void open_reset(std::vector<String>& base64_vector) {
    File file;
    const char *files[4] = {"/0.txt", "/1.txt", "/2.txt", "/3.txt"};
    for (int i = 0; i < 4; i++) {
        file = LittleFS.open(files[i], "r");
        while (file.available()) {
            base64_vector[i] = file.readString();
        }
        file.close();
        LittleFS.remove(files[i]);
        LittleFS.remove("/reset_count.txt");
    }
}

int get_reset_count() {
    int reset_count = -1;
    LittleFS.begin();
    if (LittleFS.exists("/reset_count.txt")) {
        File file;
        file = LittleFS.open("/reset_count.txt", "r");
        reset_count = file.readString().toInt();
        file.close();
    }
    return reset_count;
}