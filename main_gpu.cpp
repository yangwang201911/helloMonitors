#include <iostream>
#include <Windows.h>
#include <stdint.h>

// 定义性能计数器事件ID
#define INTEL_GPU_COUNTER_EVENT_ID 0x0F

// 定义性能计数器接口
typedef struct _INTEL_GPU_COUNTER_INTERFACE
{
    uint32_t version;               // 版本号
    uint32_t deviceId;              // 设备ID
    uint32_t revisionId;            // 修订ID
    uint32_t numCounters;           // 支持的计数器数量
    uint32_t counterIds[1];         // 计数器ID数组
} INTEL_GPU_COUNTER_INTERFACE;

// 声明性能计数器接口
extern "C" __declspec(dllexport) bool GetIntelGPUCounterInterface(INTEL_GPU_COUNTER_INTERFACE** ppCounterInterface);

int main()
{
    // 加载性能计数器接口
    HMODULE hModule = LoadLibraryW(L"IntelGPUPerfCounter.dll");
    if (hModule == NULL)
    {
        std::cerr << "Failed to load Intel GPU Perf Counter DLL." << std::endl;
        return 1;
    }

    // 获取接口函数指针
    typedef bool(*GetCounterInterfaceFunc)(INTEL_GPU_COUNTER_INTERFACE**);
    GetCounterInterfaceFunc pGetCounterInterface = reinterpret_cast<GetCounterInterfaceFunc>(GetProcAddress(hModule, "GetIntelGPUCounterInterface"));
    if (pGetCounterInterface == nullptr)
    {
        std::cerr << "Failed to get function pointer." << std::endl;
        FreeLibrary(hModule);
        return 1;
    }

    // 获取性能计数器接口
    INTEL_GPU_COUNTER_INTERFACE* pCounterInterface = nullptr;
    if (!pGetCounterInterface(&pCounterInterface))
    {
        std::cerr << "Failed to get Intel GPU Counter Interface." << std::endl;
        FreeLibrary(hModule);
        return 1;
    }

    // 打印支持的计数器数量
    std::cout << "Number of counters: " << pCounterInterface->numCounters << std::endl;

    // 查找GPU利用率计数器
    bool found = false;
    for (uint32_t i = 0; i < pCounterInterface->numCounters; ++i)
    {
        if (pCounterInterface->counterIds[i] == INTEL_GPU_COUNTER_EVENT_ID)
        {
            found = true;
            break;
        }
    }

    // 释放性能计数器接口内存
    free(pCounterInterface);

    if (!found)
    {
        std::cerr << "Intel GPU utilization counter not found." << std::endl;
        FreeLibrary(hModule);
        return 1;
    }

    std::cout << "Intel GPU utilization counter found." << std::endl;

    // 使用性能计数器获取GPU利用率
    // 这里需要编写代码调用性能计数器API来获取GPU的实际利用率

    // 释放库
    FreeLibrary(hModule);

    return 0;
}