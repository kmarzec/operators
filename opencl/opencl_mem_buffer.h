#pragma once

class opencl_mem_buffer
{
public:
    opencl_mem_buffer(opencl_context_ptr context, cl_mem_flags flags, size_t size)
        : m_context(context)
        , m_buffer(nullptr)
        , m_size(0)
        , m_deviceBuffer(nullptr)
        , m_flags(flags)
    {
        reset(nullptr, size);
    }

    virtual ~opencl_mem_buffer()
    {
        reset(nullptr, 0);
    }

    void reset(void* buffer, size_t size)
    {
        if (m_deviceBuffer)
        {
            clReleaseMemObject(m_deviceBuffer);
            m_deviceBuffer = nullptr;
        }

        m_buffer.reset((uint8_t*)buffer);
        m_size = size;

        if (m_context && m_size)
        {
            m_deviceBuffer = clCreateBuffer(m_context->get_cl_context(), m_flags, m_size, nullptr, nullptr);
        }
    }

    void enqueue_read()
    {
        if (m_context && m_deviceBuffer)
        {
            uint8_t* buffer = get_host_buffer();
            cl_command_queue queue = m_context->get_cl_queue();
            if (queue && buffer)
            {
                clEnqueueReadBuffer(queue, m_deviceBuffer, CL_TRUE, 0, m_size, m_buffer.get(), 0, nullptr, nullptr);
            }
        }
    }

    void enqueue_write()
    {
        if (m_context && m_deviceBuffer)
        {
            uint8_t* buffer = get_host_buffer();
            cl_command_queue queue = m_context->get_cl_queue();
            if (queue && buffer)
            {
                clEnqueueWriteBuffer(queue, m_deviceBuffer, CL_TRUE, 0, m_size, buffer, 0, nullptr, nullptr);
            }
        }
    }

    cl_mem get_cl_buffer() { return m_deviceBuffer; }

    uint8_t* get_host_buffer() 
    {
        if (!m_buffer && m_size)
        {
            m_buffer.reset(new uint8_t[m_size]);
        }

        return m_buffer.get(); 
    }

private:
    opencl_context_ptr m_context;
    std::unique_ptr<uint8_t[]> m_buffer;
    size_t m_size;
    cl_mem m_deviceBuffer;
    cl_mem_flags m_flags;
};

typedef std::shared_ptr<opencl_mem_buffer> opencl_mem_buffer_ptr;