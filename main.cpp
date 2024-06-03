#include <iostream>
#include <thread>
#include <chrono>
#include "monitors/device_monitor.h"
#include "monitors/cpu_performance_counter.h"
int main(int argc, char *argv[])
{
    ov::monitor::DeviceMonitor cpuMonitor{std::make_shared<ov::monitor::CpuPerformanceCounter>(), 1};
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cpuMonitor.collectData();
        for (auto load : cpuMonitor.getMeanDeviceLoad()) {
            std::cout << load * 100 << "% ";
        }
        std::cout << std::endl;
    }
    return 0;
}