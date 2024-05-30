#include "webcam_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
    HRESULT hr = S_OK;
    hr         = CoInitialize(0);
    hr         = MFStartup(MF_VERSION);
    error(hr, L"Failed to start Media Foundation.");
    WebcamManager camera_manager{};
    // camera_manager.cloneDevice(0);
    auto device_names = camera_manager.getDeviceNames();

    if (device_names.empty()) {
        std::cout << "No webcam devices found." << std::endl;
    } else {
        std::cout << "Webcam devices found:" << std::endl;
        for (const auto& name : device_names) {
            std::wcout << L"- " << name << std::endl;
        }
        auto cam = camera_manager.getDevices()[0];
        std::wcout << std::endl << std::endl;
        std::wcout << L"Picked camera: " << cam.getName() << std::endl;
        cam.listMediaTypes();
        cam.printSelectedMediaType();

        std::wcout << L"Activating" << std::endl;
        cam.activate();
        Sleep(5000);
        for (int i = 0; i < 300; ++i) {
            std::wstringstream ws;
            ws << L"output/file" << i << L".jpg";
            std::wstring filePath = ws.str();

            cam.saveFrame(filePath);
        }
    }

    hr = MFShutdown();
    CoUninitialize();
    return 0;
}