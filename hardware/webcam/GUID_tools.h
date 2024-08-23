#ifndef GUID_TOOLS_H
#define GUID_TOOLS_H

#include <iostream>
#include <map>
#include <mfapi.h>

struct GuidComparer {
    bool operator()(const GUID& lhs, const GUID& rhs) const;
};

// Additional function to convert GUID to string
std::wstring guidToString(const GUID& guid);

// Mapping function to convert Media Foundation GUIDs to readable strings
std::wstring getGuidName(const GUID& guid);

#endif // GUID_TOOLS_H