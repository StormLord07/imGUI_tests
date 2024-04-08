#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <iostream>

int main() {
    // Initialization code here
    // For DirectX 12, you would typically create a device, a swap chain, etc.
    std::cout << "DirectX 12 application starting." << std::endl;

    // Example: Create a DirectX 12 device
    ID3D12Device* device = nullptr;
    if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
        std::cout << "DirectX 12 device created successfully." << std::endl;
    } else {
        std::cout << "Failed to create DirectX 12 device." << std::endl;
    }

    // Clean up
    if (device) {
        device->Release();
    }

    return 0;
}
