// Copyright (C) 2019-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <algorithm>
#include <iostream>
#include <sstream>
#include "monitors/performance_counter.h"
#include "monitors/gpu_performance_counter.h"
#ifdef _WIN32
#define NOMINMAX
#include "query_wrapper.h"
#include <string>
#include <thread>
#include <chrono>
#include <system_error>
#define _PDH_
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <dxgi.h>
#define RENDER_ENGINE_COUNTER_INDEX 0
#define COMPUTE_ENGINE_COUNTER_INDEX 1
#define MAX_COUNTER_INDEX 2

namespace ov {
namespace monitor {
class GpuPerformanceCounter::PerformanceCounterImpl {
public:
    PerformanceCounterImpl() {
        auto devices = getNumberOfCores();
        coreTimeCounters.resize(devices.size());
        for (std::size_t i = 0; i < devices.size(); ++i) {
            coreTimeCounters[i].resize(MAX_COUNTER_INDEX);
        }
        initCoreCounters(devices);
        std::cout << "Device: " << "GPU"<< "\tNumber of cores: " << devices.size() << std::endl;
    }

    std::vector<LUID> getNumberOfCores()
    {
        IDXGIFactory *pFactory;
        HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)(&pFactory));
        if (FAILED(hr))
            return {};
        int gpuIndex = 0;
        std::vector<LUID> gpuCores;
        IDXGIAdapter *pAdapter;
        while (pFactory->EnumAdapters(gpuIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC desc;
            pAdapter->GetDesc(&desc);
            if (wcscmp(desc.Description, L"Microsoft Basic Render Driver") != 0 && desc.VendorId == 0x8086) {
                gpuCores.push_back(desc.AdapterLuid);
                wprintf(L"GPU Name: \n\t%s\nLUID: \n", desc.Description);
                std::cout << "\t" << std::hex << desc.AdapterLuid.HighPart << "-" << desc.AdapterLuid.LowPart << std::endl;
            }
            gpuIndex++;
        }
        return gpuCores;
    }

    void initCoreCounters(const std::vector<LUID>& devices) {
        auto LuidToString = [](LUID luid) -> std::string {
            std::stringstream ss;
            ss << std::hex << ((long long)luid.HighPart << 32 | luid.LowPart);
            return ss.str();
        };
        for (int i = 0; i < devices.size(); i++) {
            std::cout << "device index: " << i << "\tluid: " << LuidToString(devices.at(i)) << std::endl;
            std::string full3DCounterPath = std::string("\\GPU Engine(*_luid_*" + LuidToString(devices.at(i)) + "_phys*engtype_3D)\\Utilization Percentage");
            std::string fullComputeCounterPath = std::string("\\GPU Engine(*_luid_*" + LuidToString(devices.at(i)) + "_phys*engtype_Compute)\\Utilization Percentage");
            coreTimeCounters[i][RENDER_ENGINE_COUNTER_INDEX] = addCounter(query, expandWildCardPath(full3DCounterPath.c_str()));
            coreTimeCounters[i][COMPUTE_ENGINE_COUNTER_INDEX] = addCounter(query, expandWildCardPath(fullComputeCounterPath.c_str()));
        }
        auto status = PdhCollectQueryData(query);
    }

    std::vector<std::string> expandWildCardPath(LPCSTR WildCardPath) {
        PDH_STATUS Status = ERROR_SUCCESS;
        DWORD PathListLength = 0;
        DWORD PathListLengthBufLen;
        std::vector<std::string> pathList;
        Status = PdhExpandWildCardPathA(NULL, WildCardPath, NULL, &PathListLength, 0);
        if (Status != ERROR_SUCCESS && Status != PDH_MORE_DATA) {
            return pathList;
        }
        PathListLengthBufLen = PathListLength + 100;
        PZZSTR ExpandedPathList = (PZZSTR)malloc(PathListLengthBufLen);
        Status = PdhExpandWildCardPathA(NULL, WildCardPath, ExpandedPathList, &PathListLength, 0);
        if (Status != ERROR_SUCCESS) {
            free(ExpandedPathList);
            return pathList;
        }
        for (int i = 0; i < PathListLength;) {
            std::string path(ExpandedPathList + i);
            if (path.length() > 0) {
                //std::cout << path << std::endl;
                pathList.push_back(path);
                i += path.length() + 1;
            } else {
                break;
            }
        }
        free(ExpandedPathList);
        return pathList;
    }

    std::vector<PDH_HCOUNTER> addCounter(PDH_HQUERY Query, std::vector<std::string> pathList) {
        PDH_STATUS Status;
        std::vector<PDH_HCOUNTER> CounterList;
        for (std::string path : pathList) {
            PDH_HCOUNTER Counter;
            Status = PdhAddCounterA(Query, path.c_str(), NULL, &Counter);
            if (Status != ERROR_SUCCESS) {
                return CounterList;
            }
            Status = PdhSetCounterScaleFactor(Counter, -2); // scale counter to [0, 1]
            if (ERROR_SUCCESS != Status) {
                return CounterList;
            }
            CounterList.push_back(Counter);
        }
        return CounterList;
    }

    std::vector<double> getGpuLoad() {
        PDH_STATUS status;
        auto ts = std::chrono::system_clock::now();
        if (ts > lastTimeStamp) {
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastTimeStamp);
            if (delta.count() < 500) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500 - delta.count()));
            }
        }
        lastTimeStamp = std::chrono::system_clock::now();
        status = PdhCollectQueryData(query);
        PDH_FMT_COUNTERVALUE displayValue;
        std::vector<double> gpuLoad;
        gpuLoad.resize(coreTimeCounters.size(), 0.0);
        for (std::size_t coreIndex = 0; coreIndex < coreTimeCounters.size(); ++coreIndex) {
            double value = 0;
            auto coreCounters = coreTimeCounters[coreIndex];
            for (int counterIndex = 0; counterIndex < MAX_COUNTER_INDEX; counterIndex++) {
                auto countersList = coreCounters[counterIndex];
                for (auto counter : countersList) {
                    status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL,
                        &displayValue);
                    if (status != ERROR_SUCCESS) {
                        continue;
                    }
                    value += displayValue.doubleValue; 
                }
            }
            gpuLoad[coreIndex] = value;
        }
        return gpuLoad;
    }

private:
    QueryWrapper query;
    std::vector<std::vector<std::vector<PDH_HCOUNTER>>> coreTimeCounters;
    std::chrono::time_point<std::chrono::system_clock> lastTimeStamp = std::chrono::system_clock::now();
};

#elif __linux__
#include <chrono>
#include <regex>
#include <utility>
#include <fstream>
#include <unistd.h>

namespace {
const long clockTicks = sysconf(_SC_CLK_TCK);

const std::size_t nCores = sysconf(_SC_NPROCESSORS_CONF);

std::vector<unsigned long> getIdleCpuStat() {
    std::vector<unsigned long> idleCpuStat(nCores);
    std::ifstream procStat("/proc/stat");
    std::string line;
    std::smatch match;
    std::regex coreJiffies("^cpu(\\d+)\\s+"
        "(\\d+)\\s+"
        "(\\d+)\\s+"
        "(\\d+)\\s+"
        "(\\d+)\\s+" // idle
        "(\\d+)"); // iowait

    while (std::getline(procStat, line)) {
        if (std::regex_search(line, match, coreJiffies)) {
            // it doesn't handle overflow of sum and overflows of /proc/stat values
            unsigned long idleInfo = stoul(match[5]) + stoul(match[6]),
                coreId = stoul(match[1]);
            if (nCores <= coreId) {
                throw std::runtime_error("The number of cores has changed");
            }
            idleCpuStat[coreId] = idleInfo;
        }
    }
    return idleCpuStat;
}
}

class CpuMonitor::PerformanceCounterImpl {
public:
    PerformanceCounterImpl() : prevIdleCpuStat{getIdleCpuStat()}, prevTimePoint{std::chrono::steady_clock::now()} {}

    std::vector<double> getCpuLoad() {
        std::vector<unsigned long> idleCpuStat = getIdleCpuStat();
        auto timePoint = std::chrono::steady_clock::now();
        // don't update data too frequently which may result in negative values for cpuLoad.
        // It may happen when collectData() is called just after setHistorySize().
        if (timePoint - prevTimePoint > std::chrono::milliseconds{100}) {
            std::vector<double> cpuLoad(nCores);
            for (std::size_t i = 0; i < idleCpuStat.size(); ++i) {
                double idleDiff = idleCpuStat[i] - prevIdleCpuStat[i];
                typedef std::chrono::duration<double, std::chrono::seconds::period> Sec;
                cpuLoad[i] = 1.0
                    - idleDiff / clockTicks / std::chrono::duration_cast<Sec>(timePoint - prevTimePoint).count();
            }
            prevIdleCpuStat = std::move(idleCpuStat);
            prevTimePoint = timePoint;
            return cpuLoad;
        }
        return {};
    }
private:
    std::vector<unsigned long> prevIdleCpuStat;
    std::chrono::steady_clock::time_point prevTimePoint;
};

#else
// not implemented
namespace {
const std::size_t nCores{0};
}

class CpuMonitor::PerformanceCounterImpl {
public:
    std::vector<double> getCpuLoad() {return {};};
};
#endif
GpuPerformanceCounter::GpuPerformanceCounter() : ov::monitor::PerformanceCounter("GPU") {}
GpuPerformanceCounter::~GpuPerformanceCounter() {
    delete performanceCounter; 
}
std::vector<double> GpuPerformanceCounter::getLoad() {
    if (!performanceCounter)
        performanceCounter = new PerformanceCounterImpl();
    return performanceCounter->getGpuLoad();
}
}
}