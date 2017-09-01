#pragma once

#include "opencl_kernel_arg.h"


const char* clGetErrorString(cl_int error)
{
	switch (error) {
		// run-time and JIT compiler errors
	case 0: return "CL_SUCCESS";
	case -1: return "CL_DEVICE_NOT_FOUND";
	case -2: return "CL_DEVICE_NOT_AVAILABLE";
	case -3: return "CL_COMPILER_NOT_AVAILABLE";
	case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case -5: return "CL_OUT_OF_RESOURCES";
	case -6: return "CL_OUT_OF_HOST_MEMORY";
	case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case -8: return "CL_MEM_COPY_OVERLAP";
	case -9: return "CL_IMAGE_FORMAT_MISMATCH";
	case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case -11: return "CL_BUILD_PROGRAM_FAILURE";
	case -12: return "CL_MAP_FAILURE";
	case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case -15: return "CL_COMPILE_PROGRAM_FAILURE";
	case -16: return "CL_LINKER_NOT_AVAILABLE";
	case -17: return "CL_LINK_PROGRAM_FAILURE";
	case -18: return "CL_DEVICE_PARTITION_FAILED";
	case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

		// compile-time errors
	case -30: return "CL_INVALID_VALUE";
	case -31: return "CL_INVALID_DEVICE_TYPE";
	case -32: return "CL_INVALID_PLATFORM";
	case -33: return "CL_INVALID_DEVICE";
	case -34: return "CL_INVALID_CONTEXT";
	case -35: return "CL_INVALID_QUEUE_PROPERTIES";
	case -36: return "CL_INVALID_COMMAND_QUEUE";
	case -37: return "CL_INVALID_HOST_PTR";
	case -38: return "CL_INVALID_MEM_OBJECT";
	case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case -40: return "CL_INVALID_IMAGE_SIZE";
	case -41: return "CL_INVALID_SAMPLER";
	case -42: return "CL_INVALID_BINARY";
	case -43: return "CL_INVALID_BUILD_OPTIONS";
	case -44: return "CL_INVALID_PROGRAM";
	case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
	case -46: return "CL_INVALID_KERNEL_NAME";
	case -47: return "CL_INVALID_KERNEL_DEFINITION";
	case -48: return "CL_INVALID_KERNEL";
	case -49: return "CL_INVALID_ARG_INDEX";
	case -50: return "CL_INVALID_ARG_VALUE";
	case -51: return "CL_INVALID_ARG_SIZE";
	case -52: return "CL_INVALID_KERNEL_ARGS";
	case -53: return "CL_INVALID_WORK_DIMENSION";
	case -54: return "CL_INVALID_WORK_GROUP_SIZE";
	case -55: return "CL_INVALID_WORK_ITEM_SIZE";
	case -56: return "CL_INVALID_GLOBAL_OFFSET";
	case -57: return "CL_INVALID_EVENT_WAIT_LIST";
	case -58: return "CL_INVALID_EVENT";
	case -59: return "CL_INVALID_OPERATION";
	case -60: return "CL_INVALID_GL_OBJECT";
	case -61: return "CL_INVALID_BUFFER_SIZE";
	case -62: return "CL_INVALID_MIP_LEVEL";
	case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
	case -64: return "CL_INVALID_PROPERTY";
	case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
	case -66: return "CL_INVALID_COMPILER_OPTIONS";
	case -67: return "CL_INVALID_LINKER_OPTIONS";
	case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

		// extension errors
	case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
	case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
	case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
	case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
	case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	default: return "Unknown OpenCL error";
	}
}



class opencl_kernel
{
public:
    opencl_kernel(opencl_context_ptr context)
        : m_context(context)
        , m_program(nullptr)
        , m_kernel(nullptr)
    {
    }

    virtual ~opencl_kernel()
    {
        reset();
    }

    void reset()
    {
        if (m_kernel)
        {
            clReleaseKernel(m_kernel);
            m_kernel = nullptr;
        }

        if (m_program)
        {
            clReleaseProgram(m_program);
            m_program = nullptr;
        }

        m_programText.clear();
        m_kernelFunctionName.clear();
        m_args.clear();
    }

    void build(const std::string& programText, const std::string& kernelFunctionName)
    {
        m_programText = programText;
        m_kernelFunctionName = kernelFunctionName;

        if (m_context)
        {
            const char* programTextBuffer = m_programText.c_str();
            size_t programTextSize = m_programText.length();

            m_program = clCreateProgramWithSource(m_context->get_cl_context(), 1, &programTextBuffer, &programTextSize, nullptr);
            cl_device_id deviceId = m_context->get_device_id();
            cl_int buildResult = clBuildProgram(m_program, 1, &deviceId, "-Werror -cl-std=CL1.1", nullptr, nullptr);
            if (buildResult == CL_SUCCESS)
            {
                m_kernel = clCreateKernel(m_program, m_kernelFunctionName.c_str(), NULL);
            }
            else
            {
                std::array<char, 4096> buf;
                clGetProgramBuildInfo(m_program, deviceId, CL_PROGRAM_BUILD_LOG, buf.size(), buf.data(), nullptr);
                printf("Kernel Build failed:\n%s\n", buf.data());
            }
        }
    }

    opencl_mem_buffer_ptr add_arg_buffer(cl_mem_flags flags, size_t size)
    {
        opencl_mem_buffer_ptr buffer = std::make_shared<opencl_mem_buffer>(m_context, flags, size);
        m_args.push_back(std::make_shared<opencl_kernel_arg_buffer>(buffer));
        return buffer;
    }

    void enqeue_execute()
    {
        if (m_context && m_program && m_kernel)
        {
            set_kernel_args();

            cl_command_queue queue = m_context->get_cl_queue();
            if (queue)
            {
                clEnqueueTask(queue, m_kernel, 0, nullptr, nullptr);
            }
        }
    }

    void enqeue_execute(cl_uint workDimension, size_t localSize, size_t globalSize)
    {
        if (m_context && m_program && m_kernel)
        {
            set_kernel_args();

            cl_command_queue queue = m_context->get_cl_queue();
            if (queue)
            {
                cl_int result = clEnqueueNDRangeKernel(
                    queue, 
                    m_kernel, 
                    workDimension,
                    nullptr,         // offset
                    &globalSize,
                    &localSize,
                    0, nullptr, nullptr
                );

				if (result != CL_SUCCESS)
				{
					printf("ERORR: %s\n", clGetErrorString(result));
					abort();
				}

				//clFlush(queue);
				//clFinish(queue);
            }
        }
    }

    opencl_kernel_arg_ptr get_agr(size_t agrIdx)
    {
        return m_args[agrIdx];
    }

private:

    void set_kernel_args()
    {
        cl_uint argIdx = 0;
        for (opencl_kernel_arg_ptr& arg : m_args)
        {
            switch (arg->get_type())
            {
            case opencl_kernel_arg::type_t::buffer:
                opencl_kernel_arg_buffer* bufferArg = (opencl_kernel_arg_buffer*)arg.get();
                cl_mem memBuffer = bufferArg->get_buffer()->get_cl_buffer();
                clSetKernelArg(m_kernel, argIdx, sizeof(cl_mem), &memBuffer);
                break;
            }

            argIdx++;
        }
    }

private:
    opencl_context_ptr m_context;

    std::string m_programText;
    std::string m_kernelFunctionName;
    std::vector<opencl_kernel_arg_ptr> m_args;
    cl_program m_program;
    cl_kernel m_kernel;
};

typedef std::shared_ptr<opencl_kernel> opencl_kernel_ptr;