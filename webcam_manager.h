#ifndef WEBCAM_MANAGER_H
#define WEBCAM_MANAGER_H

#include "webcam.h"

class WebcamManager {
  private:
    std::vector<Webcam> devices_{};
    IMFAttributes*      config_{nullptr};

  public:
    WebcamManager();
    ~WebcamManager();

    Webcam&       operator[](std::size_t index);
    const Webcam& operator[](std::size_t index) const;

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
    bool activateDevice(const std::wstring& name);

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
    bool deactivateDevice(const std::wstring& name);

    /**
     * @brief Deactivate a device by index.
     * @param index The index of the device to activate.
     * @return true if the device was activated, false otherwise.
     */
    bool deactivateDevice(std::size_t index);
};

#endif // WEBACM_MANAGER_H