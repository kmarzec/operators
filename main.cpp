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

/*
const char* programText =
R"(   
    __kernel void hello(__global int* ops)
    {
        int NUMBER_OF_DIGITS = 9;
        int m_number_of_operator_locatations = NUMBER_OF_DIGITS - 1;
        int m_number_of_operators = NUMBER_OF_DIGITS - 1;

                        


                        int iNumOpLocationCombinations = 40320;
            int iNumOpCombinations = 1679616;
int iNumTotalCombinations = 67722117120;

int rpn_operation__Count = 6;

        /////////////////////

           int id = get_global_id(0);


            long iOpCombinationCode = id % iNumOpCombinations;
             long iOpLocationCode = id / iNumOpCombinations;



            for (int n = 0; n < m_number_of_operators; ++n)
	        {
		        ops[m_number_of_operators*id + n] = iOpCombinationCode % rpn_operation__Count;
		        iOpCombinationCode /= rpn_operation__Count;
	        }


	   //int id = get_global_id(0);
       // v3[id] = v1[id] + v2[id]; 
    }
)";
*/

void search_cpu()
{
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
}


void search_opencl()
{
    /*
    const size_t dataSize = 5;
    const size_t dataSizeBytes = dataSize * sizeof(float);
    float v1[] = { 1, 2, 3, 4, 5 };
    float v2[] = { 6, 7, 8, 9, 10 };


    opencl_driver driver;
    opencl_context_ptr context = driver.create_context();

    opencl_kernel kernel(context);
    kernel.build(programText, "hello");

    opencl_mem_buffer_ptr bv1 = kernel.add_arg_buffer(CL_MEM_READ_ONLY, dataSizeBytes);
    memcpy(bv1->get_host_buffer(), v1, sizeof(v1));
    bv1->enqueue_write();

    opencl_mem_buffer_ptr bv2 = kernel.add_arg_buffer(CL_MEM_READ_ONLY, dataSizeBytes);
    memcpy(bv2->get_host_buffer(), v2, sizeof(v2));
    bv2->enqueue_write();

    opencl_mem_buffer_ptr bv3 = kernel.add_arg_buffer(CL_MEM_WRITE_ONLY, dataSizeBytes);

    kernel.enqeue_execute(1, 5, 5);

    bv3->enqueue_read();

    float* v3 = (float*)bv3->get_host_buffer();

    for (int i = 0; i < dataSize; ++i)
    {
        printf("%.2f\n", v3[i]);
    }*/


    const size_t dataSize = 32;

    const size_t numOps = 8;
    const size_t opsSizeBytes = dataSize * (sizeof(cl_int) * numOps);

    const size_t numOpLocations = 8;
    const size_t opsLocationsSizeBytes = dataSize * (sizeof(cl_int) * numOpLocations);


    opencl_driver driver;
    opencl_context_ptr context = driver.create_context();

    opencl_kernel kernel(context);
    std::string programText((std::istreambuf_iterator<char>(std::ifstream("../program.cl"))), std::istreambuf_iterator<char>());
    kernel.build(programText, "hello");

    opencl_mem_buffer_ptr opsbuff = kernel.add_arg_buffer(CL_MEM_WRITE_ONLY, opsSizeBytes);
    opencl_mem_buffer_ptr opslocationsbuff = kernel.add_arg_buffer(CL_MEM_WRITE_ONLY, opsLocationsSizeBytes);


    kernel.enqeue_execute(1, dataSize, dataSize);
    opsbuff->enqueue_read();
    opslocationsbuff->enqueue_read();


    cl_int* ops = (cl_int*)opsbuff->get_host_buffer();
    cl_int* opsloc = (cl_int*)opslocationsbuff->get_host_buffer();


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
}


int main()
{
    
    search_opencl();


    //search_cpu();
   
	
	if (g_pauseAtExit)
	{
		system("pause");
	}
    return 0;
}

