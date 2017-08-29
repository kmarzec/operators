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


#include "opencl/opencl_driver.h"
#include "opencl/opencl_kernel.h"


const bool g_pauseAtExit = true;


const char* programText = 
R"(   
    __kernel void hello(__global char* message)
    {
	    message[0] = 'H';
	    message[1] = 'e';
	    message[2] = 'l';
	    message[3] = 'l';
	    message[4] = 'o';
	    message[5] = ',';
	    message[6] = ' ';
	    message[7] = 'W';
	    message[8] = 'o';
	    message[9] = 'r';
	    message[10] = 'l';
	    message[11] = 'd';
	    message[12] = '!';
	    message[13] = 0;
    }
)";


int main()
{
    opencl_driver driver;
    opencl_context_ptr context = driver.create_context();

    opencl_kernel kernel(context);
    kernel.build(programText, "hello");
    opencl_mem_buffer_ptr buf = kernel.add_arg_buffer(CL_MEM_WRITE_ONLY, 32);
    kernel.enqeue_execute();
    buf->enqueue_read();

    printf("%s\n", buf->get_host_buffer());

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

