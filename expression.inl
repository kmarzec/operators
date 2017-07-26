#pragma once

#include "expression.h"

//----------------------------------------------------------------------------------------------------------------------
//

inline void expression::resolve_operations(int64_t iOpCombinationCode)
{
	for (int64_t n = 0; n < NUM_OPS; ++n)
	{
		aiOps[n] = (rpn_operation)(iOpCombinationCode % static_cast<rpn_operation_type>(rpn_operation::__Count));
		iOpCombinationCode /= static_cast<rpn_operation_type>(rpn_operation::__Count);
	}
}

//----------------------------------------------------------------------------------------------------------------------
//

inline void expression::resolve_operation_locations(int64_t iOpLocationCode)
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

//----------------------------------------------------------------------------------------------------------------------
//

inline void expression::generate_rpn_expression()
{
	int64_t iCharPtr = NUM_DIGITS + NUM_OPS - 1;
	int64_t iNumDigits = 0;
	int64_t iOpIdx = 0;

	while (iCharPtr >= 0)
	{
		if (iNumDigits < NUM_OP_SPOTS && aiOpLocations[iNumDigits] > 0)
		{
			m_rpnExpression.get_item(iCharPtr).set_operation(aiOps[iOpIdx++]);
			aiOpLocations[iNumDigits]--;
		}
		else
		{
			m_rpnExpression.get_item(iCharPtr).set_source_value(NUM_DIGITS - iNumDigits);
			iNumDigits++;
		}

		iCharPtr--;
	}
}

//----------------------------------------------------------------------------------------------------------------------
//

inline void expression::evaluate()
{
	m_rpnExpression.evaluate();
}

//----------------------------------------------------------------------------------------------------------------------
//

inline std::string expression::to_string()
{
	return m_rpnExpression.to_string();
}

//----------------------------------------------------------------------------------------------------------------------
//

inline const rpn_item& expression::get_result()
{
	return m_rpnExpression.get_stack_top();
}

//----------------------------------------------------------------------------------------------------------------------
//
