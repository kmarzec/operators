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


const int64_t NUM_DIGITS = 3;


const int64_t NUM_OP_SPOTS = NUM_DIGITS - 1;
const int64_t NUM_OPS = NUM_DIGITS - 1;



enum EOps
{
	EOP_PLUS,
	EOP_MINUS,
	EOP_MUL,
	EOP_DIV,
	EOP_CONCAT,

	__EOP_COUNT,

	
	EOP_POW
};


char acOpSymbols[] = {'+', '-', '*', '/', '|', '^'};



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
			//, op(__EOP_COUNT)
			//, numerator(-1)
			//, denominator(-1)
		{}

		RPNStackItem(EOps op)
			: type(Operand)
			, op(op)
			//, numerator(-1)
			//, denominator(-1)
		{}

		RPNStackItem(int64_t val)
			: type(SourceValue)
			//, op(__EOP_COUNT)
			, numerator(val)
			, denominator(1)
		{}

		RPNStackItem(int64_t nominator, int64_t denominator)
			: type(CalculatedValue)
			//, op(__EOP_COUNT)
			, numerator(nominator)
			, denominator(denominator)
		{
			if (denominator == 0)
			{
				type = NANValue;
			}
		}

		void SetOp(EOps o)
		{
			type = Operand;
			op = o;
			//numerator = -1;
			//denominator = -1;
		}

		void SetSourceVal(int64_t val)
		{
			type = SourceValue;
			//op = __EOP_COUNT;
			numerator = val;
			denominator = 1;
		}

		void SetCalculatedVal(int64_t nominator, int64_t d)
		{
			type = (d == 0) ? NANValue : CalculatedValue;
			//op = __EOP_COUNT;
			numerator = nominator;
			denominator = d;
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
	RPNStackItem m_vRPNExpression[NUM_DIGITS + NUM_OPS];
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
		int64_t iCharPtr = NUM_DIGITS + NUM_OPS - 1;
		int64_t iNumDigits = 0;
		int64_t iOpIdx = 0;

		while (iCharPtr >= 0)
		{
			if (iNumDigits < NUM_OP_SPOTS && aiOpLocations[iNumDigits] > 0)
			{
				m_vRPNExpression[iCharPtr].SetOp(aiOps[iOpIdx++]);
				aiOpLocations[iNumDigits]--;
			}
			else
			{
				m_vRPNExpression[iCharPtr].SetSourceVal(NUM_DIGITS - iNumDigits);
				iNumDigits++;
			}

			iCharPtr--;
		}	
	}

	std::string ToRPNString()
	{
		std::string resultStr;

		for (const RPNStackItem& item : m_vRPNExpression)
		{
			switch (item.type)
			{
			case RPNStackItem::Operand:
				resultStr += acOpSymbols[item.op];
				break;
			case RPNStackItem::SourceValue:
				resultStr += std::to_string(item.numerator);
				break;

			default:
				assert(0);
			};
		}

		return resultStr;
	}


	class RPNStack
	{
	public:

		RPNStack()
			: ptr(-1)
		{}

		RPNStackItem& push()
		{
			assert(ptr < NUM_DIGITS-1);
			return m_stack[++ptr];
		}

		void pop()
		{
			assert(ptr >= 0);
			ptr--;
		}

		const RPNStackItem& top()
		{
			return m_stack[ptr];
		}

		int64_t size()
		{
			return ptr+1;
		}

		void clear()
		{
			ptr = -1;
		}

	private:

		RPNStackItem m_stack[NUM_DIGITS];
		int64_t ptr;
	};

	RPNStack stack;


	void EvaluateRPNExpression()
	{
		//std::stack<RPNStackItem> stack;
		stack.clear();


		for (const RPNStackItem& item :  m_vRPNExpression)
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
					RPNStackItem& result = stack.push();
					result.SetCalculatedVal(nominator, denominator);
					result.Reduce();
					break;
				}
				case EOP_MINUS:
				{
					int64_t nominator = val1.numerator * val2.denominator - val2.numerator * val1.denominator;
					int64_t denominator = val1.denominator * val2.denominator;
					RPNStackItem& result = stack.push();
					result.SetCalculatedVal(nominator, denominator);
					result.Reduce();
					break;
				}
				case EOP_MUL:
				{
					int64_t nominator = val1.numerator * val2.numerator;
					int64_t denominator = val1.denominator * val2.denominator;
					RPNStackItem& result = stack.push();
					result.SetCalculatedVal(nominator, denominator);
					result.Reduce();
					break;
				}
				case EOP_DIV:
				{
					int64_t nominator = val1.numerator * val2.denominator;
					int64_t denominator = val1.denominator * val2.numerator;
					RPNStackItem& result = stack.push();
					result.SetCalculatedVal(nominator, denominator);
					result.Reduce();
					break;
				}
				case EOP_CONCAT:
				{
					if (val1.type != RPNStackItem::SourceValue || val2.type != RPNStackItem::SourceValue)
					{
						stack.push().SetCalculatedVal(0, 0);
					}
					else
					{
						int64_t left = val1.numerator;
						int64_t right = val2.numerator;
						int64_t mult = 10;
						while (right / mult)
						{
							mult *= 10;
						}
						int64_t res = right + (left * mult);
						RPNStackItem& result = stack.push();
						result.SetSourceVal(res);
					}

					break;
				}
				default:
					assert(0);
				};

				break;
			}

			case RPNStackItem::SourceValue:
				stack.push().SetSourceVal(item.numerator);
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

	const int arraySize = 100;

	std::vector<Expression> results(arraySize);


	Expression e;

	for (int64_t i = 0; i < iNumTotalCombinations; ++i)
	{
		int64_t iOpCombinationCode = i % iNumOpCombinations;
		int64_t iOpLocationCode = i / iNumOpCombinations;

		

		e.ResolveOps(iOpCombinationCode);
		e.ResolveOpLocations(iOpLocationCode);
		e.GenerateRPNExpression();
		e.EvaluateRPNExpression();

		

		if ((e.m_result.type == Expression::RPNStackItem::SourceValue || e.m_result.type == Expression::RPNStackItem::CalculatedValue) && e.m_result.denominator == 1)
		{
			if (e.m_result.numerator >= 0 && e.m_result.numerator < arraySize)
			{
				if (results[e.m_result.numerator].m_result.type == Expression::RPNStackItem::None)
				{
					results[e.m_result.numerator] = e;
				}
			}
		}
	}


	
	for (int64_t i = 0; i < arraySize; ++i)
	{
		std::string epxString = results[i].ToRPNString();
		printf("%I64d = %s\n", i, epxString.c_str());
	}
	


	printf("Time: %.2f\n", timer.GetElapsedTime());

	system("pause");
    return 0;
}

