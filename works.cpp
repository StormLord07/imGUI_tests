#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <iostream>

// Helper function for initializing Media Foundation
HRESULT InitializeMediaFoundation() {
    // Initialize COM
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM: " << std::hex << hr << std::endl;
        return hr;
    }

    // Initialize Media Foundation
    hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize Media Foundation: " << std::hex << hr << std::endl;
        CoUninitialize();
        return hr;
    }

    return S_OK;
}

// Helper function for shutting down Media Foundation
void ShutdownMediaFoundation() {
    MFShutdown();
    CoUninitialize();
}

class MediaSourceManager {
public:
    MediaSourceManager(IMFActivate *pActivator)
        : m_pActivator(pActivator), m_pMediaSource(nullptr) {
        if (m_pActivator)
            m_pActivator->AddRef();
    }

    ~MediaSourceManager() {
        ShutdownMediaSource();
        if (m_pActivator)
            m_pActivator->Release();
    }

    HRESULT CreateMediaSource() {
        ShutdownMediaSource();

        if (m_pActivator) {
            HRESULT hr = m_pActivator->ActivateObject(__uuidof(IMFMediaSource), reinterpret_cast<void **>(&m_pMediaSource));
            return hr;
        }
        return E_UNEXPECTED;
    }

    void ShutdownMediaSource() {
        if (m_pMediaSource) {
            m_pMediaSource->Shutdown();
            m_pMediaSource->Release();
            m_pMediaSource = nullptr;
        }
    }

private:
    IMFActivate *m_pActivator;
    IMFMediaSource *m_pMediaSource;
};

HRESULT GetFirstVideoCaptureDevice(IMFActivate **ppActivate) {
    IMFAttributes *pAttributes = nullptr;
    IMFActivate **ppDevices = nullptr;
    UINT32 count = 0;
    HRESULT hr = MFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr)) {
        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (SUCCEEDED(hr)) {
            hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
            if (SUCCEEDED(hr) && count > 0) {
                // Get the first available video capture device
                *ppActivate = ppDevices[0];
                (*ppActivate)->AddRef();

                // Release all other devices
                for (UINT32 i = 1; i < count; ++i) {
                    ppDevices[i]->Release();
                }
            }
            else {
                hr = E_FAIL; // No devices found or failed to enumerate devices
            }
            CoTaskMemFree(ppDevices);
        }
        pAttributes->Release();
    }
    return hr;
}

int main() {
    HRESULT hr = InitializeMediaFoundation();
    if (FAILED(hr)) {
        return 1;
    }

    IMFActivate *pActivator = nullptr;
    hr = GetFirstVideoCaptureDevice(&pActivator);
    if (SUCCEEDED(hr)) {
        std::cout << "Video capture device obtained successfully." << std::endl;

        // Example usage of MediaSourceManager
        MediaSourceManager manager(pActivator);

        hr = manager.CreateMediaSource();
        if (SUCCEEDED(hr)) {
            std::cout << "Media source created successfully." << std::endl;
        }

        // Use the media source as needed, then shut it down
        manager.ShutdownMediaSource();

        // Recreate the media source when needed
        hr = manager.CreateMediaSource();
        if (SUCCEEDED(hr)) {
            std::cout << "Media source recreated successfully." << std::endl;
        }

        pActivator->Release();
    }
    else {
        std::cerr << "Failed to obtain a video capture device." << std::endl;
    }

    ShutdownMediaFoundation();
    return 0;
}