// Copyright (C) 2019-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "monitors/device_monitor.h"

#include <algorithm>
#include <iostream>

namespace ov {
namespace monitor {
DeviceMonitor::DeviceMonitor(const std::shared_ptr<ov::monitor::PerformanceCounter>& performanceCounter, unsigned historySize) :
    samplesNumber{0},
    historySize{historySize},
    performanceCounter{performanceCounter} {
        if (performanceCounter) {
            auto counterSize = performanceCounter->getCoreTimeCounters();
            deviceLoadSum.resize( counterSize > 0 ? counterSize : 1, 0);
        }
    }

DeviceMonitor::~DeviceMonitor() = default;

void DeviceMonitor::setHistorySize(std::size_t size) {
    historySize = size > 0 ? size : 1;
    std::ptrdiff_t newSize = static_cast<std::ptrdiff_t>(std::min(size, deviceLoadHistory.size()));
    deviceLoadHistory.erase(deviceLoadHistory.begin(), deviceLoadHistory.end() - newSize);
}

void DeviceMonitor::collectData() {
    std::vector<double> deviceLoad = performanceCounter->getLoad();
    if (!deviceLoad.empty()) {
        for (std::size_t i = 0; i < deviceLoad.size(); ++i) {
            if (historySize > 1)
                deviceLoadSum[i] += deviceLoad[i];
            else
                deviceLoadSum[i] = deviceLoad[i];
        }
        ++samplesNumber;

        deviceLoadHistory.push_back(std::move(deviceLoad));
        if (deviceLoadHistory.size() > historySize) {
            deviceLoadHistory.pop_front();
        }
    }
}

std::size_t DeviceMonitor::getHistorySize() const {
    return historySize;
}

std::deque<std::vector<double>> DeviceMonitor::getLastHistory() const {
    return deviceLoadHistory;
}

std::vector<double> DeviceMonitor::getMeanDeviceLoad() const {
    std::vector<double> meanDeviceLoad;
    meanDeviceLoad.reserve(deviceLoadSum.size());
    for (double coreLoad : deviceLoadSum) {
        if (historySize > 1)
            meanDeviceLoad.push_back(samplesNumber ? coreLoad / samplesNumber : 0);
        else
            meanDeviceLoad.push_back(samplesNumber ? coreLoad : 0);
    }
    return meanDeviceLoad;
}
}
}
