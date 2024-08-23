#ifndef WEBCAM_H
#define WEBCAM_H

#include "GUID_tools.h"
#include <algorithm>
#include <comdef.h>
#include <fstream>
#include <iostream>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>

void error(HRESULT hr, const std::wstring& message = L"");

class Webcam {
  private:
    IMFActivate*               device_{nullptr};
    IMFMediaSource*            active_device_{nullptr};
    IMFSourceReader*           source_reader_{nullptr};
    IMFAttributes*             config_{nullptr};
    std::vector<IMFMediaType*> media_types_{};

    uint16_t     chosen_media_type_index_{};
    std::wstring name_{};
    bool         active_{false};

  public:
    Webcam() = default;
    Webcam(IMFActivate* device, IMFAttributes* config = nullptr);
    Webcam(const Webcam& other);
    ~Webcam();

    Webcam& operator=(const Webcam& other);

    bool         isActive() const;
    IMFActivate* getDevice() const;
    std::wstring getName() const;

    void setMediaTypeIndex(uint16_t index);

    void listMediaTypes();
    void printMediaType(uint64_t index);
    void printSelectedMediaType();

    int16_t activate();
    int16_t activate(uint16_t index);
    int16_t deactivate();

    void saveFrameAsJPEG(IMFSample* sample, const std::wstring& filePath);
    void saveFrame(const std::wstring& filePath);

    HRESULT getFrame(IMFSample** sample);
};

#endif // WEBCAM_H