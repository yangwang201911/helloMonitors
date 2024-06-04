// Copyright (C) 2019-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include <vector>
#include <string>

namespace ov {
namespace monitor {
class PerformanceCounter {
public:
    PerformanceCounter(std::string deviceName, int nCores = 0) : coreTimeCounters(nCores >= 0 ? nCores : 0) {}
    virtual std::vector<double> getLoad() = 0;
    std::string name() {
        return deviceName;
    }
    int getCoreTimeCounters() {
        return coreTimeCounters;
    }
    void setCoreTimeCounters(int nCores) {
        coreTimeCounters = nCores;
    }

private:
    int coreTimeCounters;
    std::string deviceName;
};
}
}