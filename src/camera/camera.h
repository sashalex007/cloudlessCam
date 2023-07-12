#ifndef CAMERA_H
#define CAMERA_H

#include <ArduCAM.h>

class Camera {
    private:
       ArduCAM cam;
    public:
        explicit Camera(int camera_signal, int camera_power);
        void capture(String& base64_string);
};

#endif