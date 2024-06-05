#include <iostream>
#include <thread>
#include <chrono>
#include "monitors/device_monitor.h"
#include "monitors/cpu_performance_counter.h"
#include "monitors/gpu_performance_counter.h"
int main(int argc, char *argv[])
{
    ov::monitor::DeviceMonitor cpuMonitor{std::make_shared<ov::monitor::CpuPerformanceCounter>(), 1};
    ov::monitor::DeviceMonitor gpuMonitor{std::make_shared<ov::monitor::GpuPerformanceCounter>(), 1};
    while (1)
    {
        std::cout << "CPU: ";
        for (auto load : cpuMonitor.getMeanDeviceLoad()) {
            std::cout << load * 100 << "% ";
        }
        std::cout << std::endl;
        cpuMonitor.collectData();

        std::cout << "GPU: ";
        for (auto load : gpuMonitor.getMeanDeviceLoad()) {
            std::cout << load * 100 << "% ";
        }
        std::cout << std::endl;
        gpuMonitor.collectData();
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}