#pragma once

#include "expression.h"

//template <int NUMBER_OF_DIGITS> int64_t expression<NUMBER_OF_DIGITS>::m_number_of_operator_locatations = -1;
//template <int NUMBER_OF_DIGITS> int64_t expression<NUMBER_OF_DIGITS>::m_number_of_operators = -1;
template <int NUMBER_OF_DIGITS> int64_t expression<NUMBER_OF_DIGITS>::iNumOpLocationCombinations = -1;
template <int NUMBER_OF_DIGITS> int64_t expression<NUMBER_OF_DIGITS>::iNumOpCombinations = -1;
template <int NUMBER_OF_DIGITS> int64_t expression<NUMBER_OF_DIGITS>::iNumTotalCombinations = -1;

//----------------------------------------------------------------------------------------------------------------------
//

template <int NUMBER_OF_DIGITS>
inline void expression<NUMBER_OF_DIGITS>::resolve_expression(int64_t combination_code)
{
    int64_t iOpCombinationCode = combination_code % iNumOpCombinations;
    int64_t iOpLocationCode = combination_code / iNumOpCombinations;

    resolve_operations(iOpCombinationCode);
    resolve_operation_locations(iOpLocationCode);
    generate_rpn_expression();
}

//----------------------------------------------------------------------------------------------------------------------
//

template <int NUMBER_OF_DIGITS>
inline void expression<NUMBER_OF_DIGITS>::init()
{
    iNumOpLocationCombinations = 1;
    for (int64_t j = 1; j <= m_number_of_operator_locatations; ++j)
    {
        iNumOpLocationCombinations *= j;
    }


    iNumOpCombinations = 1;
    for (int64_t j = 1; j <= m_number_of_operators; ++j)
    {
        iNumOpCombinations *= static_cast<rpn_operation_type>(rpn_operation::__Count);
    }

    iNumTotalCombinations = iNumOpLocationCombinations * iNumOpCombinations;
}


//----------------------------------------------------------------------------------------------------------------------
//

template <int NUMBER_OF_DIGITS>
inline void expression<NUMBER_OF_DIGITS>::resolve_operations(int64_t iOpCombinationCode)
{
	for (int64_t n = 0; n < m_number_of_operators; ++n)
	{
		aiOps[n] = (rpn_operation)(iOpCombinationCode % static_cast<rpn_operation_type>(rpn_operation::__Count));
		iOpCombinationCode /= static_cast<rpn_operation_type>(rpn_operation::__Count);
	}
}

//----------------------------------------------------------------------------------------------------------------------
//

template <int NUMBER_OF_DIGITS>
inline void expression<NUMBER_OF_DIGITS>::resolve_operation_locations(int64_t iOpLocationCode)
{
	for (int64_t n = 0; n < m_number_of_operator_locatations; ++n)
	{
		aiOpLocations[n] = 0;
	}

	aiOpLocations[0]++;
	for (int64_t n = 2; n <= m_number_of_operators; ++n)
	{
		int64_t iOpLocation = iOpLocationCode%n;
		aiOpLocations[iOpLocation]++;
		iOpLocationCode /= n;
	}
}

//----------------------------------------------------------------------------------------------------------------------
//

template <int NUMBER_OF_DIGITS>
inline void expression<NUMBER_OF_DIGITS>::generate_rpn_expression()
{
	int64_t iCharPtr = NUMBER_OF_DIGITS + m_number_of_operators - 1;
	int64_t iNumDigits = 0;
	int64_t iOpIdx = 0;

	while (iCharPtr >= 0)
	{
		if (iNumDigits < m_number_of_operator_locatations && aiOpLocations[iNumDigits] > 0)
		{
			m_rpnExpression.get_item(iCharPtr).set_operation(aiOps[iOpIdx++]);
			aiOpLocations[iNumDigits]--;
		}
		else
		{
			m_rpnExpression.get_item(iCharPtr).set_source_value(NUMBER_OF_DIGITS - iNumDigits);
			iNumDigits++;
		}

		iCharPtr--;
	}
}

//----------------------------------------------------------------------------------------------------------------------
//

template <int NUMBER_OF_DIGITS>
inline void expression<NUMBER_OF_DIGITS>::evaluate()
{
	m_rpnExpression.evaluate();
}

//----------------------------------------------------------------------------------------------------------------------
//

template <int NUMBER_OF_DIGITS>
inline std::string expression<NUMBER_OF_DIGITS>::to_string()
{
	return m_rpnExpression.to_string();
}

//----------------------------------------------------------------------------------------------------------------------
//

template <int NUMBER_OF_DIGITS>
inline const rpn_item& expression<NUMBER_OF_DIGITS>::get_result()
{
	return m_rpnExpression.get_stack_top();
}

//----------------------------------------------------------------------------------------------------------------------
//
