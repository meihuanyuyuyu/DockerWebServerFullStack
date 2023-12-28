#ifndef __GPU_INFO_HPP__
#define __GPU_INFO_HPP__

#include<string>
#include<nvml.h>

namespace gpuClient{
    struct MemoryInfo{
        unsigned long long total;
        unsigned long long used;
        unsigned long long free;
    };

    struct UtilizationInfo{
        unsigned int gpu_utilization;
        unsigned int gpu_memory_utilization;
    };

    struct ProcessInfo{
        unsigned int pid;
        unsigned int used_memory;
    };

    struct gpuInfo
    {
        std::string gpu_device_name;
        unsigned int device_id;
        UtilizationInfo utilization_info;
        MemoryInfo memory_info;
        /* data */
    };
}