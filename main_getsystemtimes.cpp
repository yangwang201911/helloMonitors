#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <thread>
#include <chrono>

FILETIME prevIdleTime, prevKernelTime, prevUserTime;

typedef BOOL ( __stdcall * SYSTEM_TIMES_CALLBACK)( LPFILETIME lpIdleTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime );

BOOL GetSystemTimesAsFileTime(PFILETIME lpIdleTime, PFILETIME lpKernelTime, PFILETIME lpUserTime);
ULONGLONG SubtractTimes(const FILETIME ftA, const FILETIME ftB);
double CalculateCPULoad();

double CalculateCPULoad() {
    FILETIME idleTime, kernelTime, userTime;
    ULONGLONG idleTicks, kernelTicks, userTicks;
    double idleDiff, kernelDiff, userDiff;

    if (!GetSystemTimesAsFileTime(&idleTime, &kernelTime, &userTime)) {
        printf("\nFailed to get system times. Error code: %lu\n", GetLastError());
        return 0.0;
    }

    idleTicks = SubtractTimes(idleTime, prevIdleTime);
    kernelTicks = SubtractTimes(kernelTime, prevKernelTime);
    userTicks = SubtractTimes(userTime, prevUserTime);

    idleDiff = (double)idleTicks;
    kernelDiff = (double)kernelTicks;
    userDiff = (double)userTicks;

    double totalTicks = kernelDiff + userDiff;
    double cpuUsage = 100 - ((idleDiff / totalTicks) * 100.0);

    prevIdleTime = idleTime;
    prevKernelTime = kernelTime;
    prevUserTime = userTime;

    return cpuUsage;
}

BOOL GetSystemTimesAsFileTime(PFILETIME lpIdleTime, PFILETIME lpKernelTime, PFILETIME lpUserTime) {
    SYSTEM_TIMES_CALLBACK pfnGetSystemTimes;
    HMODULE hKernel32 = GetModuleHandle("kernel32.dll");

    if (hKernel32 == NULL) {
        return FALSE;
    }

    pfnGetSystemTimes = (SYSTEM_TIMES_CALLBACK)GetProcAddress(hKernel32, "GetSystemTimes");
    if (pfnGetSystemTimes == NULL) {
        return FALSE;
    }

    if (!pfnGetSystemTimes(lpIdleTime, lpKernelTime, lpUserTime)) {
        return FALSE;
    }

    return TRUE;
}

ULONGLONG SubtractTimes(const FILETIME ftA, const FILETIME ftB) {
    ULARGE_INTEGER a, b;

    a.LowPart = ftA.dwLowDateTime;
    a.HighPart = ftA.dwHighDateTime;

    b.LowPart = ftB.dwLowDateTime;
    b.HighPart = ftB.dwHighDateTime;

    return a.QuadPart - b.QuadPart;
}

int main() {
    double cpuUsage;

    GetSystemTimesAsFileTime(&prevIdleTime, &prevKernelTime, &prevUserTime);

    int i = 0;
    int time = 20;
    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        cpuUsage = CalculateCPULoad();
        if (i++ % 50 == 0)
            printf("Time: %d - CPU utilization: %.2f%%\n", time, cpuUsage);
    }

    return 0;
}