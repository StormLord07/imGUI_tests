#ifndef WEBCAM_H
#define WEBCAM_H

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <comdef.h>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>

void error(HRESULT hr, const std::wstring &message = L"");

class Webcam {
private:
    IMFActivate *device_{ nullptr };
    IMFMediaSource *active_device_{ nullptr };
    IMFSourceReader *source_reader_{ nullptr };
    IMFAttributes *config_{ nullptr };
    std::wstring name_{};

    std::vector<IMFMediaType *> media_types_{};

    uint16_t chosen_subtype_index_{};
    bool active_{ false };

public:
    Webcam(IMFActivate *device, IMFAttributes *config = nullptr);
    Webcam(const Webcam &other);
    Webcam &operator=(const Webcam &other);
    ~Webcam();

    bool isActive() const;


    HRESULT activate();
    HRESULT deactivate();

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

    /**
     * @brief Updates the list of connected cameras
     *
     */
    void rescanDevices();

    /**
     * @brief Get the names of connected cameras
     *
     * @return std::vector<std::wstring>
     */
    std::vector<std::wstring> getDeviceNames() const;

    /**
     * @brief Get the list of connected cameras
     *
     * @return std::vector<Webcam>
     */
    std::vector<Webcam> getDevices() const;

    /**
     * @brief Activate a device by name.
     * @param wstring The name of the device to activate.
     * @return true if the device was activated, false otherwise.
     */
    bool activateDevice(const std::wstring &name);

    /**
     * @brief Activate a device by index.
     * @param index The index of the device to activate.
     * @return `true` if the device was activated, false otherwise.
     */
    bool activateDevice(std::size_t index);

    /**
     * @brief Activate a device by name.
     * @param wstring The name of the device to activate.
     * @return true if the device was activated, false otherwise.
     */
    bool deactivateDevice(const std::wstring &name);

    /**
     * @brief Deactivate a device by index.
     * @param index The index of the device to activate.
     * @return true if the device was activated, false otherwise.
     */
    bool deactivateDevice(std::size_t index);
};

#endif // WEBCAM_H