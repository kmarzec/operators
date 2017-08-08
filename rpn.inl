#pragma once

#include <map>

//----------------------------------------------------------------------------------------------------------------------
//

template <int STACK_SIZE>
inline rpn_stack<STACK_SIZE>::rpn_stack()
	: m_ptr(-1)
{}

//----------------------------------------------------------------------------------------------------------------------
//

template <int STACK_SIZE>
inline rpn_item& rpn_stack<STACK_SIZE>::push()
{
	assert(m_ptr < STACK_SIZE - 1);
	return m_stack[++m_ptr];
}

//----------------------------------------------------------------------------------------------------------------------
//

template <int STACK_SIZE>
inline void rpn_stack<STACK_SIZE>::pop()
{
	assert(m_ptr >= 0);
	m_ptr--;
}

//----------------------------------------------------------------------------------------------------------------------
//

template <int STACK_SIZE>
inline const rpn_item& rpn_stack<STACK_SIZE>::top()
{
	assert(m_ptr >= 0);
	return m_stack[m_ptr];
}

//----------------------------------------------------------------------------------------------------------------------
//

template <int STACK_SIZE>
inline int64_t rpn_stack<STACK_SIZE>::size()
{
	return m_ptr + 1;
}

//----------------------------------------------------------------------------------------------------------------------
//

template <int STACK_SIZE>
inline void rpn_stack<STACK_SIZE>::clear()
{
	m_ptr = -1;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//

template<int EXP_LENGHT, int STACK_SIZE>
inline const rpn_item& rpn_expression<EXP_LENGHT, STACK_SIZE>::get_stack_top()
{
	assert(m_stack.size() > 0);
	return m_stack.top();
}

//----------------------------------------------------------------------------------------------------------------------
//

template<int EXP_LENGHT, int STACK_SIZE>
inline rpn_item& rpn_expression<EXP_LENGHT, STACK_SIZE>::get_item(int64_t idx)
{
	assert(idx >= 0 && idx < EXP_LENGHT);
	return m_expression[idx];
}

//----------------------------------------------------------------------------------------------------------------------
//

template<int EXP_LENGHT, int STACK_SIZE>
inline std::string rpn_expression<EXP_LENGHT, STACK_SIZE>::to_string()
{
	std::string resultStr;

	static const std::map<rpn_operation, char> operationToChar = 
	{
		{ rpn_operation::Add, '+'},
		{ rpn_operation::Sub, '-' },
		{ rpn_operation::Mul, '*' },
		{ rpn_operation::Div, '/' },
		{ rpn_operation::Pow, '^' },
		{ rpn_operation::Concat, '|' }
	};

	if (m_stack.top().get_type() != item_type::None)
	{
		for (const rpn_item& item : m_expression)
		{
			switch (item.get_type())
			{
			case item_type::Operand:
            {
                auto it = operationToChar.find(item.op);
                if (it != operationToChar.end())
                {
                    resultStr += it->second;
                }
                else
                {
                    resultStr += '?';
                }
                break;
            }
			case item_type::SourceValue:
				resultStr += std::to_string(item.number.numerator);
				break;

			default:
				assert(0);
			};
		}
	}

	return resultStr;
}

//----------------------------------------------------------------------------------------------------------------------
//

template<int EXP_LENGHT, int STACK_SIZE>
inline void rpn_expression<EXP_LENGHT, STACK_SIZE>::evaluate()
{
	m_stack.clear();

	for (const rpn_item& item : m_expression)
	{
		switch (item.get_type())
		{
		case item_type::Operand:
		{
			assert(m_stack.size() >= 2);
			rpn_item val2 = m_stack.top();
			m_stack.pop();
			rpn_item val1 = m_stack.top();
			m_stack.pop();
			assert(val1.is_number());
			assert(val2.is_number());

			switch (item.op)
			{
            case rpn_operation::Add:
			{
				rpn_item& result = m_stack.push();
				if (safe_add_rational_number(val1.number, val2.number, result.number))
				{
					result.validate_and_reduce_calculated_value();
				}
				else
				{
					result.set_error_value(item_type::Overflow);
				}
				break;
			}
			case rpn_operation::Sub:
			{
				rpn_item& result = m_stack.push();
				if (safe_sub_rational_number(val1.number, val2.number, result.number))
				{
					result.validate_and_reduce_calculated_value();
				}
				else
				{
					result.set_error_value(item_type::Overflow);
				}
				break;
			}
			case rpn_operation::Mul:
			{
				rpn_item& result = m_stack.push();
				if (safe_mul_rational_number(val1.number, val2.number, result.number))
				{
					result.validate_and_reduce_calculated_value();
				}
				else
				{
					result.set_error_value(item_type::Overflow);
				}
				break;
			}
			case rpn_operation::Div:
			{
				rpn_item& result = m_stack.push();
				if (safe_div_rational_number(val1.number, val2.number, result.number))
				{
					result.validate_and_reduce_calculated_value();
				}
				else
				{
					result.set_error_value(item_type::Overflow);
				}
				break;
			}
			case rpn_operation::Pow:
			{
				if (val2.number.denominator == 1)
				{
					rpn_item& result = m_stack.push();
					if (safe_pow_rational_number(val1.number, val2.number, result.number))
					{
						result.validate_and_reduce_calculated_value();
					}
					else
					{
						result.set_error_value(item_type::Overflow);
					}
				}
				else
				{
					m_stack.push().set_error_value(item_type::ExponentNotInteger);
				}

				break;
			}
			case rpn_operation::Concat:
			{
				if (val1.get_type() != item_type::SourceValue || val2.get_type() != item_type::SourceValue)
				{
					m_stack.push().set_error_value(item_type::BadConcat);
				}
				else
				{
					int64_t left = val1.number.numerator;
					int64_t right = val2.number.numerator;

					bool multOK = true;
					int64_t mult = 10LL;
					while ((right / mult) && multOK)
					{
						multOK = safe_mul(mult, 10LL, mult);
					}

					int64_t resPart;
					int64_t numerator;
					if (safe_mul(left, mult, resPart) &&
						safe_add(right, resPart, numerator))
					{
						m_stack.push().set_source_value(numerator);
					}
					else
					{
						m_stack.push().set_error_value(item_type::Overflow);
					}

				}
				break;
			}

			default:
				assert(0);
			};

			break;
		}

		case item_type::SourceValue:
			m_stack.push().set_source_value(item.number.numerator);
			break;

		default:
			assert(0);
		};

		assert(m_stack.size() > 0);
		if (!m_stack.top().is_number())
		{
			break;
		}
	}

	assert(m_stack.size() > 0);
	assert((m_stack.size() == 1 && m_stack.top().is_number()) || m_stack.top().is_error_value());
}

//----------------------------------------------------------------------------------------------------------------------
//