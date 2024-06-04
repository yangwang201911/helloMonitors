// Copyright (C) 2019-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <deque>
#include <memory>
#include <vector>
#include "performance_counter.h"

namespace ov {
namespace monitor {
class GpuPerformanceCounter : public ov::monitor::PerformanceCounter {
public:
    GpuPerformanceCounter(int nCores = 0);
    ~GpuPerformanceCounter();
    std::vector<double> getLoad() override;
private:
    int nCores = 0;
    class PerformanceCounterImpl;
    PerformanceCounterImpl* performanceCounter = NULL;
};
}
}