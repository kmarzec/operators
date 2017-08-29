#pragma once

#include <CL/cl.h>

class opencl_context
{
public:
    opencl_context(cl_device_id deviceId)
        : m_deviceId(deviceId)
        , m_context(nullptr)
        , m_queue(nullptr)
    {
        m_context = clCreateContext(nullptr, 1, &deviceId, nullptr, nullptr, nullptr);
        m_queue = clCreateCommandQueue(m_context, deviceId, 0, nullptr);
    }

    ~opencl_context()
    {
        if (m_queue)
        {
            clReleaseCommandQueue(m_queue);
            m_queue = nullptr;
        }

        if (m_context)
        {
            clReleaseContext(m_context);
            m_context = nullptr;
        }

        m_deviceId = nullptr;
    }

    void flush()
    {
        if (m_queue)
        {
            clFlush(m_queue);
            clFinish(m_queue);
        }
    }

    cl_context get_cl_context() { return m_context; }
    cl_device_id get_device_id() { return m_deviceId; }
    cl_command_queue get_cl_queue() { return m_queue; }

private:
    cl_device_id m_deviceId;
    cl_context m_context;
    cl_command_queue m_queue;
};

typedef std::shared_ptr<opencl_context> opencl_context_ptr;