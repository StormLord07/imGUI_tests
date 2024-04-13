#include "webcam.h"

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

    if (FAILED(hr)) {
        _com_error err(hr);
        std::wcerr << L"Error: " << hr << ": " << err.ErrorMessage() << std::endl << "\n";
    }

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

    if (FAILED(hr)) {
        _com_error err(hr);
        std::wcerr << L"Error: " << hr << ": " << err.ErrorMessage() << std::endl << "\n";
    }
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

// #endregion

// #region Webcam

Webcam::Webcam(IMFActivate *device, IMFAttributes *config) : device_(device), active_device_(nullptr), media_type_(nullptr), source_reader_(nullptr), active_(false), info_(), config_(config) {
    HRESULT hr = S_OK;
    if (this->device_) {
        this->device_->AddRef();
    }
    if (this->config_) {
        this->config_->AddRef();
    }
    LPWSTR name = nullptr;
    this->device_->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, NULL);
    this->info_.name_ = name;

    hr = device_->ActivateObject(IID_PPV_ARGS(&this->active_device_));

    if (SUCCEEDED(hr)) {
        hr = MFCreateSourceReaderFromMediaSource(this->active_device_, this->config_, &this->source_reader_);
    }

    if (SUCCEEDED(hr)) {
        this->source_reader_->AddRef();
        uint32_t index = 0;
        while (SUCCEEDED(source_reader_->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, index, &this->media_type_))) {
            setInfo(this->media_type_, index);
            this->media_type_ = nullptr;
            ++index;
        }
    }

    if (this->active_device_) {
        this->active_device_->Shutdown();
        this->active_device_ = nullptr;
    }
    if (this->media_type_) {
        this->media_type_->Release();
        this->media_type_ = nullptr;
    }
    if (this->source_reader_) {
        this->source_reader_->Release();
        this->source_reader_ = nullptr;
    }


    CoTaskMemFree(name);

    if (FAILED(hr)) {
        _com_error err(hr);
        std::wcerr << L"Error: " << hr << ": " << err.ErrorMessage() << std::endl << "\n";
    }
}

Webcam::Webcam(const Webcam &other)
    : device_(other.device_), active_device_(other.active_device_), media_type_(other.media_type_),
    source_reader_(other.source_reader_), config_(other.config_),
    info_(other.info_), chosen_subtype_index_(other.chosen_subtype_index_),
    active_(other.active_) {
    if (device_) {
        device_->AddRef();
    }
    if (config_) {
        config_->AddRef();
    }
    if (active_device_) {
        active_device_->AddRef();
    }
    if (source_reader_) {
        source_reader_->AddRef();
    }
}

Webcam &Webcam::operator=(const Webcam &other) {
    if (this != &other) {
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

        this->device_ = other.device_;
        this->config_ = other.config_;
        this->active_device_ = other.active_device_;
        this->source_reader_ = other.source_reader_;
        this->info_.name_ = other.info_.name_;
        this->info_.major_type_ = other.info_.major_type_;
        this->info_.supported_subtypes_ = other.info_.supported_subtypes_;
        this->info_.resolutions_ = other.info_.resolutions_;
        this->info_.frame_rates_ = other.info_.frame_rates_;
        this->chosen_subtype_index_ = other.chosen_subtype_index_;
        this->active_ = other.active_;

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
    }
    return *this;
}

Webcam::~Webcam() {
    if (this->source_reader_) {
        this->source_reader_->Release();
    }
    if (this->media_type_) {
        this->media_type_->Release();
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
}

bool Webcam::isActive() const {
    return this->active_;
}

void Webcam::setInfo(IMFMediaType *media_type, uint32_t index) {
    GUID subType = {};
    uint32_t width = 0, height = 0;
    uint32_t numerator = 0, denominator = 0;

    media_type->GetGUID(MF_MT_SUBTYPE, &subType);
    MFGetAttributeSize(media_type, MF_MT_FRAME_SIZE, &width, &height);
    MFGetAttributeRatio(media_type, MF_MT_FRAME_RATE, &numerator, &denominator);

    this->info_.supported_subtypes_.push_back(subType);
    this->info_.resolutions_.emplace_back(width, height);
    this->info_.frame_rates_.emplace_back(numerator, denominator);
}

IMFActivate *Webcam::getDevice() const {
    this->device_->AddRef();
    return this->device_;
}

std::wstring Webcam::getName() const {
    return this->info_.name_;
}

// #endregion