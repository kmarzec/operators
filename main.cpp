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


#define ENABLE_INTSAFE_SIGNED_FUNCTIONS
#include <intsafe.h>


////////////////////////////

const int64_t NUM_DIGITS = 9;


bool s_bPrintAllCombinations = false;
bool s_bPrintResultArray = true;
bool s_bMultiThreaded = true;
bool s_bPauseAtExit = false;

const int arraySize = 12000;


enum EOps
{
	EOP_PLUS,
	EOP_MINUS,
	EOP_MUL,
	EOP_DIV,
	EOP_POW,
	EOP_CONCAT,

	__EOP_COUNT = 6,
};


char acOpSymbols[] = {'+', '-', '*', '/', '^', '|'};

///////////////////////////////

const int64_t NUM_OP_SPOTS = NUM_DIGITS - 1;
const int64_t NUM_OPS = NUM_DIGITS - 1;




uint64_t gcd(uint64_t u, uint64_t v)
{
	int shift;

	/* GCD(0,v) == v; GCD(u,0) == u, GCD(0,0) == 0 */
	if (u == 0) return v;
	if (v == 0) return u;

	/* Let shift := lg K, where K is the greatest power of 2
	dividing both u and v. */
	for (shift = 0; ((u | v) & 1) == 0; ++shift) {
		u >>= 1;
		v >>= 1;
	}

	while ((u & 1) == 0)
		u >>= 1;

	/* From here on, u is always odd. */
	do {
		/* remove all factors of 2 in v -- they are not common */
		/*   note: v is not zero, so while will terminate */
		while ((v & 1) == 0)  /* Loop X */
			v >>= 1;

		/* Now u and v are both odd. Swap if necessary so u <= v,
		then set v = v - u (which is even). For bignums, the
		swapping is just pointer movement, and the subtraction
		can be done in-place. */
		if (u > v) {
			uint64_t t = v; v = u; u = t;
		}  // Swap u and v.
		v = v - u;                       // Here v >= u.
	} while (v != 0);

	/* restore common factors of 2 */
	return u << shift;
}



struct Expression
{
	struct RPNStackItem
	{
		enum Type
		{
			None,
			SourceValue,
			CalculatedValue,
			Operand,

			NANValue,
			Overflow,
			BadConcat,
			ExponentNotInteger
		};

	private:

		Type type;

	public:

		int64_t numerator;
		int64_t denominator;
		EOps op;

		RPNStackItem()
			: type(None)
			//, op(__EOP_COUNT)
			//, numerator(-1)
			//, denominator(-1)
		{}

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
			numerator = nominator;
			denominator = d;

			if (d == 0)
			{
				type = NANValue;
			}
			else
			{
				type = CalculatedValue;

				if (denominator < 0)
				{
					numerator = -numerator;
					denominator = -denominator;
				}

				if (denominator != 1)
				{
					//if (numerator % denominator == 0)
					//{
					//	numerator = numerator / denominator;
					//	denominator = 1;
					//}

					int64_t div = gcd(numerator < 0 ? -numerator : numerator, denominator);

					assert((numerator % div) == 0);
					assert((denominator % div) == 0);

					numerator /= div;
					denominator /= div;
				}
			}
		}

		bool IsNumber() const
		{
			return type == CalculatedValue || type == SourceValue;
		}

		Type GetType() const
		{
			return type;
		}

		void SetErrorValue(Type t)
		{
			type = t;
			assert(IsErrorValue());
		}

		bool IsErrorValue() const
		{
			return
				type == NANValue ||
				type == Overflow ||
				type == BadConcat ||
				type == ExponentNotInteger;
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

		if (m_result.GetType() != RPNStackItem::None)
		{
			for (const RPNStackItem& item : m_vRPNExpression)
			{
				switch (item.GetType())
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


	HRESULT ipow(int64_t base, int64_t exp, int64_t* result)
	{
		assert(exp >= 0);

		*result = 1;

		while (exp)
		{
			if (exp & 1)
			{
				//*result *= base;

				HRESULT res = LongLongMult(*result, base, result);
				if (res != S_OK)
				{
					return res;
				}
			}

			exp >>= 1;

			//base *= base;
			HRESULT res = LongLongMult(base, base, &base);
			if (res != S_OK)
			{
				return res;
			}
		}

		return S_OK;
	}


	void EvaluateRPNExpression()
	{
		//std::stack<RPNStackItem> stack;
		stack.clear();


		for (const RPNStackItem& item :  m_vRPNExpression)
		{
			switch (item.GetType())
			{
			case RPNStackItem::Operand:
			{
				assert(stack.size() >= 2);
				RPNStackItem val2 = stack.top();
				stack.pop();
				RPNStackItem val1 = stack.top();
				stack.pop();
				assert(val1.IsNumber());
				assert(val2.IsNumber());

				switch (item.op)
				{
				case EOP_PLUS:
				{
					//int64_t nominator = val1.numerator * val2.denominator + val2.numerator * val1.denominator;
					//int64_t denominator = val1.denominator * val2.denominator;
					
					int64_t nominatorPart1;
					int64_t nominatorPart2;
					int64_t nominator;
					int64_t denominator;

					HRESULT res1 = LongLongMult(val1.numerator, val2.denominator, &nominatorPart1);
					HRESULT res2 = LongLongMult(val2.numerator, val1.denominator, &nominatorPart2);
					HRESULT res3 = LongLongAdd(nominatorPart1, nominatorPart2, &nominator);
					HRESULT res4 = LongLongMult(val1.denominator, val2.denominator, &denominator);
					
					if (res1 == S_OK && res2 == S_OK && res3 == S_OK && res4 == S_OK)
					{
						RPNStackItem& result = stack.push();
						result.SetCalculatedVal(nominator, denominator);
					}
					else
					{
						stack.push().SetErrorValue(RPNStackItem::Overflow);
					}
					break;
				}
				case EOP_MINUS:
				{
					//int64_t nominator = val1.numerator * val2.denominator - val2.numerator * val1.denominator;
					//int64_t denominator = val1.denominator * val2.denominator;

					int64_t nominatorPart1;
					int64_t nominatorPart2;
					int64_t nominator;
					int64_t denominator;

					HRESULT res1 = LongLongMult(val1.numerator, val2.denominator, &nominatorPart1);
					HRESULT res2 = LongLongMult(val2.numerator, val1.denominator, &nominatorPart2);
					HRESULT res3 = LongLongSub(nominatorPart1, nominatorPart2, &nominator);
					HRESULT res4 = LongLongMult(val1.denominator, val2.denominator, &denominator);

					if (res1 == S_OK && res2 == S_OK && res3 == S_OK && res4 == S_OK)
					{
						RPNStackItem& result = stack.push();
						result.SetCalculatedVal(nominator, denominator);
					}
					else
					{
						stack.push().SetErrorValue(RPNStackItem::Overflow);
					}
					break;
				}
				case EOP_MUL:
				{
					int64_t nominator; // = val1.numerator * val2.numerator;
					int64_t denominator; // = val1.denominator * val2.denominator;
					
					HRESULT res1 = LongLongMult(val1.numerator, val2.numerator, &nominator);
					HRESULT res2 = LongLongMult(val1.denominator, val2.denominator, &denominator);
					
					if (res1 == S_OK && res2 == S_OK)
					{
						RPNStackItem& result = stack.push();
						result.SetCalculatedVal(nominator, denominator);
					}
					else
					{
						stack.push().SetErrorValue(RPNStackItem::Overflow);
					}
					break;
				}
				case EOP_DIV:
				{
					//int64_t nominator = val1.numerator * val2.denominator;
					//int64_t denominator = val1.denominator * val2.numerator;

					int64_t nominator; // = val1.numerator * val2.numerator;
					int64_t denominator; // = val1.denominator * val2.denominator;

					HRESULT res1 = LongLongMult(val1.numerator, val2.denominator, &nominator);
					HRESULT res2 = LongLongMult(val1.denominator, val2.numerator, &denominator);

					if (res1 == S_OK && res2 == S_OK)
					{
						RPNStackItem& result = stack.push();
						result.SetCalculatedVal(nominator, denominator);
					}
					else
					{
						stack.push().SetErrorValue(RPNStackItem::Overflow);
					}
					break;
				}
				case EOP_CONCAT:
				{
					if (val1.GetType() != RPNStackItem::SourceValue || val2.GetType() != RPNStackItem::SourceValue)
					{
						stack.push().SetErrorValue(RPNStackItem::BadConcat);
					}
					else
					{
						int64_t left = val1.numerator;
						int64_t right = val2.numerator;

						HRESULT res1 = S_OK;
						int64_t mult = 10;
						while (right / mult)
						{
							//mult *= 10;
							res1 = LongLongMult(mult, 10LL, &mult);
						}

						//int64_t res = right + (left * mult);
						int64_t resPart;
						int64_t numerator;
						HRESULT res2 = LongLongMult(left, mult, &resPart);
						HRESULT res3 = LongLongAdd(right, resPart, &numerator);

						if (res1 == S_OK && res2 == S_OK && res3 == S_OK)
						{
							RPNStackItem& result = stack.push();
							result.SetSourceVal(numerator);
						}
						else
						{
							stack.push().SetErrorValue(RPNStackItem::Overflow);
						}
					}
					break;
				}
				case EOP_POW:
				{
					if (val2.denominator == 1)
					{
						if (val2.numerator > 0)
						{
							int64_t nominator;
							HRESULT res1 = ipow(val1.numerator, val2.numerator, &nominator);
							int64_t denominator;
							HRESULT res2 = ipow(val1.denominator, val2.numerator, &denominator);

							if (res1 == S_OK && res2 == S_OK)
							{
								RPNStackItem& result = stack.push();
								result.SetCalculatedVal(nominator, denominator);
							}
							else
							{
								stack.push().SetErrorValue(RPNStackItem::Overflow);
							}
						}
						else
						{
							int64_t nominator;
							HRESULT res1 = ipow(val1.denominator, -val2.numerator, &nominator);
							int64_t denominator;
							HRESULT res2 = ipow(val1.numerator, -val2.numerator, &denominator);

							if (res1 == S_OK && res2 == S_OK)
							{
								RPNStackItem& result = stack.push();
								result.SetCalculatedVal(nominator, denominator);
							}
							else
							{
								stack.push().SetErrorValue(RPNStackItem::Overflow);
							}
						}
					}
					else
					{
						stack.push().SetErrorValue(RPNStackItem::ExponentNotInteger);
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
			if (!stack.top().IsNumber())
			{
				break;
			}
		}

		assert(stack.size() > 0);
		assert((stack.size() == 1 && stack.top().IsNumber()) || stack.top().IsErrorValue());
		
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



class SpinLock 
{
	std::atomic_flag locked = ATOMIC_FLAG_INIT;

public:
	void lock() 
	{
		while (locked.test_and_set(std::memory_order_acquire)) { ; }
	}

	void unlock() 
	{
		locked.clear(std::memory_order_release);
	}
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

	float fEstimatedMCPS = 16.1f;
	float fEstimatedTime = iNumTotalCombinations / (fEstimatedMCPS * 1000000.0f);
	printf("Estimated Time: %.2f\n", fEstimatedTime);


	std::vector<std::pair<Expression, SpinLock>> results(arraySize);


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
		Expression e;

		for (int64_t i = start; i < end; ++i)
		{
			int64_t iOpCombinationCode = i % iNumOpCombinations;
			int64_t iOpLocationCode = i / iNumOpCombinations;

			e.ResolveOps(iOpCombinationCode);
			e.ResolveOpLocations(iOpLocationCode);
			e.GenerateRPNExpression();
			e.EvaluateRPNExpression();

			if(s_bPrintAllCombinations)
			{
				std::string epxString = e.ToRPNString();
				printf(epxString.c_str());
				if ((e.m_result.IsNumber()))
				{
					if (e.m_result.denominator == 1)
					{
						printf(" = %I64d\n", e.m_result.numerator);
					}
					else
					{
						printf(" = %I64d/%I64d\n", e.m_result.numerator, e.m_result.denominator);
					}
				}
				else
				{
					printf(" = \n");
				}
			}

			if (1)
			{
				iTotal++;

				if (e.m_result.IsNumber())
				{
					assert(e.m_result.denominator > 0);

					if (e.m_result.numerator >= 0)
					{
						if (e.m_result.denominator == 1)
						{
							iIntsPositive++;
						}
						else
						{
							iFractionsPositive++;
						}
					}
					else
					{
						if (e.m_result.denominator == 1)
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
					assert(e.m_result.IsErrorValue());

					switch (e.m_result.GetType())
					{
					case Expression::RPNStackItem::NANValue:
						iErrorsNANValue++;
						break;
					case Expression::RPNStackItem::Overflow:
						iErrorsOverflow++;
						break;
					case Expression::RPNStackItem::BadConcat:
						iErrorsBadConcat++;
						break;
					case Expression::RPNStackItem::ExponentNotInteger:
						iErrorsExponentNotInteger++;
						break;
					default:
						assert(0);
					}
				}
			}


			if (s_bPrintResultArray)
			{
				if ((e.m_result.IsNumber()) && e.m_result.denominator == 1)
				{
					if (e.m_result.numerator >= 0 && e.m_result.numerator < arraySize)
					{
						results[e.m_result.numerator].second.lock();

						if (results[e.m_result.numerator].first.m_result.GetType() == Expression::RPNStackItem::None)
						{
							results[e.m_result.numerator].first = e;
						}

						results[e.m_result.numerator].second.unlock();
					}
				}
			}


		}
	};

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

	
	if (s_bPrintResultArray)
	{
		for (int64_t i = 0; i < arraySize; ++i)
		{
			std::string epxString = results[i].first.ToRPNString();
			printf("%I64d = %s\n", i, epxString.c_str());
		}
	}

	if (1)
	{
		printf("iIntsPositive = %I64d (%.2f)\n", (int64_t)iIntsPositive, (float)iIntsPositive / (float)iTotal);
		printf("iIntsNegative = %I64d (%.2f)\n", (int64_t)iIntsNegative, (float)iIntsNegative / (float)iTotal);
		printf("iFractionsPositive = %I64d (%.2f)\n", (int64_t)iFractionsPositive, (float)iFractionsPositive / (float)iTotal);
		printf("iFractionsNegative = %I64d (%.2f)\n", (int64_t)iFractionsNegative, (float)iFractionsNegative / (float)iTotal);
		printf("iErrorsNANValue = %I64d (%.2f)\n", (int64_t)iErrorsNANValue, (float)iErrorsNANValue / (float)iTotal);
		printf("iErrorsOverflow = %I64d (%.2f)\n", (int64_t)iErrorsOverflow, (float)iErrorsOverflow / (float)iTotal);
		printf("iErrorsBadConcat = %I64d (%.2f)\n", (int64_t)iErrorsBadConcat, (float)iErrorsBadConcat / (float)iTotal);
		printf("iErrorsExponentNotInteger = %I64d (%.2f)\n", (int64_t)iErrorsExponentNotInteger, (float)iErrorsExponentNotInteger / (float)iTotal);

	}


	float time = timer.GetElapsedTime();
	printf("Time: %.2f\n", time);
	float fMCPS = (float)iNumTotalCombinations / (time*1000000.0f);
	printf("Millions of combinations/s: %.2f\n", fMCPS);

	if (s_bPauseAtExit)
	{
		system("pause");
	}
    return 0;
}

