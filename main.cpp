// ascendingproblem.cpp : Defines the entry point64_t for the console application.
//

#include "stdafx.h"
#include <cstdint>
#include <cstdlib>


const int64_t NUM_DIGITS = 3;


const int64_t NUM_OP_SPOTS = NUM_DIGITS - 1;
const int64_t NUM_OPS = NUM_DIGITS - 1;



enum EOps
{
	EOP_PLUS,
	EOP_MINUS,
	EOP_MUL,
	EOP_DIV,
	EOP_POW,
	EOP_CONCAT,

	__EOP_COUNT
};


char acOpSymbols[] = {'+', '-', '*', '/', '^', '|'};




int main()
{
	int64_t iNumOpLocationCombinations = 1;
	for (int64_t j = 1; j <= NUM_OP_SPOTS; ++j)
	{
		iNumOpLocationCombinations *= j;
	}


	int64_t iNumOpCombinations = 1;
	for (int64_t j = 1; j <= NUM_OPS; ++j)
	{
		iNumOpCombinations *= __EOP_COUNT;
	}


	int64_t iNumTotalCombinations = iNumOpLocationCombinations * iNumOpCombinations;





	printf("NumDigits: %I64d\n", NUM_DIGITS);
	printf("NumOpSpots: %I64d\n", NUM_OP_SPOTS);
	printf("NumOps: %I64d\n", NUM_OPS);
	printf("iNumOpCombinations: %I64d\n", iNumOpCombinations);
	printf("iNumOpLocationCombinations: %I64d\n", iNumOpLocationCombinations);
	printf("iNumTotalCombinations: %I64d\n", iNumTotalCombinations);



	//return 0; 

	for (int64_t i = 0; i < iNumTotalCombinations; ++i)
	{
		int64_t iOpCombinationCode = i % iNumOpCombinations;


		int64_t aiOps[NUM_OPS];
		for (int64_t n = 0; n < NUM_OPS; ++n)
		{
			aiOps[n] = iOpCombinationCode % __EOP_COUNT;
			iOpCombinationCode /= __EOP_COUNT;
		}

		//for (int64_t j = 0; j < NUM_OPS; ++j)
		//{
		//	print64_tf("%d, ", aiOps[j]);
		//}
		//print64_tf(" : ");


		int64_t iOpLocationCode = i / iNumOpCombinations;

		int64_t aiOpLocations[NUM_OP_SPOTS];
		for (int64_t n = 0; n < NUM_OP_SPOTS; ++n)
		{
			aiOpLocations[n] = 0;
		}

		aiOpLocations[0]++;
		for (int64_t n = 2; n <= NUM_OPS; ++n)
		{
			int64_t iOpLocation = iOpLocationCode%n;
			aiOpLocations[iOpLocation]++;
			iOpLocationCode /= n;
		}


		//for (int64_t j = 0; j < NUM_OPS; ++j)
		//{
		//	print64_tf("%d, ", aiOpLocations[j]);
		//}
		//print64_tf("\n");


		char acExp[NUM_DIGITS + NUM_OPS + 1];
		acExp[NUM_DIGITS + NUM_OPS] = 0;

		int64_t iCharPtr = NUM_DIGITS + NUM_OPS -1;
		int64_t iNumDigits = 0;
		int64_t iOpIdx = 0;

		while (iCharPtr >= 0)
		{
			if (iNumDigits < NUM_OP_SPOTS && aiOpLocations[iNumDigits] > 0)
			{
				acExp[iCharPtr] = acOpSymbols[aiOps[iOpIdx++]];
				aiOpLocations[iNumDigits]--;
			}
			else
			{
				
				acExp[iCharPtr] = '0' + (char)(NUM_DIGITS - iNumDigits);
				iNumDigits++;
			}

			iCharPtr--;
		}

		printf("%s\n", acExp);


	}




	system("pause");
    return 0;
}

