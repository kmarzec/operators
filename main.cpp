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
#include <fstream>


const bool g_pauseAtExit = true;


void search_cpu()
{
    expression_traverser::config_t config;

    config.printAllCombinations = false;
    config.multiThreaded = true;
    config.printStats = true;

    config.benchmarkMode = true;
    config.benchmarkModeMaxCombinations = 5000000;

    config.generateResultArray = false;
    config.arraySize = 100;

    expression_traverser worker_threads(config);
    worker_threads.calculate();
}


void search_opencl()
{
    std::string programText((std::istreambuf_iterator<char>(std::ifstream("../program.cl"))), std::istreambuf_iterator<char>());


    timer timer;

    const size_t dataSize = 1394;
    const size_t resultDataSizeBytes = dataSize * sizeof(cl_long);


    opencl_driver driver;
    opencl_context_ptr context = driver.create_context();

    opencl_kernel kernel(context);
    kernel.build(programText, "hello");

    opencl_mem_buffer_ptr resultBuff = kernel.add_arg_buffer(CL_MEM_WRITE_ONLY, resultDataSizeBytes);


    kernel.enqeue_execute(1, (dataSize)/2, dataSize);
    resultBuff->enqueue_read();
    cl_long* result = (cl_long*)resultBuff->get_host_buffer();



    float time = timer.get_elapsed_time();

    

/*
    for (int i = 0; i < dataSize; ++i)
    {
        for (int j = 0; j < numOps; ++j)
        {
            printf("%d ", ops[i * 8 + j]);
        }

        printf("\t");

        for (int j = 0; j < numOpLocations; ++j)
        {
            printf("%d ", opsloc[i * 8 + j]);
        }

        printf("\n");
    }
    */

    printf("Time: %.2f\n", time);
    float fMCPS = (float)dataSize / (time*1000000.0f);
    printf("Millions of combinations/s: %.2f\n", fMCPS);
}


int main()
{
    //int64_t r = 0;
    //safe_ipow(2, 117, r);

    //expression<9>::init();
    //expression<9> e;
    //e.resolve_expression(1394);
    //e.evaluate();

    
    search_opencl();


    //search_cpu();
   
	
	if (g_pauseAtExit)
	{
		system("pause");
	}
    return 0;
}

