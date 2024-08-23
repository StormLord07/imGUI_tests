#include "webcam_manager.h"

WebcamManager::WebcamManager() {

    this->config_              = nullptr;
    IMFActivate** devices_temp = nullptr;
    uint32_t      device_count = 0;

    HRESULT hr = MFCreateAttributes(&this->config_, 1);
    if (SUCCEEDED(hr)) {
        hr = this->config_->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    }

    if (SUCCEEDED(hr)) {
        hr = MFEnumDeviceSources(this->config_, &devices_temp, &device_count);
    }

    if (SUCCEEDED(hr)) {
        for (uint32_t i = 0; i < device_count; i++) {
            this->devices_.emplace_back(devices_temp[i], this->config_);
        }
    }

    CoTaskMemFree(devices_temp);

    error(hr, L"Unable to initialize webcam manager");
}

void WebcamManager::cloneDevice(std::size_t index) {
    HRESULT        hr     = S_OK;
    IMFAttributes* attrs  = NULL;
    IMFActivate*   cloned = NULL;
    // Get attributes from the original IMFActivate
    hr =
        this->devices_[index].getDevice()->QueryInterface(IID_PPV_ARGS(&attrs));
    if (SUCCEEDED(hr)) {
        // Create a new device source activation object with these attributes
        hr = MFCreateDeviceSourceActivate(attrs, &cloned);
        attrs->Release();
    }
    Webcam new_device(cloned, this->config_);
    if (index < this->devices_.size()) {
        this->devices_.insert(devices_.begin() + index + 1,
                              std::move(new_device));
    }
}

WebcamManager::~WebcamManager() {
    this->devices_.clear();
    if (this->config_) {
        this->config_->Release();
    }
}

Webcam& WebcamManager::operator[](std::size_t index) {
    return this->devices_[index];
}

const Webcam& WebcamManager::operator[](std::size_t index) const {
    return this->devices_[index];
}

void WebcamManager::rescanDevices() {
    this->devices_.clear();

    IMFActivate** devices_temp = nullptr;
    uint32_t      device_count = 0;

    HRESULT hr =
        MFEnumDeviceSources(this->config_, &devices_temp, &device_count);

    if (SUCCEEDED(hr)) {
        for (uint32_t i = 0; i < device_count; i++) {
            this->devices_.emplace_back(devices_temp[i]);
        }
    }

    CoTaskMemFree(devices_temp);

    error(hr, L"Unable to rescan devices");
}

std::vector<std::wstring> WebcamManager::getDeviceNames() const {
    std::vector<std::wstring> device_names;
    for (auto device : this->devices_) {
        device_names.push_back(device.getName());
    }
    return device_names;
}

std::vector<Webcam> WebcamManager::getDevices() const { return this->devices_; }

bool WebcamManager::activateDevice(const std::wstring& name) {
    for (auto& device : this->devices_) {
        if (device.getName() == name) {
            return SUCCEEDED(device.activate());
        }
    }
    return false;
}

bool WebcamManager::activateDevice(std::size_t index) {
    if (index < this->devices_.size()) {
        return SUCCEEDED(this->devices_[index].activate());
    }
    return false;
}

bool WebcamManager::deactivateDevice(const std::wstring& name) {
    for (auto& device : this->devices_) {
        if (device.getName() == name) {
            return SUCCEEDED(device.deactivate());
        }
    }
    return false;
}

bool WebcamManager::deactivateDevice(std::size_t index) {
    if (index < this->devices_.size()) {
        return SUCCEEDED(this->devices_[index].deactivate());
    }
    return false;
}
