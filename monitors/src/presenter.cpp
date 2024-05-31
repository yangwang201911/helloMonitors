// Copyright (C) 2019-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <iostream>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>

#include "monitors/presenter.h"

namespace {
const std::map<int, MonitorType> keyToMonitorType{
    {'C', MonitorType::CpuAverage},
    {'D', MonitorType::DistributionCpu},
    {'M', MonitorType::Memory}};

std::set<MonitorType> strKeysToMonitorSet(const std::string& keys) {
    std::set<MonitorType> enabledMonitors;
    if (keys == "h") {
        return enabledMonitors;
    }
    for (unsigned char key: keys) {
        if (key == 'h') {
            throw std::runtime_error("Unacceptable combination of monitor types-can't show and hide info at the same time");
        }
        auto iter = keyToMonitorType.find(std::toupper(key));
        if (keyToMonitorType.end() == iter) {
            throw std::runtime_error("Unknown monitor type");
        } else {
            enabledMonitors.insert(iter->second);
        }
    }
    return enabledMonitors;
}
}

Presenter::Presenter(std::set<MonitorType> enabledMonitors,
        int yPos,
        std::size_t historySize) :
            yPos{yPos},
            historySize{historySize},
            distributionCpuEnabled{false},
            strStream{std::ios_base::app} {
    for (MonitorType monitor : enabledMonitors) {
        addRemoveMonitor(monitor);
    }
}

Presenter::Presenter(const std::string& keys, int yPos, std::size_t historySize) :
    Presenter{strKeysToMonitorSet(keys), yPos, historySize} {}

void Presenter::addRemoveMonitor(MonitorType monitor) {
    unsigned updatedHistorySize = 1;
    switch(monitor) {
        case MonitorType::CpuAverage: {
            if (historySize <= 0) {
                cpuMonitor.setHistorySize(0);
            } else {
                cpuMonitor.setHistorySize(historySize);
            }
            std::cout << "CPU history size: " << cpuMonitor.getHistorySize() << std::endl;
            break;
        }
        case MonitorType::DistributionCpu: {
            if (distributionCpuEnabled) {
                distributionCpuEnabled = false;
                if (1 == cpuMonitor.getHistorySize()) { // cpuMonitor was used only for DistributionCpu => disable it
                    cpuMonitor.setHistorySize(0);
                }
            } else {
                distributionCpuEnabled = true;
                cpuMonitor.setHistorySize(std::max(std::size_t{1}, cpuMonitor.getHistorySize()));
            }
            break;
        }
        case MonitorType::Memory: {
            if (memoryMonitor.getHistorySize() > 1) {
                memoryMonitor.setHistorySize(0);
            } else {
                memoryMonitor.setHistorySize(updatedHistorySize);
            }
            break;
        }
    }
}

void Presenter::handleKey(int key) {
    key = std::toupper(key);
    if ('H' == key) {
        if (0 == cpuMonitor.getHistorySize() && memoryMonitor.getHistorySize() <= 1) {
            addRemoveMonitor(MonitorType::CpuAverage);
            addRemoveMonitor(MonitorType::DistributionCpu);
            addRemoveMonitor(MonitorType::Memory);
        } else {
            cpuMonitor.setHistorySize(0);
            distributionCpuEnabled = false;
            memoryMonitor.setHistorySize(0);
        }
    } else {
        auto iter = keyToMonitorType.find(key);
        if (keyToMonitorType.end() != iter) {
            addRemoveMonitor(iter->second);
        }
    }
}

void Presenter::collect() {
    const std::chrono::steady_clock::time_point curTimeStamp = std::chrono::steady_clock::now();
    if (curTimeStamp - prevTimeStamp >= std::chrono::milliseconds{1000}) {
        prevTimeStamp = curTimeStamp;
        if (0 != cpuMonitor.getHistorySize()) {
            cpuMonitor.collectData();
        }
        if (memoryMonitor.getHistorySize() > 1) {
            memoryMonitor.collectData();
        }
    }

    int numberOfEnabledMonitors = (cpuMonitor.getHistorySize() > 1) + distributionCpuEnabled
        + (memoryMonitor.getHistorySize() > 1);

    if (cpuMonitor.getHistorySize() > 1 && --numberOfEnabledMonitors >= 0) {
        std::deque<std::vector<double>> lastHistory = cpuMonitor.getLastHistory();
        for (int i = lastHistory.size() - 1; i >= 0; --i) {
            double mean = std::accumulate(lastHistory[i].begin(), lastHistory[i].end(), 0.0) / lastHistory[i].size();
        }

        strStream.str("CPU");
        if (!lastHistory.empty()) {
            strStream << ": " << std::fixed << std::setprecision(1)
                << std::accumulate(lastHistory.back().begin(), lastHistory.back().end(), 0.0)
                    / lastHistory.back().size() * 100 << '%';
        }
    }

    if (distributionCpuEnabled && --numberOfEnabledMonitors >= 0) {
        std::deque<std::vector<double>> lastHistory = cpuMonitor.getLastHistory();
        strStream.str("Core load");
        if (!lastHistory.empty()) {
            strStream << ": " << std::fixed << std::setprecision(1)
                << std::accumulate(lastHistory.back().begin(), lastHistory.back().end(), 0.0)
                    / lastHistory.back().size() * 100 << '%';
        }
    }
}

std::vector<std::string> Presenter::reportMeans() const {
    std::vector<std::string> collectedData;
    if (cpuMonitor.getHistorySize() > 1 || distributionCpuEnabled || memoryMonitor.getHistorySize() > 1) {
        collectedData.push_back("Resources usage:");
    }
    if (cpuMonitor.getHistorySize() > 1) {
        std::ostringstream collectedDataStream;
        collectedDataStream << std::fixed << std::setprecision(1);
        collectedDataStream << "\tMean core utilization: ";
        double meanSum = 0.0;
        for (double mean : cpuMonitor.getMeanCpuLoad()) {
            collectedDataStream << mean * 100 << "% ";
            meanSum += mean * 100;
        }
        collectedData.push_back(collectedDataStream.str());
    }
    if (distributionCpuEnabled) {
        std::ostringstream collectedDataStream;
        collectedDataStream << std::fixed << std::setprecision(1);
        std::vector<double> meanCpuLoad = cpuMonitor.getMeanCpuLoad();
        double mean = std::accumulate(meanCpuLoad.begin(), meanCpuLoad.end(), 0.0) / meanCpuLoad.size();
        collectedDataStream << "\tMean CPU utilization: " << mean * 100 << "%";
        collectedData.push_back(collectedDataStream.str());
    }
    if (memoryMonitor.getHistorySize() > 1) {
        std::ostringstream collectedDataStream;
        collectedDataStream << std::fixed << std::setprecision(1);
        collectedDataStream << "\tMemory mean usage: " << memoryMonitor.getMeanMem() << " GiB";
        collectedData.push_back(collectedDataStream.str());
        collectedDataStream.str("");
        collectedDataStream << "\tMean swap usage: " << memoryMonitor.getMeanSwap() << " GiB";
        collectedData.push_back(collectedDataStream.str());
    }

    return collectedData;
}
