#ifndef WEBCAM_H

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <iostream>
#include <vector>
#include <string>


class webcam {
private:
    std::vector<IMFActivate *> devices_{};
    IMFAttributes *config_{ nullptr };
public:
    webcam();
    ~webcam();
    void rescan_devices();

    std::vector<IMFActivate *> get_devices() const;
    std::vector<std::wstring> get_device_names() const;

};

#endif // WEBCAM_H