#include "webcam.h"

#include <locale.h>

void error(HRESULT hr, const std::wstring &message) {
    if (FAILED(hr)) {
        LPWSTR lpMsgBuf;
        DWORD bufLen = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            hr,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // English language
            (LPWSTR)&lpMsgBuf,
            0, NULL);

        if (bufLen) {
            LPCWSTR lpMsgStr = (LPCWSTR)lpMsgBuf;
            std::wstring result(lpMsgStr, lpMsgStr + bufLen);

            std::wcerr << message << L" Error: " << hr << L": " << result << std::endl;

            LocalFree(lpMsgBuf);
        }
    }
}

// #region WebcamManager

WebcamManager::WebcamManager() {

    this->config_ = nullptr;
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
            this->devices_.emplace_back(devices_temp[i], this->config_);
        }
    }

    CoTaskMemFree(devices_temp);

    error(hr, L"Unable to initialize webcam manager");

}

WebcamManager::~WebcamManager() {
    this->devices_.clear();
    if (this->config_) {
        this->config_->Release();
    }
}

Webcam &WebcamManager::operator[](std::size_t index) {
    return this->devices_[index];
}

const Webcam &WebcamManager::operator[](std::size_t index) const {
    return this->devices_[index];
}

void WebcamManager::rescanDevices() {
    this->devices_.clear();

    IMFActivate **devices_temp = nullptr;
    uint32_t device_count = 0;

    HRESULT hr = MFEnumDeviceSources(this->config_, &devices_temp, &device_count);

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

std::vector<Webcam> WebcamManager::getDevices() const {
    return this->devices_;
}

bool WebcamManager::activateDevice(const std::wstring &name) {
    for (auto &device : this->devices_) {
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

bool WebcamManager::deactivateDevice(const std::wstring &name) {
    for (auto &device : this->devices_) {
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

// #endregion

// #region Webcam

Webcam::Webcam(IMFActivate *device, IMFAttributes *config) : device_(device), active_device_(nullptr), source_reader_(nullptr), active_(false), media_types_(), config_(config) {
    HRESULT hr = S_OK;
    if (this->device_) {
        this->device_->AddRef();
    }
    if (this->config_) {
        this->config_->AddRef();
    }
    LPWSTR name = nullptr;
    this->device_->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, NULL);
    this->name_ = name;
    CoTaskMemFree(name);


    hr = this->device_->ActivateObject(IID_PPV_ARGS(&this->active_device_));

    if (SUCCEEDED(hr)) {
        this->active_device_->AddRef();
        hr = MFCreateSourceReaderFromMediaSource(this->active_device_, this->config_, &this->source_reader_);
    }

    if (SUCCEEDED(hr)) {
        this->source_reader_->AddRef();
        uint32_t index = 0;
        IMFMediaType *media_type = nullptr;
        while (SUCCEEDED(source_reader_->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, index, &media_type))) {
            this->media_types_.push_back(media_type);
            media_type->AddRef();
            media_type = nullptr;
            ++index;
        }
        std::sort(this->media_types_.begin(), this->media_types_.end(), [](IMFMediaType *a, IMFMediaType *b) {
            UINT32 widthA, heightA, widthB, heightB;
            MFGetAttributeSize(a, MF_MT_FRAME_SIZE, &widthA, &heightA);
            MFGetAttributeSize(b, MF_MT_FRAME_SIZE, &widthB, &heightB);

            GUID encodingA; a->GetGUID(MF_MT_SUBTYPE, &encodingA);
            GUID encodingB; b->GetGUID(MF_MT_SUBTYPE, &encodingB);

            // If resolutions are not the same, sort by resolution
            if (widthA * heightA != widthB * heightB) {
                return (widthA * heightA) > (widthB * heightB);
            }

            // If resolutions are the same, MJPEG should come first
            if (encodingA == MFVideoFormat_MJPG && encodingB != MFVideoFormat_MJPG) {
                return true;
            }
            else if (encodingB == MFVideoFormat_MJPG && encodingA != MFVideoFormat_MJPG) {
                return false;
            }

            // If resolutions and encodings are the same, it doesn't matter which comes first
            return false;
            });

    }
    this->device_->ShutdownObject();
    if (this->active_device_) {
        this->active_device_->Shutdown();
        this->active_device_->Release();
        this->active_device_ = nullptr;
    }
    if (this->source_reader_) {
        this->source_reader_->Release();
        this->source_reader_ = nullptr;
    }

    error(hr, L"unable to initialize webcam");
}

Webcam::Webcam(const Webcam &other)
    : device_(other.device_),
    active_device_(other.active_device_),
    source_reader_(other.source_reader_),
    config_(other.config_),
    media_types_(other.media_types_),
    chosen_subtype_index_(other.chosen_subtype_index_),
    active_(other.active_),
    name_(other.name_) {

    if (this->device_) {
        this->device_->AddRef();
    }
    if (this->config_) {
        this->config_->AddRef();
    }
    if (this->active_device_) {
        this->active_device_->AddRef();
    }
    if (this->source_reader_) {
        this->source_reader_->AddRef();
    }
    for (auto &media_type : this->media_types_) {
        media_type->AddRef();
    }
}

Webcam &Webcam::operator=(const Webcam &other) {
    if (this != &other) {
        // Release old resources
        if (this->device_) {
            this->device_->Release();
        }
        if (this->config_) {
            this->config_->Release();
        }
        if (this->active_device_) {
            this->active_device_->Release();
        }
        if (this->source_reader_) {
            this->source_reader_->Release();
        }
        for (auto &media_type : this->media_types_) {
            if (media_type) {
                media_type->Release();
            }
        }

        // Copy new resources
        this->device_ = other.device_;
        this->active_device_ = other.active_device_;
        this->source_reader_ = other.source_reader_;
        this->config_ = other.config_;
        this->media_types_ = other.media_types_; // Still assumes shallow copy
        this->chosen_subtype_index_ = other.chosen_subtype_index_;
        this->active_ = other.active_;
        this->name_ = other.name_;

        // Add reference to new resources
        if (this->device_) {
            this->device_->AddRef();
        }
        if (this->config_) {
            this->config_->AddRef();
        }
        if (this->active_device_) {
            this->active_device_->AddRef();
        }
        if (this->source_reader_) {
            this->source_reader_->AddRef();
        }
        for (auto &media_type : this->media_types_) {
            media_type->AddRef();
        }
    }
    return *this;
}

Webcam::~Webcam() {
    if (this->source_reader_) {
        this->source_reader_->Release();
    }
    if (this->active_device_) {
        this->active_device_->Release();
    }
    if (this->config_) {
        this->config_->Release();
    }
    if (this->device_) {
        this->device_->Release();
    }
    for (auto media_type : this->media_types_) {
        media_type->Release();
    }
}

bool Webcam::isActive() const {
    return this->active_;
}

IMFActivate *Webcam::getDevice() const {
    this->device_->AddRef();
    return this->device_;
}

std::wstring Webcam::getName() const {
    return this->name_;
}

HRESULT Webcam::activate() {
    HRESULT hr = S_OK;

    if (this->device_ && !this->active_device_) {
        hr = this->device_->ActivateObject(IID_PPV_ARGS(&this->active_device_));
    }

    if (SUCCEEDED(hr)) {
        this->active_device_->AddRef();
        hr = MFCreateSourceReaderFromMediaSource(this->active_device_, this->config_, &this->source_reader_);
    }

    if (SUCCEEDED(hr)) {
        this->source_reader_->AddRef();
        hr = this->source_reader_->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, this->media_types_[0]);
        this->active_ = true;
    }

    if (FAILED(hr)) {
        if (this->active_device_) {
            this->active_device_->Shutdown();
            this->active_device_ = nullptr;
        }
        if (this->source_reader_) {
            this->source_reader_->Release();
            this->source_reader_ = nullptr;
        }
        error(hr, L"unable to activate webcam " + this->name_);
    }

    return hr;
}

HRESULT Webcam::deactivate() {
    return S_OK;
}

// #endregion