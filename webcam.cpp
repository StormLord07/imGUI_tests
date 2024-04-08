#include "webcam.h"

webcam::webcam() {

    config_ = nullptr;
    IMFActivate **devices_temp = nullptr;
    uint32_t device_count = 0;

    HRESULT hr = MFCreateAttributes(&this->config_, 1);
    if (SUCCEEDED(hr)) {
        hr = this->config_->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
        );
    }

    if (SUCCEEDED(hr)) {
        hr = MFEnumDeviceSources(this->config_, &devices_temp, &device_count);
    }

    if (SUCCEEDED(hr)) {
        for (uint32_t i = 0; i < device_count; i++) {
            this->devices_.push_back(devices_temp[i]);
            devices_temp[i]->AddRef();
        }
    }

    CoTaskMemFree(devices_temp);

    if (FAILED(hr)) {
        std::cerr << "Error: " << hr << "\n";
    }

}

webcam::~webcam() {
}

void webcam::rescan_devices() {
    for (auto device : this->devices_) {
        device->Release();
    }
    this->devices_.clear();

    IMFActivate **devices_temp = nullptr;
    uint32_t device_count = 0;

    HRESULT hr = MFEnumDeviceSources(this->config_, &devices_temp, &device_count);

    if (SUCCEEDED(hr)) {
        for (uint32_t i = 0; i < device_count; i++) {
            this->devices_.push_back(devices_temp[i]);
            devices_temp[i]->AddRef();
        }
    }

    CoTaskMemFree(devices_temp);

    if (FAILED(hr)) {
        std::cerr << "Error: " << hr << "\n";
    }
}

std::vector<IMFActivate *> webcam::get_devices() const {
    return this->devices_;
}

std::vector<std::wstring> webcam::get_device_names() const {
    std::vector<std::wstring> device_names;
    for (auto device : this->devices_) {
        LPWSTR name = nullptr;
        HRESULT hr = device->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
            &name,
            nullptr
        );
        if (SUCCEEDED(hr)) {
            device_names.push_back(name);
            CoTaskMemFree(name);
        }
    }
    return device_names;
}
