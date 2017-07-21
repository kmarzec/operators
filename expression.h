#pragma once


#include "rationalnumber.h"
#include "rpn.h"








struct Expression
{
	rpn_operation aiOps[NUM_OPS];
	int64_t aiOpLocations[NUM_OP_SPOTS];

	rpn_expression<NUM_DIGITS + NUM_OPS, NUM_DIGITS> m_rpnExpression;


	void ResolveOps(int64_t iOpCombinationCode)
	{
		for (int64_t n = 0; n < NUM_OPS; ++n)
		{
			aiOps[n] = (rpn_operation)(iOpCombinationCode % __EOP_COUNT);
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
				m_rpnExpression.get_item(iCharPtr).SetOp(aiOps[iOpIdx++]);
				aiOpLocations[iNumDigits]--;
			}
			else
			{
				m_rpnExpression.get_item(iCharPtr).SetSourceVal(NUM_DIGITS - iNumDigits);
				iNumDigits++;
			}

			iCharPtr--;
		}
	}


	inline void evaluate()
	{
		m_rpnExpression.evaluate();
	}

	inline std::string to_string()
	{
		return m_rpnExpression.to_string();
	}

	inline const rpn_item& get_result()
	{
		return m_rpnExpression.get_stack_top();
	}


};

#include "expression.inl"
