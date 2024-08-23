#include "hardware/webcam_manager.h"

class DeviceManager {
  public:
    DeviceManager();
    ~DeviceManager();

    // Initialize Media Foundation
    HRESULT Initialize();

    // Enumerate audio devices
    std::vector<IMFMediaDevice*> EnumerateAudioDevices();

    // Enumerate webcam devices
    std::vector<IMFMediaSource*> EnumerateWebcamDevices();

    // Get a specific audio device
    IMFMediaDevice* GetAudioDevice(int index);

    // Get a specific webcam device
    IMFMediaSource* GetWebcamDevice(int index);

  private:
    // Helper methods
    HRESULT InitializeMediaFoundationS();
    void    Cleanup();

    IMFAttributes*       m_pAttributes;  // For device attributes
    IMFMediaSource*      m_pMediaSource; // For media sources
    HardwareVideoManager webcam_manager_;
    HardwareAudioManager audio_manager_;
};