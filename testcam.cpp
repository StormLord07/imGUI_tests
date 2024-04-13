#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <iostream>
#include <comdef.h> // For _com_error
#include <iomanip>
#include <map>

// Utility function to check HRESULT and handle errors
void CHK(HRESULT hr) {
    if (FAILED(hr)) {
        _com_error err(hr);
        std::wcerr << L"Error: " << err.ErrorMessage() << std::endl;
        exit(hr);
    }
}

void PrintGUID(const GUID &guid) {
    std::wcout << std::hex << std::uppercase
        << std::setw(8) << guid.Data1 << '-'
        << std::setw(4) << guid.Data2 << '-'
        << std::setw(4) << guid.Data3 << '-'
        << std::setw(2) << static_cast<short>(guid.Data4[0])
        << std::setw(2) << static_cast<short>(guid.Data4[1]) << '-'
        << std::setw(2) << static_cast<short>(guid.Data4[2])
        << std::setw(2) << static_cast<short>(guid.Data4[3])
        << std::setw(2) << static_cast<short>(guid.Data4[4])
        << std::setw(2) << static_cast<short>(guid.Data4[5])
        << std::setw(2) << static_cast<short>(guid.Data4[6])
        << std::setw(2) << static_cast<short>(guid.Data4[7]);
}


// Additional function to convert GUID to string
std::wstring GuidToString(const GUID &guid) {
    wchar_t buf[39]; // Length for GUID string with braces and null-terminator
    StringFromGUID2(guid, buf, 39);
    return std::wstring(buf);
}


struct GuidComparer {
    bool operator()(const GUID &lhs, const GUID &rhs) const {
        return memcmp(&lhs, &rhs, sizeof(GUID)) < 0;
    }
};


// Mapping function to convert Media Foundation GUIDs to readable strings
std::wstring GetGuidName(const GUID &guid) {
    static const std::map<GUID, std::wstring, GuidComparer> guidMap = {
        { MFMediaType_Audio, L"Audio" },
        { MFMediaType_Video, L"Video" },
        { MFMediaType_HTML, L"HTML" },
        { MFMediaType_Binary, L"Binary" },
        { MFMediaType_FileTransfer, L"File Transfer" },
        { MFMediaType_Image, L"Image" },
        { MFMediaType_Stream, L"Stream" },
        { MFVideoFormat_AI44, L"AI44" },
        { MFVideoFormat_ARGB32, L"ARGB32" },
        { MFVideoFormat_AYUV, L"AYUV" },
        { MFVideoFormat_DV25, L"DV25" },
        { MFVideoFormat_DV50, L"DV50" },
        { MFVideoFormat_DVH1, L"DVH1" },
        { MFVideoFormat_DVSD, L"DVSD" },
        { MFVideoFormat_DVSL, L"DVSL" },
        { MFVideoFormat_H264, L"H264" },
        { MFVideoFormat_I420, L"I420" },
        { MFVideoFormat_IYUV, L"IYUV" },
        { MFVideoFormat_NV11, L"NV11" },
        { MFVideoFormat_NV12, L"NV12" },
        { MFVideoFormat_RGB24, L"RGB24" },
        { MFVideoFormat_RGB32, L"RGB32" },
        { MFVideoFormat_RGB555, L"RGB555" },
        { MFVideoFormat_RGB565, L"RGB565" },
        { MFVideoFormat_RGB8, L"RGB8" },
        { MFVideoFormat_UYVY, L"UYVY" },
        { MFVideoFormat_v210, L"v210" },
        { MFVideoFormat_v410, L"v410" },
        { MFVideoFormat_WMV1, L"WMV1" },
        { MFVideoFormat_WMV2, L"WMV2" },
        { MFVideoFormat_WMV3, L"WMV3" },
        { MFVideoFormat_WVC1, L"WVC1" },
        { MFVideoFormat_YUY2, L"YUY2" },
        { MFVideoFormat_YV12, L"YV12" },
        { MFAudioFormat_PCM, L"PCM" },
        { MFAudioFormat_Float, L"Float" },
        { MFAudioFormat_DTS, L"DTS" },
        { MFAudioFormat_Dolby_AC3_SPDIF, L"Dolby AC3 SPDIF" },
        { MFAudioFormat_DRM, L"DRM" },
        { MFAudioFormat_WMAudioV8, L"WMAudioV8" },
        { MFAudioFormat_WMAudioV9, L"WMAudioV9" },
        { MFAudioFormat_WMAudio_Lossless, L"WMAudio Lossless" },
        { MFAudioFormat_WMASPDIF, L"WMASPDIF" },
        { MFAudioFormat_MP3, L"MP3" },
        { MFAudioFormat_MPEG, L"MPEG" }
    };

    auto it = guidMap.find(guid);
    if (it != guidMap.end()) {
        return L"Type: " + it->second + L" GUID: " + GuidToString(guid);
    }
    else {
        return L"Unknown Type (" + GuidToString(guid) + L")";
    }
}

// Function to print media type details in a more readable format
void PrintMediaType(IMFMediaType *pType) {
    GUID majorType = {};
    GUID subType = {};
    UINT32 width = 0, height = 0;
    UINT32 numerator = 0, denominator = 0;

    pType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
    pType->GetGUID(MF_MT_SUBTYPE, &subType);
    MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
    MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &numerator, &denominator);

    std::wcout << L"Major Type: " << GetGuidName(majorType) << std::endl;
    std::wcout << L"Sub Type: " << GetGuidName(subType) << std::endl;
    std::wcout << L"Resolution: " << width << L"x" << height << std::endl;
    std::wcout << L"Frame Rate: " << numerator << L"/" << denominator << std::endl;
}


int main() {
    HRESULT hr = S_OK;
    IMFAttributes *pAttributes = nullptr;
    IMFActivate **ppDevices = nullptr;
    UINT32 count = 0;

    CHK(MFStartup(MF_VERSION));

    // Create attributes to specify video capture devices
    CHK(MFCreateAttributes(&pAttributes, 1));
    CHK(pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));

    // Enumerate the video devices
    CHK(MFEnumDeviceSources(pAttributes, &ppDevices, &count));

    if (count == 0) {
        std::cout << "No video devices found." << std::endl;
    }
    else {
        // Iterate through found devices
        for (UINT32 i = 0; i < count; ++i) {
            WCHAR *szFriendlyName = nullptr;
            ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, NULL);
            std::wcout << L"Device " << i + 1 << L": " << szFriendlyName << std::endl;

            // Create a media source from the activation object
            IMFMediaSource *pSource = nullptr;
            CHK(ppDevices[i]->ActivateObject(IID_PPV_ARGS(&pSource)));

            // Create a source reader
            IMFSourceReader *pReader = nullptr;
            CHK(MFCreateSourceReaderFromMediaSource(pSource, pAttributes, &pReader));

            // List all media types (formats)
            IMFMediaType *pMediaType = nullptr;
            DWORD mIndex = 0;
            while (SUCCEEDED(pReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, mIndex, &pMediaType))) {
                std::wcout << L"Format " << mIndex + 1 << L":" << std::endl;
                PrintMediaType(pMediaType);
                pMediaType->Release();
                ++mIndex;
            }

            if (pSource) pSource->Release();
            if (pReader) pReader->Release();
            CoTaskMemFree(szFriendlyName);
        }
    }

    // Cleanup
    if (ppDevices) {
        for (UINT32 i = 0; i < count; i++) {
            if (ppDevices[i]) ppDevices[i]->Release();
        }
        CoTaskMemFree(ppDevices);
    }
    if (pAttributes) pAttributes->Release();

    MFShutdown();
    return 0;
}
