// Copyright (C) 2019-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include <iostream>
#include <vector>
#include <string>

namespace ov {
namespace monitor {
class PerformanceCounter {
public:
    PerformanceCounter(std::string deviceName) {
    }
    virtual std::vector<double> getLoad() = 0;
    std::string name() {
        return deviceName;
    }

private:
    std::string deviceName;
};
}
}