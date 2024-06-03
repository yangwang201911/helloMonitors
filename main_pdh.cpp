#include <iostream>
#include <Windows.h>
#include <Pdh.h>
#include <pdhmsg.h>
#include <stdio.h>
#include <string>
#include <d3d11.h>
#include <dxgi.h>
int getGPUNumber()
{
    IDXGIFactory *pFactory;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)(&pFactory));
    if (FAILED(hr))
    {
        // Handle error
    }
    int gpuIndex = 0;
    int realGpuNmb = 0;
    IDXGIAdapter *pAdapter;
    while (pFactory->EnumAdapters(gpuIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        pAdapter->GetDesc(&desc);
        if (wcscmp(desc.Description, L"Microsoft Basic Render Driver") != 0)
        {
            wprintf(L"Device Number: %d\n", gpuIndex);
            wprintf(L"  Device Name: %s\n", desc.Description);
            realGpuNmb++;
        }
        gpuIndex++;
    }
    return realGpuNmb;
}

int getCPULoadPDH()
{
    // 初始化 PDH 查询
    PDH_STATUS status;
    PDH_HQUERY query;
    status = PdhOpenQuery(NULL, 0, &query);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "PdhOpenQuery failed with error code: " << status << std::endl;
        return 1;
    }
    // 添加 CPU 利用率性能计数器
    PDH_HCOUNTER counter;
    std::string fullCounterPath{"\\Processor(_Total)\\% Processor Time"};
    status = PdhAddCounter(query, fullCounterPath.c_str(), 0, &counter);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "PdhAddCounter failed with error code: " << status << std::endl;
        PdhCloseQuery(query);
        return 1;
    }

    while (1)
    {
        // 收集查询数据
        status = PdhCollectQueryData(query);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhCollectQueryData failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }
        // 等待一段时间，再次收集数据
        Sleep(1000); // 等待 1 秒
        status = PdhCollectQueryData(query);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhCollectQueryData failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }

        // 获取性能计数器值
        PDH_FMT_COUNTERVALUE value;
        status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhGetFormattedCounterValue failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }

        // 输出 CPU 利用率
        std::cout << "CPU usage: " << value.doubleValue << "%" << std::endl;
    }
    // 关闭查询
    PdhCloseQuery(query);

    return 0;
}

int getGPULoadPDH(int index = 0)
{
    // 初始化 PDH 查询
    PDH_STATUS status;
    PDH_HQUERY query;
    status = PdhOpenQuery(NULL, 0, &query);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "PdhOpenQuery failed with error code: " << status << std::endl;
        return 1;
    }
    // 添加 GPU 利用率性能计数器
    PDH_HCOUNTER counter;
    std::string fullCounterPath{"\\GPU Engine(*phys_" + std::to_string(index) + "_*3d)\\Utilization Percentage"};
    status = PdhAddCounter(query, fullCounterPath.c_str(), 0, &counter);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "PdhAddCounter failed with error code: " << status << std::endl;
        PdhCloseQuery(query);
        return 1;
    }

    while (1)
    {
        // 收集查询数据
        status = PdhCollectQueryData(query);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhCollectQueryData failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }
        // 等待一段时间，再次收集数据
        Sleep(1000); // 等待 1 秒
        status = PdhCollectQueryData(query);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhCollectQueryData failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }

        // 获取性能计数器值
        PDH_FMT_COUNTERVALUE value;
        status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhGetFormattedCounterValue failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }

        // 输出 GPU 利用率
        std::cout << "GPU usage: " << value.doubleValue << "%" << std::endl;
    }
    // 关闭查询
    PdhCloseQuery(query);

    return 0;
}

void PrintCounterPaths(LPCWSTR szObjectName)
{
    PDH_STATUS Status;
    DWORD dwBufferSize = 0;
    DWORD dwBufferCount = 0;
    LPWSTR szCounterListBuffer = NULL;
    LPWSTR szInstanceListBuffer = NULL;
    // 获取对象的计数器和实例名
    Status = PdhEnumObjectItemsW(
        NULL,                 // real time source
        NULL,                 // local machine
        szObjectName,         // object to enumerate
        szCounterListBuffer,  // buffer to receive counter names
        &dwBufferSize,        // size of counter name buffer
        szInstanceListBuffer, // buffer to receive instance names
        &dwBufferCount,       // size of instance name buffer
        PERF_DETAIL_WIZARD,   // detail level
        0);                   // default format
    if (Status == PDH_MORE_DATA)
    {
        // 分配缓冲区
        szCounterListBuffer = (LPWSTR)malloc(dwBufferSize * sizeof(WCHAR));
        szInstanceListBuffer = (LPWSTR)malloc(dwBufferCount * sizeof(WCHAR));
        if (szCounterListBuffer != NULL && szInstanceListBuffer != NULL)
        {
            Status = PdhEnumObjectItemsW(
                NULL, NULL, szObjectName,
                szCounterListBuffer, &dwBufferSize,
                szInstanceListBuffer, &dwBufferCount,
                PERF_DETAIL_WIZARD, 0);
        }
    }
    if (Status == ERROR_SUCCESS)
    {
        // 打印所有的计数器路径
        for (LPWSTR szThisCounter = szCounterListBuffer;
             *szThisCounter != '\0';
             szThisCounter += wcslen(szThisCounter) + 1)
        {
            printf("%ws\\%ws\n", szObjectName, szThisCounter);
        }
    }
    // 释放缓冲区
    if (szCounterListBuffer != NULL)
    {
        free(szCounterListBuffer);
    }
    if (szInstanceListBuffer != NULL)
    {
        free(szInstanceListBuffer);
    }
}
int getPdhObjectsPath()
{
    PDH_STATUS Status;
    DWORD dwBufferSize = 0;
    LPWSTR szObjectListBuffer = NULL;
    // 枚举本地计算机上的所有性能对象
    Status = PdhEnumObjectsW(
        NULL,               // real time source
        NULL,               // local machine
        szObjectListBuffer, // buffer to receive object names
        &dwBufferSize,      // size of object name buffer
        PERF_DETAIL_WIZARD, // detail level
        FALSE);             // refresh flag
    if (Status == PDH_MORE_DATA)
    {
        // 分配缓冲区
        szObjectListBuffer = (LPWSTR)malloc(dwBufferSize * sizeof(WCHAR));
        if (szObjectListBuffer != NULL)
        {
            Status = PdhEnumObjectsW(
                NULL, NULL, szObjectListBuffer,
                &dwBufferSize, PERF_DETAIL_WIZARD, FALSE);
        }
    }
    if (Status == ERROR_SUCCESS)
    {
        // 对于每个对象，打印所有的计数器路径
        for (LPWSTR szThisObject = szObjectListBuffer;
             *szThisObject != '\0';
             szThisObject += wcslen(szThisObject) + 1)
        {
            PrintCounterPaths(szThisObject);
        }
    }
    // 释放缓冲区
    if (szObjectListBuffer != NULL)
    {
        free(szObjectListBuffer);
    }
    return 0;
}

int main()
{
    getPdhObjectsPath();
    int nGPUs = getGPUNumber();
    printf("number of GPUs: %d\n", nGPUs);
    getGPULoadPDH(nGPUs - 1);
    return 0;
}