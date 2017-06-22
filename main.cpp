// ascendingproblem.cpp : Defines the entry point64_t for the console application.
//

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cassert>

#include <vector>
#include <stack>

#include <string>


const int64_t NUM_DIGITS = 6;


const int64_t NUM_OP_SPOTS = NUM_DIGITS - 1;
const int64_t NUM_OPS = NUM_DIGITS - 1;



enum EOps
{
	EOP_PLUS,
	EOP_MINUS,
	EOP_MUL,
	EOP_DIV,

	__EOP_COUNT,

	
	EOP_POW,
	EOP_CONCAT
};


char acOpSymbols[] = {'+', '-', '*', '/', '^', '|'};



struct Expression
{
	struct RPNStackItem
	{
		enum Type
		{
			None,
			NANValue,
			SourceValue,
			CalculatedValue,
			Operand
		};

		Type type;
		int64_t numerator;
		int64_t denominator;
		EOps op;

		RPNStackItem()
			: type(None)
			, op(__EOP_COUNT)
			, numerator(-1)
			, denominator(-1)
		{}

		RPNStackItem(EOps op)
			: type(Operand)
			, op(op)
			, numerator(-1)
			, denominator(-1)
		{}

		RPNStackItem(int64_t val)
			: type(SourceValue)
			, op(__EOP_COUNT)
			, numerator(val)
			, denominator(1)
		{}

		RPNStackItem(int64_t nominator, int64_t denominator)
			: type(CalculatedValue)
			, op(__EOP_COUNT)
			, numerator(nominator)
			, denominator(denominator)
		{
			if (denominator == 0)
			{
				type = NANValue;
			}
		}

		void Reduce()
		{
			assert(type == CalculatedValue || type == NANValue);

			if (type == CalculatedValue)
			{

				if (denominator < 0)
				{
					numerator = -numerator;
					denominator = -denominator;
				}

				if (denominator != 1)
				{
					if (numerator % denominator == 0)
					{
						numerator = numerator / denominator;
						denominator = 1;
					}
				}
			}
		}
	};

	EOps aiOps[NUM_OPS];
	int64_t aiOpLocations[NUM_OP_SPOTS];
	std::vector<RPNStackItem> m_vRPNExpression;
	std::string m_expString;
	RPNStackItem m_result;
	
	void ResolveOps(int64_t iOpCombinationCode)
	{
		for (int64_t n = 0; n < NUM_OPS; ++n)
		{
			aiOps[n] = (EOps)(iOpCombinationCode % __EOP_COUNT);
			iOpCombinationCode /= __EOP_COUNT;
		}
	}

	void ResolveOpLocations(int64_t iOpLocationCode)
	{
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
	}

	void GenerateRPNExpression()
	{
		//char acExp[NUM_DIGITS + NUM_OPS + 1];
		//acExp[NUM_DIGITS + NUM_OPS] = 0;

		m_vRPNExpression.clear();
		m_vRPNExpression.resize(NUM_DIGITS + NUM_OPS);



		int64_t iCharPtr = NUM_DIGITS + NUM_OPS - 1;
		int64_t iNumDigits = 0;
		int64_t iOpIdx = 0;

		while (iCharPtr >= 0)
		{
			if (iNumDigits < NUM_OP_SPOTS && aiOpLocations[iNumDigits] > 0)
			{
				m_vRPNExpression[iCharPtr] = RPNStackItem(aiOps[iOpIdx++]);
				aiOpLocations[iNumDigits]--;
			}
			else
			{
				m_vRPNExpression[iCharPtr] = RPNStackItem(NUM_DIGITS - iNumDigits);
				iNumDigits++;
			}

			iCharPtr--;
		}

		//printf("%s\n", acExp);

		m_expString.clear();

		for (const RPNStackItem& item : m_vRPNExpression)
		{
			switch (item.type)
			{
			case RPNStackItem::Operand:
				m_expString += acOpSymbols[item.op];
				break;
			case RPNStackItem::SourceValue:
				m_expString += std::to_string(item.numerator);
				break;

			default:
				assert(0);
			};
		}

		//expString += "\n";
		//printf(expString.c_str());
	}

	void EvaluateRPNExpression()
	{
		std::stack<RPNStackItem> stack;

		for (const RPNStackItem& item : m_vRPNExpression)
		{
			switch (item.type)
			{
			case RPNStackItem::Operand:
			{
				assert(stack.size() >= 2);
				RPNStackItem val2 = stack.top();
				stack.pop();
				RPNStackItem val1 = stack.top();
				stack.pop();
				assert(val1.type == RPNStackItem::SourceValue || val1.type == RPNStackItem::CalculatedValue);
				assert(val2.type == RPNStackItem::SourceValue || val2.type == RPNStackItem::CalculatedValue);

				switch (item.op)
				{
				case EOP_PLUS:
				{
					int64_t nominator = val1.numerator * val2.denominator + val2.numerator * val1.denominator;
					int64_t denominator = val1.denominator * val2.denominator;
					RPNStackItem result(nominator, denominator);
					result.Reduce();
					stack.push(result);
					break;
				}
				case EOP_MINUS:
				{
					int64_t nominator = val1.numerator * val2.denominator - val2.numerator * val1.denominator;
					int64_t denominator = val1.denominator * val2.denominator;
					RPNStackItem result(nominator, denominator);
					result.Reduce();
					stack.push(result);
					break;
				}
				case EOP_MUL:
				{
					int64_t nominator = val1.numerator * val2.numerator;
					int64_t denominator = val1.denominator * val2.denominator;
					RPNStackItem result(nominator, denominator);
					result.Reduce();
					stack.push(result);
					break;
				}
				case EOP_DIV:
				{
					int64_t nominator = val1.numerator * val2.denominator;
					int64_t denominator = val1.denominator * val2.numerator;
					RPNStackItem result(nominator, denominator);
					result.Reduce();
					stack.push(result);
					break;
				}
				default:
					assert(0);
				};

				break;
			}

			case RPNStackItem::SourceValue:
				stack.push(item);
				break;

			default:
				assert(0);
			};

			assert(stack.size() > 0);
			if (stack.top().type == RPNStackItem::NANValue)
			{
				break;
			}
		}

		assert(stack.size() == 1 || stack.top().type == RPNStackItem::NANValue);
		assert(
			stack.top().type == RPNStackItem::SourceValue || 
			stack.top().type == RPNStackItem::CalculatedValue || 
			stack.top().type == RPNStackItem::NANValue);

		/*
		if (stack.top().denominator == 1)
		{
			printf(" = %I64d\n", stack.top().numerator);
		}
		else
		{
			printf(" = %I64d/%I64d\n", stack.top().numerator, stack.top().denominator);
		}*/

		m_result = stack.top();
	}
};



class Timer
{
public:
	Timer()
	{
		m_start = clock();
	}

	float GetElapsedTime()
	{
		clock_t t = clock() - m_start;
		return (float)t / (float)CLOCKS_PER_SEC;
	}

private:
	clock_t m_start;
};



int main()
{
	Timer timer;

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



	std::vector<Expression> results(100);


	for (int64_t i = 0; i < iNumTotalCombinations; ++i)
	{
		int64_t iOpCombinationCode = i % iNumOpCombinations;
		int64_t iOpLocationCode = i / iNumOpCombinations;

		Expression e;

		e.ResolveOps(iOpCombinationCode);
		e.ResolveOpLocations(iOpLocationCode);
		e.GenerateRPNExpression();
		e.EvaluateRPNExpression();

		if ((e.m_result.type == Expression::RPNStackItem::SourceValue || e.m_result.type == Expression::RPNStackItem::CalculatedValue) && e.m_result.denominator == 1)
		{
			if (e.m_result.numerator >= 0 && e.m_result.numerator < 100)
			{
				if (results[e.m_result.numerator].m_result.type == Expression::RPNStackItem::None)
				{
					results[e.m_result.numerator] = e;
				}
			}
		}
	}



	for (int64_t i = 0; i < 100; ++i)
	{
		printf("%I64d = %s\n", i, results[i].m_expString.c_str());
	}



	printf("Time: %.2f\n", timer.GetElapsedTime());

	system("pause");
    return 0;
}

