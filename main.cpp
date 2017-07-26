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

////////////////////////////



bool s_bPrintAllCombinations = false;
bool s_bPrintResultArray = false;
bool s_bMultiThreaded = true;
bool s_bPauseAtExit = true;
bool s_bDisplayStats = true;

bool s_bBenchmarkMode = true;
int64_t s_iBenchmarkModeMaxCombinations = 100000000;

const int arraySize = 12000;




///////////////////////////////

const int64_t NUM_DIGITS = 9;
const int64_t NUM_OP_SPOTS = NUM_DIGITS - 1;
const int64_t NUM_OPS = NUM_DIGITS - 1;


#include "expression.h"


int main()
{
	timer timer;

	int64_t iNumOpLocationCombinations = 1;
	for (int64_t j = 1; j <= NUM_OP_SPOTS; ++j)
	{
		iNumOpLocationCombinations *= j;
	}


	int64_t iNumOpCombinations = 1;
	for (int64_t j = 1; j <= NUM_OPS; ++j)
	{
		iNumOpCombinations *= static_cast<rpn_operation_type>(rpn_operation::__Count);
	}


	int64_t iNumTotalCombinations = iNumOpLocationCombinations * iNumOpCombinations;



	printf("NumDigits: %I64d\n", NUM_DIGITS);
	printf("NumOpSpots: %I64d\n", NUM_OP_SPOTS);
	printf("NumOps: %I64d\n", NUM_OPS);
	printf("iNumOpCombinations: %I64d\n", iNumOpCombinations);
	printf("iNumOpLocationCombinations: %I64d\n", iNumOpLocationCombinations);
	printf("iNumTotalCombinations: %I64d\n", iNumTotalCombinations);

	float fEstimatedMCPS = 16.1f;
	float fEstimatedTime = iNumTotalCombinations / (fEstimatedMCPS * 1000000.0f);
	printf("Estimated Time: %.2f\n", fEstimatedTime);


	std::vector<std::pair<expression, spin_lock>> results(arraySize);


	std::atomic<int64_t> iIntsInRange = 0;
	std::atomic<int64_t> iIntsPositive = 0;
	std::atomic<int64_t> iIntsNegative = 0;
	std::atomic<int64_t> iFractionsPositive = 0;
	std::atomic<int64_t> iFractionsNegative = 0;
	std::atomic<int64_t> iErrorsNANValue = 0;
	std::atomic<int64_t> iErrorsOverflow = 0;
	std::atomic<int64_t> iErrorsBadConcat = 0;
	std::atomic<int64_t> iErrorsExponentNotInteger = 0;
	std::atomic<int64_t> iTotal = 0;



	auto l = [&](int64_t start, int64_t end)
	{
		expression e;

		for (int64_t i = start; i < end; ++i)
		{
			int64_t iOpCombinationCode = i % iNumOpCombinations;
			int64_t iOpLocationCode = i / iNumOpCombinations;

			e.resolve_operations(iOpCombinationCode);
			e.resolve_operation_locations(iOpLocationCode);
			e.generate_rpn_expression();
			e.evaluate();

			if(s_bPrintAllCombinations)
			{
                // TOFIX printf from threads

				std::string epxString = e.to_string();
				printf(epxString.c_str());
				if ((e.get_result().is_number()))
				{
					if (e.get_result().number.denominator == 1)
					{
						printf(" = %I64d\n", e.get_result().number.numerator);
					}
					else
					{
						printf(" = %I64d/%I64d\n", e.get_result().number.numerator, e.get_result().number.denominator);
					}
				}
				else
				{
					printf(" = \n");
				}
			}

			if (s_bDisplayStats)
			{
				iTotal++;

				if (e.get_result().is_number())
				{
					assert(e.get_result().denominator > 0);

					if (e.get_result().number.numerator >= 0)
					{
						if (e.get_result().number.denominator == 1)
						{
							iIntsPositive++;

							if (e.get_result().number.numerator < arraySize)
							{
								iIntsInRange++;
							}
						}
						else
						{
							iFractionsPositive++;
						}
					}
					else
					{
						if (e.get_result().number.denominator == 1)
						{
							iIntsNegative++;
						}
						else
						{
							iFractionsNegative++;
						}
					}
				}
				else
				{
					assert(e.get_result().is_error_value());

					switch (e.get_result().get_type())
					{
					case item_type::NANValue:
						iErrorsNANValue++;
						break;
					case item_type::Overflow:
						iErrorsOverflow++;
						break;
					case item_type::BadConcat:
						iErrorsBadConcat++;
						break;
					case item_type::ExponentNotInteger:
						iErrorsExponentNotInteger++;
						break;
					default:
						assert(0);
					}
				}
			}


			if (s_bPrintResultArray)
			{
				if ((e.get_result().is_number()) && e.get_result().number.denominator == 1)
				{
					if (e.get_result().number.numerator >= 0 && e.get_result().number.numerator < arraySize)
					{
						results[e.get_result().number.numerator].second.lock();

						if (results[e.get_result().number.numerator].first.get_result().get_type() == item_type::None)
						{
							results[e.get_result().number.numerator].first = e;
						}

						results[e.get_result().number.numerator].second.unlock();
					}
				}
			}


		}
	};

	if (s_bBenchmarkMode)
	{
		iNumTotalCombinations = iNumTotalCombinations > s_iBenchmarkModeMaxCombinations ? s_iBenchmarkModeMaxCombinations : iNumTotalCombinations;
	}

	std::vector<std::thread> vThreads;
	int64_t numThreads = s_bMultiThreaded ? std::thread::hardware_concurrency() : 1;
	int64_t combPerThread = iNumTotalCombinations / numThreads;

	for (int t = 0; t < numThreads; ++t)
	{
		int64_t start = t * combPerThread;
		int64_t end = (t + 1) * combPerThread;
		if (t == numThreads-1)
		{
			end = iNumTotalCombinations;
		}

		vThreads.emplace_back(l, start, end);
	}

	for (std::thread& thread : vThreads)
	{
		thread.join();
	}

	float time = timer.get_elapsed_time();
	
	if (s_bPrintResultArray)
	{
		for (int64_t i = 0; i < arraySize; ++i)
		{
			std::string epxString = results[i].first.to_string();
			printf("%I64d = %s\n", i, epxString.c_str());
		}
	}

	if (s_bDisplayStats)
	{
		printf("iIntsInRange = %I64d (%.2f%%)\n", (int64_t)iIntsInRange, (float)iIntsInRange / (float)iTotal * 100.0f);
		printf("iIntsPositive = %I64d (%.2f%%)\n", (int64_t)iIntsPositive, (float)iIntsPositive / (float)iTotal * 100.0f);
		printf("iIntsNegative = %I64d (%.2f%%)\n", (int64_t)iIntsNegative, (float)iIntsNegative / (float)iTotal * 100.0f);
		printf("iFractionsPositive = %I64d (%.2f%%)\n", (int64_t)iFractionsPositive, (float)iFractionsPositive / (float)iTotal * 100.0f);
		printf("iFractionsNegative = %I64d (%.2f%%)\n", (int64_t)iFractionsNegative, (float)iFractionsNegative / (float)iTotal * 100.0f);
		printf("iErrorsNANValue = %I64d (%.2f%%)\n", (int64_t)iErrorsNANValue, (float)iErrorsNANValue / (float)iTotal * 100.0f);
		printf("iErrorsOverflow = %I64d (%.2f%%)\n", (int64_t)iErrorsOverflow, (float)iErrorsOverflow / (float)iTotal * 100.0f);
		printf("iErrorsBadConcat = %I64d (%.2f%%)\n", (int64_t)iErrorsBadConcat, (float)iErrorsBadConcat / (float)iTotal * 100.0f);
		printf("iErrorsExponentNotInteger = %I64d (%.2f%%)\n", (int64_t)iErrorsExponentNotInteger, (float)iErrorsExponentNotInteger / (float)iTotal * 100.0f);

	}
	
	printf("Time: %.2f\n", time);
	float fMCPS = (float)iNumTotalCombinations / (time*1000000.0f);
	printf("Millions of combinations/s: %.2f\n", fMCPS);

	if (s_bPauseAtExit)
	{
		system("pause");
	}
    return 0;
}

