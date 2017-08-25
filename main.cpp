// ascendingproblem.cpp : Defines the entry point64_t for the console application.
//

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cassert>

#include <vector>
#include <stack>
#include <thread>
#include <atomic>

#include <string>

#include "safemath.h"
#include "tools.h"
#include "expression.h"

#include "expresion_traverser.h"



///////////////////////////////////////////////////

#include <CL/cl.h>
#include <fstream>


class cl_driver
{
    /*
    const std::vector<std::pair<cl_platform_info, const char*>> m_platformAttributes = {
        { CL_PLATFORM_NAME, "Name" },
        { CL_PLATFORM_VENDOR, "Vendor" },
        { CL_PLATFORM_VERSION, "Version" },
        { CL_PLATFORM_PROFILE, "Profile" },
        { CL_PLATFORM_EXTENSIONS, "Extensions" }
    };


    const std::vector<std::pair<cl_device_info, const char*>> m_deviceAttributes = {
        { CL_DEVICE_NAME, "Name" },
        { CL_DEVICE_VERSION, "Hardware version" },
        { CL_DRIVER_VERSION, "Software version" },
        { CL_DEVICE_OPENCL_C_VERSION, "OpenCL C version" }
    };
    */


    struct SPlatform
    {
        cl_platform_id id;
        std::vector<cl_device_id> vDevices;
    };


    std::string GetPlatformInfo(cl_platform_id platform, cl_platform_info paramaterName)
    {
        std::array<char, 1024> buf;
        clGetPlatformInfo(platform, paramaterName, buf.size(), buf.data(), NULL);
        return std::string(buf);
    }


    void cl_driver()
    {
        cl_uint numPlatforms;
        clGetPlatformIDs(0, NULL, &numPlatforms);

        std::vector<cl_platform_id> vPlatformIds(numPlatforms);
        clGetPlatformIDs(numPlatforms, &vPlatformIds[0], NULL);

        std::vector<SPlatform> vPlatforms;
        for (cl_platform_id platformId : vPlatformIds)
        {
            vPlatforms.push_back(SPlatform());
            vPlatforms.back().id = platformId;
        }

        std::vector<char> vInfoString(1024);
        char* pInfoString = &vInfoString[0];
        int iInfoStringMaxSize = vInfoString.size();


        for (cl_uint i = 0; i < numPlatforms; i++)
        {
            printf("\n %d. Platform \n", i + 1);

            for (int j = 0; j < g_platformAttributes.size(); j++)
            {
                clGetPlatformInfo(vPlatforms[i].id, g_platformAttributes[j].first, iInfoStringMaxSize, pInfoString, NULL);
                printf("  %d.%d %-11s: %s\n", i + 1, j + 1, g_platformAttributes[j].second, pInfoString);
            }

            // get all devices
            cl_uint deviceCount;
            clGetDeviceIDs(vPlatforms[i].id, CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);

            std::vector<cl_device_id>& vDevices = vPlatforms[i].vDevices;
            vDevices.resize(deviceCount);
            clGetDeviceIDs(vPlatforms[i].id, CL_DEVICE_TYPE_ALL, deviceCount, &vDevices[0], NULL);

            for (int iDevId = 0; iDevId < deviceCount; ++iDevId)
            {
                /*
                // print device name
                clGetDeviceInfo(vDevices[iDevId], CL_DEVICE_NAME, iInfoStringMaxSize, pInfoString, NULL);
                printf("\t%d. Device: %s\n", iDevId + 1, pInfoString);

                // print hardware device version
                clGetDeviceInfo(vDevices[iDevId], CL_DEVICE_VERSION, iInfoStringMaxSize, pInfoString, NULL);
                printf("\t %d.%d Hardware version: %s\n", iDevId + 1, 1, pInfoString);

                // print software driver version
                clGetDeviceInfo(vDevices[iDevId], CL_DRIVER_VERSION, iInfoStringMaxSize, pInfoString, NULL);
                printf("\t %d.%d Software version: %s\n", iDevId + 1, 2, pInfoString);

                // print c version supported by compiler for device
                clGetDeviceInfo(vDevices[iDevId], CL_DEVICE_OPENCL_C_VERSION, iInfoStringMaxSize, pInfoString, NULL);
                printf("\t %d.%d OpenCL C version: %s\n", iDevId + 1, 3, pInfoString);
                */

                for (int j = 0; j < g_deviceAttributes.size(); j++)
                {
                    clGetDeviceInfo(vDevices[iDevId], g_deviceAttributes[j].first, iInfoStringMaxSize, pInfoString, NULL);
                    printf("\t %d.%d %s: %s\n", iDevId + 1, 3, g_deviceAttributes[j].second, pInfoString);
                }


                // print parallel compute units
                cl_uint maxComputeUnits;
                clGetDeviceInfo(vDevices[iDevId], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, NULL);
                printf("\t %d.%d Parallel compute units: %d\n", iDevId + 1, 4, maxComputeUnits);


            }



            printf("\n");
        }


        if (!vPlatforms.empty() && !vPlatforms[0].vDevices.empty())
        {
            cl_device_id* pDevice = &vPlatforms[0].vDevices[0];

            cl_context context;
            context = clCreateContext(NULL, 1, pDevice, NULL, NULL, NULL);

            cl_command_queue queue;
            queue = clCreateCommandQueue(context, *pDevice, 0, NULL);


            {
                std::string programBuffer((std::istreambuf_iterator<char>(std::ifstream("program.cl"))), std::istreambuf_iterator<char>());
                const char* pszProgramBuffer = programBuffer.c_str();
                const size_t iProgramLength = programBuffer.length();

                if (!programBuffer.empty())
                {
                    cl_program program;
                    program = clCreateProgramWithSource(context, 1, &pszProgramBuffer, &iProgramLength, NULL);
                    cl_int buildResult = clBuildProgram(program, 1, pDevice, "-Werror -cl-std=CL1.1", NULL, NULL);
                    if (buildResult == CL_SUCCESS)
                    {
                        cl_kernel kernel;
                        kernel = clCreateKernel(program, "hello", NULL);

                        cl_mem kernelBuffer;
                        char hostBuffer[32];
                        kernelBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 32 * sizeof(char), NULL, NULL);
                        clSetKernelArg(kernel, 0, sizeof(cl_mem), &kernelBuffer);
                        clEnqueueTask(queue, kernel, 0, NULL, NULL);
                        clEnqueueReadBuffer(queue, kernelBuffer, CL_TRUE, 0, 32 * sizeof(char), hostBuffer, 0, NULL, NULL);

                        clFlush(queue);
                        clFinish(queue);

                        printf("%s\n", hostBuffer);

                        clReleaseKernel(kernel);
                    }
                    else
                    {
                        // check build error and build status first
                        cl_build_status status;
                        clGetProgramBuildInfo(program, *pDevice, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, NULL);

                        // check build log
                        clGetProgramBuildInfo(program, *pDevice, CL_PROGRAM_BUILD_LOG, iInfoStringMaxSize, pInfoString, NULL);
                        printf("Build failed:\n\terror=%d\n\tstatus=%d\n\tprogramLog:nn%s\n\n", buildResult, status, pInfoString);
                    }

                    clReleaseProgram(program);
                }
            }


            clReleaseCommandQueue(queue);
            clReleaseContext(context);
        }
    }

};

///////////////////////////////////////////////////



const bool g_pauseAtExit = true;



int main()
{
    init_cl();


    return 0;



    expression_traverser::config_t config;

    config.printAllCombinations = false;
    config.multiThreaded = true;
    config.printStats = true;

    config.benchmarkMode = true;
    config.benchmarkModeMaxCombinations = 500000000;

    config.generateResultArray = false;
    config.arraySize = 100;

    expression_traverser worker_threads(config);
    worker_threads.calculate();
	
	if (g_pauseAtExit)
	{
		system("pause");
	}
    return 0;
}

