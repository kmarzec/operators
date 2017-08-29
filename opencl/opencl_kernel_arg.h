#pragma once

#include "opencl_mem_buffer.h"

///////////////////////////////////////////////////


class opencl_kernel_arg
{
public:
    enum class type_t
    {
        buffer
    };

    opencl_kernel_arg(type_t type)
        : m_type(type)
    {}

    virtual ~opencl_kernel_arg() {};

    type_t get_type() { return m_type; }

private:
    type_t m_type;
};

typedef std::shared_ptr<opencl_kernel_arg> opencl_kernel_arg_ptr;


///////////////////////////////////////////////////


class opencl_kernel_arg_buffer
    : public opencl_kernel_arg
{
public:
    opencl_kernel_arg_buffer(opencl_mem_buffer_ptr buffer)
        : opencl_kernel_arg(type_t::buffer)
        , m_buffer(buffer)
    {}

    opencl_mem_buffer_ptr get_buffer() { return m_buffer; }

private:
    opencl_mem_buffer_ptr m_buffer;
};

typedef std::shared_ptr<opencl_kernel_arg_buffer> opencl_kernel_arg_buffer_ptr;