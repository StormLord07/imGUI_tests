#ifndef WEBCAM_H
#define WEBCAM_H

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <comdef.h>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

struct WebcamInfo {
    std::wstring name_{};
    GUID major_type_ = MFMediaType_Video;
    std::vector<GUID> supported_subtypes_{};
    std::vector<std::pair<uint16_t, uint16_t>> resolutions_{};
    std::vector<std::pair<uint16_t, uint16_t>> frame_rates_{};
};

class Webcam {
private:
    IMFActivate *device_{ nullptr };
    IMFMediaSource *active_device_{ nullptr };
    IMFMediaType *media_type_{ nullptr };
    IMFSourceReader *source_reader_{ nullptr };
    IMFAttributes *config_{ nullptr };

    WebcamInfo info_{};

    uint16_t chosen_subtype_index_{};
    bool active_{ false };

    void setInfo(IMFMediaType *media_type, uint32_t index);

public:
    Webcam(IMFActivate *device, IMFAttributes *config = nullptr);
    Webcam(const Webcam &other);
    Webcam &operator=(const Webcam &other);
    ~Webcam();

    bool isActive() const;

    IMFActivate *getDevice() const;
    std::wstring getName() const;
};

class WebcamManager {
private:
    std::vector<Webcam> devices_{};
    IMFAttributes *config_{ nullptr };

public:
    WebcamManager();
    ~WebcamManager();

    Webcam &operator[](std::size_t index);

    const Webcam &operator[](std::size_t index) const;

    void rescanDevices();

    std::vector<std::wstring> getDeviceNames() const;
    std::vector<Webcam> getDevices() const;

};

#endif // WEBCAM_H