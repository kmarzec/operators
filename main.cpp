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


const bool g_pauseAtExit = true;



int main()
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
	
	if (g_pauseAtExit)
	{
		system("pause");
	}
    return 0;
}

