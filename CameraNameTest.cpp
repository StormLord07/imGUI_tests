#include "webcam.h"
#include <iostream>
#include <fstream>


int main() {
    HRESULT hr = MFStartup(MF_VERSION);
    error(hr, L"Failed to start Media Foundation.");
    WebcamManager camera_manager{};
    auto device_names = camera_manager.getDeviceNames();

    if (device_names.empty()) {
        std::cout << "No webcam devices found." << std::endl;
    }
    else {
        std::cout << "Webcam devices found:" << std::endl;
        for (const auto &name : device_names) {
            std::wcout << L"- " << name << std::endl;
        }
        auto cam = camera_manager[0];
        // std::wcout << L"Picked camera: " << cam.getName() << std::endl;
        cam.activate();
    }

    hr = MFShutdown();

    return 0;
}