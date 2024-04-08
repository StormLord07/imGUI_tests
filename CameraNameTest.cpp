#include "webcam.h"
#include <iostream>

int main() {
    webcam cam;
    auto device_names = cam.get_device_names();

    if (device_names.empty()) {
        std::cout << "No webcam devices found." << std::endl;
    }
    else {
        std::cout << "Webcam devices found:" << std::endl;
        for (const auto &name : device_names) {
            std::wcout << L"- " << name << std::endl;
        }
    }

    return 0;
}
