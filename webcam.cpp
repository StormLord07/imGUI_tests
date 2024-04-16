#include "webcam.h"

#include <locale.h>

void error(HRESULT hr, const std::wstring& message) {
    if (FAILED(hr)) {
        LPWSTR lpMsgBuf;
        DWORD  bufLen = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, hr,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // English language
            (LPWSTR)&lpMsgBuf, 0, NULL);

        if (bufLen) {
            LPCWSTR      lpMsgStr = (LPCWSTR)lpMsgBuf;
            std::wstring result(lpMsgStr, lpMsgStr + bufLen);

            std::wcerr << message << L" Error: " << hr << L": " << result
                       << std::endl;

            LocalFree(lpMsgBuf);
        }
    }
}

Webcam::Webcam(IMFActivate* device, IMFAttributes* config)
    : device_(device), active_device_(nullptr), source_reader_(nullptr),
      active_(false), media_types_(), config_(config) {
    HRESULT hr = S_OK;
    if (this->device_) {
        this->device_->AddRef();
    }
    if (this->config_) {
        this->config_->AddRef();
    }
    LPWSTR name = nullptr;
    this->device_->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                      &name, NULL);
    this->name_ = name;
    CoTaskMemFree(name);

    hr = this->device_->ActivateObject(IID_PPV_ARGS(&this->active_device_));

    if (SUCCEEDED(hr)) {
        this->active_device_->AddRef();
        hr = MFCreateSourceReaderFromMediaSource(
            this->active_device_, this->config_, &this->source_reader_);
    }

    if (SUCCEEDED(hr)) {
        this->source_reader_->AddRef();
        uint32_t      index      = 0;
        IMFMediaType* media_type = nullptr;
        while (SUCCEEDED(source_reader_->GetNativeMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, index, &media_type))) {
            this->media_types_.push_back(media_type);
            media_type->AddRef();
            media_type = nullptr;
            ++index;
        }
        std::sort(
            this->media_types_.begin(), this->media_types_.end(),
            [](IMFMediaType* a, IMFMediaType* b) {
                UINT32 widthA, heightA, widthB, heightB;
                MFGetAttributeSize(a, MF_MT_FRAME_SIZE, &widthA, &heightA);
                MFGetAttributeSize(b, MF_MT_FRAME_SIZE, &widthB, &heightB);

                GUID encodingA;
                a->GetGUID(MF_MT_SUBTYPE, &encodingA);
                GUID encodingB;
                b->GetGUID(MF_MT_SUBTYPE, &encodingB);

                // If resolutions are not the same, sort by resolution
                if (widthA * heightA != widthB * heightB) {
                    return (widthA * heightA) > (widthB * heightB);
                }

                // If resolutions are the same, MJPEG should come first
                if (encodingA == MFVideoFormat_MJPG &&
                    encodingB != MFVideoFormat_MJPG) {
                    return true;
                } else if (encodingB == MFVideoFormat_MJPG &&
                           encodingA != MFVideoFormat_MJPG) {
                    return false;
                }

                // If resolutions and encodings are the same, it doesn't matter
                // which comes first
                return false;
            });
    }

    if (this->active_device_) {
        this->active_device_->Shutdown();
        this->active_device_->Release();
        this->active_device_ = nullptr;
    }
    if (this->source_reader_) {
        this->source_reader_->Release();
        this->source_reader_ = nullptr;
    }
    this->device_->ShutdownObject();

    error(hr, L"unable to initialize webcam");
}

Webcam::Webcam(const Webcam& other)
    : device_(other.device_), active_device_(other.active_device_),
      source_reader_(other.source_reader_), config_(other.config_),
      media_types_(other.media_types_),
      chosen_media_type_index_(other.chosen_media_type_index_),
      active_(other.active_), name_(other.name_) {

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
    for (auto& media_type : this->media_types_) {
        media_type->AddRef();
    }
}

Webcam::~Webcam() {
    this->active_ = false;
    if (this->source_reader_) {
        this->source_reader_->Release();
        this->source_reader_ = nullptr;
    }
    if (this->active_device_) {
        this->active_device_->Shutdown();
        this->active_device_->Release();
        this->active_device_ = nullptr;
    }
    if (this->config_) {
        this->config_->Release();
        this->config_ = nullptr;
    }
    if (this->device_) {
        this->device_->ShutdownObject();
        this->device_->Release();
        this->device_ = nullptr;
    }
    for (auto media_type : this->media_types_) {
        media_type->Release();
        media_type = nullptr;
    }
    media_types_.clear();
}

Webcam& Webcam::operator=(const Webcam& other) {
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
        for (auto& media_type : this->media_types_) {
            if (media_type) {
                media_type->Release();
            }
        }

        // Copy new resources
        this->device_        = other.device_;
        this->active_device_ = other.active_device_;
        this->source_reader_ = other.source_reader_;
        this->config_        = other.config_;
        this->media_types_   = other.media_types_; // Still assumes shallow copy
        this->chosen_media_type_index_ = other.chosen_media_type_index_;
        this->active_                  = other.active_;
        this->name_                    = other.name_;

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
        for (auto& media_type : this->media_types_) {
            media_type->AddRef();
        }
    }
    return *this;
}

bool Webcam::isActive() const { return this->active_; }

IMFActivate* Webcam::getDevice() const {
    this->device_->AddRef();
    return this->device_;
}

std::wstring Webcam::getName() const { return this->name_; }

void Webcam::setMediaTypeInex(uint16_t index) {
    this->chosen_media_type_index_ = index;
}

int16_t Webcam::activate() {

    if (this->active_) {
        return 304;
    }

    HRESULT hr = S_OK;

    if (this->device_ && !this->active_device_) {
        hr = this->device_->ActivateObject(IID_PPV_ARGS(&this->active_device_));
    }

    if (SUCCEEDED(hr)) {
        this->active_device_->AddRef();
        hr = MFCreateSourceReaderFromMediaSource(
            this->active_device_, this->config_, &this->source_reader_);
    }

    if (SUCCEEDED(hr)) {
        this->source_reader_->AddRef();
        hr = this->source_reader_->SetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL,
            this->media_types_[this->chosen_media_type_index_]);
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
    } else {
        std::wcout << L"Activation succesfull " + this->name_ << std::endl;
    }

    return 0;
}

int16_t Webcam::activate(uint16_t index) {
    if (index >= this->media_types_.size()) {
        return -1; // TODO: temporary error code
    }
    setMediaTypeInex(index);
    return this->activate();
}

int16_t Webcam::deactivate() {
    if (!this->active_) {
        return 304;
    }
    this->active_ = false;
    if (this->active_device_) {
        this->active_device_->Shutdown();
        this->active_device_->Release();
        this->active_device_ = nullptr;
    }
    if (this->source_reader_) {
        this->source_reader_->Release();
        this->source_reader_ = nullptr;
    }
    this->device_->ShutdownObject();

    std::wcout << L"Deactivation succesfull " + this->name_ << std::endl;

    return 0;
}
