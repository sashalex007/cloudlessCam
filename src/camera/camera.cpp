#include <ArduCAM.h>
#include <base64.h>
#include <SPI.h>
#include <Wire.h>
#include <esp32-hal.h>
#include "camera.h"

void Camera::capture(String& base64_string) {
    byte buff[255];
    bool skip = false;
    static int i = 0;
    uint8_t temp = 0, temp_last = 0;
    uint32_t length = 0;
    bool is_header = false;

    // Flush the FIFO
    cam.flush_fifo();
    // Clear the capture done flag
    cam.clear_fifo_flag();
    // Start capture
    cam.start_capture();

    Serial.println(F("Start Capture"));
    while (!cam.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
        //
    }
    Serial.println(F("Capture Done."));
    length = cam.read_fifo_length();
    Serial.print(F("The fifo length is :"));
    Serial.println(length, DEC);

    if (length >= MAX_FIFO_SIZE)  // 8M
    {
        Serial.println(F("Over size."));
        skip = true;
    }
    if (length == 0)  // 0 kb
    {
        Serial.println(F("Size is 0."));
        skip = true;
    }

    cam.CS_LOW();
    cam.set_fifo_burst();
    if (!skip) {
        while (length--) {
            temp_last = temp;
            temp = SPI.transfer(0x00);
            // Read JPEG data from FIFO
            if ((temp == 0xD9) && (temp_last == 0xFF))  // If find the end ,break while,
            {
                buff[i++] = temp;
                // Write the remain bytes in the buffer
                cam.CS_HIGH();
                base64_string += base64::encode(buff, i);
                is_header = false;
                i = 0;
            }
            if (is_header == true) {
                // Write image data to buffer if not full
                if (i < 255) {
                    buff[i++] = temp;
                } else {
                    // Write 256 bytes image data to file
                    cam.CS_HIGH();
                    base64_string += base64::encode(buff, 255);
                    i = 0;
                    buff[i++] = temp;
                    cam.CS_LOW();
                    cam.set_fifo_burst();
                }
            } else if ((temp == 0xD8) & (temp_last == 0xFF)) {
                is_header = true;
                buff[i++] = temp_last;
                buff[i++] = temp;
            }
        }
    } else {
        cam.CS_HIGH();
    }
}

void set_exposure(ArduCAM& cam) {
    uint8_t R4;   // Register 4: bits 0 and 1 correspond to bits 0 and 1 of exposure multiplier
    uint8_t R10;  // Register 10: the 8 bits correspond to bits 2-9 of exposure multiplier
    uint8_t R13;  // Register 13: bit 0 switches auto exposure control on(1)/off(0); bit 2 switches auto gain control on(1)/off(0)
    uint8_t R45;  // Register 45: bits 0-5 correspond to bits 10-15 of exposure multiplier
    cam.wrSensorReg8_8(0xFF, 0x01);
    cam.rdSensorReg8_8(0x13, &R13);
    cam.rdSensorReg8_8(0x04, &R4);
    cam.rdSensorReg8_8(0x10, &R10);
    cam.rdSensorReg8_8(0x45, &R45);

    // uint16_t Exp[]={50,500,5000,50000};
    uint16_t Exp = 25000;                // Exposure time in us
    uint16_t a = round(Exp / 53.39316);  // DEC number corresponding to exposure time
    uint16_t r4shift = 14;
    uint16_t r10leftshift = 6;
    uint16_t r10rightshift = 8;
    uint16_t r45leftshift = 10;
    uint16_t b = a << r4shift;
    uint8_t r4 = b >> r4shift;                           // Extracts bits 0 and 1
    uint8_t r10 = (a << r10leftshift) >> r10rightshift;  // Extracts bits 2-9
    uint8_t r45 = a >> r45leftshift;                     // Extracts bits 10-15

    for (int i = 0; i < 2; i++) {
        bitWrite(R4, i, bitRead(r4, i));
    }
    for (int i = 0; i < 8; i++) {
        bitWrite(R10, i, bitRead(r10, i));
    }
    for (int i = 0; i < 6; i++) {
        bitWrite(R45, i, bitRead(r45, i));
    }
    bitClear(R13, 0);

    cam.wrSensorReg8_8(0x13, (int)R13);
    cam.wrSensorReg8_8(0x04, (int)R4);
    cam.wrSensorReg8_8(0x10, (int)R10);
    cam.wrSensorReg8_8(0x45, (int)R45);
    uint8_t new13, new4, new10, new45;
    cam.rdSensorReg8_8(0x13, &new13);
    cam.rdSensorReg8_8(0x04, &new4);
    cam.rdSensorReg8_8(0x10, &new10);
    cam.rdSensorReg8_8(0x45, &new45);
}

void init(ArduCAM& cam, int camera_signal, int camera_power) {
    uint8_t temp;
    // set the CS as an output:
    pinMode(camera_signal, OUTPUT);
    pinMode(camera_power, OUTPUT);
    digitalWrite(camera_power, HIGH);
    Wire.begin();
    // initialize SPI:
    SPI.begin();
    SPI.setFrequency(4000000);  // 4MHz

    // Reset the CPLD
    cam.write_reg(0x07, 0x80);
    delay(100);
    cam.write_reg(0x07, 0x00);
    delay(100);
    // Check if the ArduCAM SPI bus is OK
    cam.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = cam.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
        Serial.println(temp, HEX);
        Serial.println(F("SPI1 interface Error!"));
        ESP.restart();
    }
    // Change to JPEG capture mode and initialize the OV2640 module
    cam.set_format(JPEG);
    cam.InitCAM();
    set_exposure(cam);
    cam.OV2640_set_JPEG_size(OV2640_800x600);
    cam.clear_fifo_flag();
    delay(500);  // delay for camera sensor initialization
}

Camera::Camera(int camera_signal, int camera_power) {
    cam = {OV2640, camera_signal};
    init(cam, camera_signal, camera_power);
}
