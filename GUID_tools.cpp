#include "GUID_tools.h"

bool GuidComparer::operator()(const GUID& lhs, const GUID& rhs) const {
    return memcmp(&lhs, &rhs, sizeof(GUID)) < 0;
}

// Additional function to convert GUID to string
std::wstring guidToString(const GUID& guid) {
    wchar_t buf[39]; // Length for GUID string with braces and null-terminator
    StringFromGUID2(guid, buf, 39);
    return std::wstring(buf);
}

// Mapping function to convert Media Foundation GUIDs to readable strings
std::wstring getGuidName(const GUID& guid) {
    static const std::map<GUID, std::wstring, GuidComparer> guidMap = {
        {MFMediaType_Audio, L"Audio"},
        {MFMediaType_Video, L"Video"},
        {MFMediaType_HTML, L"HTML"},
        {MFMediaType_Binary, L"Binary"},
        {MFMediaType_FileTransfer, L"File Transfer"},
        {MFMediaType_Image, L"Image"},
        {MFMediaType_Stream, L"Stream"},
        {MFVideoFormat_AI44, L"AI44"},
        {MFVideoFormat_ARGB32, L"ARGB32"},
        {MFVideoFormat_AYUV, L"AYUV"},
        {MFVideoFormat_DV25, L"DV25"},
        {MFVideoFormat_DV50, L"DV50"},
        {MFVideoFormat_DVH1, L"DVH1"},
        {MFVideoFormat_DVSD, L"DVSD"},
        {MFVideoFormat_DVSL, L"DVSL"},
        {MFVideoFormat_H264, L"H264"},
        {MFVideoFormat_I420, L"I420"},
        {MFVideoFormat_IYUV, L"IYUV"},
        {MFVideoFormat_NV11, L"NV11"},
        {MFVideoFormat_NV12, L"NV12"},
        {MFVideoFormat_RGB24, L"RGB24"},
        {MFVideoFormat_RGB32, L"RGB32"},
        {MFVideoFormat_RGB555, L"RGB555"},
        {MFVideoFormat_RGB565, L"RGB565"},
        {MFVideoFormat_RGB8, L"RGB8"},
        {MFVideoFormat_UYVY, L"UYVY"},
        {MFVideoFormat_v210, L"v210"},
        {MFVideoFormat_v410, L"v410"},
        {MFVideoFormat_WMV1, L"WMV1"},
        {MFVideoFormat_WMV2, L"WMV2"},
        {MFVideoFormat_WMV3, L"WMV3"},
        {MFVideoFormat_WVC1, L"WVC1"},
        {MFVideoFormat_YUY2, L"YUY2"},
        {MFVideoFormat_YV12, L"YV12"},
        {MFAudioFormat_PCM, L"PCM"},
        {MFAudioFormat_Float, L"Float"},
        {MFAudioFormat_DTS, L"DTS"},
        {MFAudioFormat_Dolby_AC3_SPDIF, L"Dolby AC3 SPDIF"},
        {MFAudioFormat_DRM, L"DRM"},
        {MFAudioFormat_WMAudioV8, L"WMAudioV8"},
        {MFAudioFormat_WMAudioV9, L"WMAudioV9"},
        {MFAudioFormat_WMAudio_Lossless, L"WMAudio Lossless"},
        {MFAudioFormat_WMASPDIF, L"WMASPDIF"},
        {MFAudioFormat_MP3, L"MP3"},
        {MFAudioFormat_MPEG, L"MPEG"},
        {MFVideoFormat_MJPG, L"MJPG"}};

    auto it = guidMap.find(guid);
    if (it != guidMap.end()) {
        return L"Type: " + it->second + L" GUID: " + guidToString(guid);
    } else {
        return L"Unknown Type (" + guidToString(guid) + L")";
    }
}