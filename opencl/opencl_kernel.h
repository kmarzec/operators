#pragma once

#include "opencl_kernel_arg.h"

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
                std::array<char, 1024> buf;
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
                clEnqueueNDRangeKernel(
                    queue, 
                    m_kernel, 
                    workDimension,
                    nullptr,         // offset
                    &globalSize,
                    &localSize,
                    0, nullptr, nullptr
                );
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