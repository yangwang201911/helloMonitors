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
        return -1;
    int gpuIndex = 0;
    int realGpuNmb = 0;
    IDXGIAdapter *pAdapter;
    while (pFactory->EnumAdapters(gpuIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        pAdapter->GetDesc(&desc);
        wprintf(L"  Device Name: %s\tVendor ID: 0x%x\tDevice ID: 0x%x\n", desc.Description, desc.VendorId, desc.AdapterLuid);
        if (wcscmp(desc.Description, L"Microsoft Basic Render Driver") != 0 || desc.VendorId == 0x8086) {
            realGpuNmb++;
        }
        gpuIndex++;
    }
    return realGpuNmb;
}

int getCPULoadPDH()
{
    PDH_STATUS status;
    PDH_HQUERY query;
    status = PdhOpenQuery(NULL, 0, &query);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "PdhOpenQuery failed with error code: " << status << std::endl;
        return 1;
    }
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
        status = PdhCollectQueryData(query);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhCollectQueryData failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }
        Sleep(1000);
        status = PdhCollectQueryData(query);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhCollectQueryData failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }

        PDH_FMT_COUNTERVALUE value;
        status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhGetFormattedCounterValue failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }

        std::cout << "CPU usage: " << value.doubleValue << "%" << std::endl;
    }
    PdhCloseQuery(query);

    return 0;
}

int getGPULoadPDH(int index = 0)
{
    PDH_STATUS status;
    PDH_HQUERY query;
    status = PdhOpenQuery(NULL, 0, &query);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "PdhOpenQuery failed with error code: " << status << std::endl;
        return 1;
    }
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
        status = PdhCollectQueryData(query);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhCollectQueryData failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }
        Sleep(1000);
        status = PdhCollectQueryData(query);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhCollectQueryData failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }

        PDH_FMT_COUNTERVALUE value;
        status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);
        if (status != ERROR_SUCCESS)
        {
            std::cerr << "PdhGetFormattedCounterValue failed with error code: " << status << std::endl;
            PdhCloseQuery(query);
            return 1;
        }

        std::cout << "GPU usage: " << value.doubleValue << "%" << std::endl;
    }
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
        for (LPWSTR szThisCounter = szCounterListBuffer;
             *szThisCounter != '\0';
             szThisCounter += wcslen(szThisCounter) + 1)
        {
            printf("%ws\\%ws\n", szObjectName, szThisCounter);
        }
    }
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
    Status = PdhEnumObjectsW(
        NULL,               // real time source
        NULL,               // local machine
        szObjectListBuffer, // buffer to receive object names
        &dwBufferSize,      // size of object name buffer
        PERF_DETAIL_WIZARD, // detail level
        FALSE);             // refresh flag
    if (Status == PDH_MORE_DATA)
    {
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
        for (LPWSTR szThisObject = szObjectListBuffer;
             *szThisObject != '\0';
             szThisObject += wcslen(szThisObject) + 1)
        {
            PrintCounterPaths(szThisObject);
        }
    }
    if (szObjectListBuffer != NULL)
    {
        free(szObjectListBuffer);
    }
    return 0;
}

int main()
{
    // getPdhObjectsPath();
    int nGPUs = getGPUNumber();
    printf("number of GPUs: %d\n", nGPUs);
    getGPULoadPDH(nGPUs - 1);
    return 0;
}