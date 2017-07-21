#pragma once



enum rpn_operation
{
	EOP_PLUS,
	EOP_MINUS,
	EOP_MUL,
	EOP_DIV,
	EOP_POW,
	EOP_CONCAT,

	__EOP_COUNT = 6,
};


struct rpn_item
{
	enum item_type
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
	item_type type;

public:

	RationalNumber number;

	rpn_operation op;

	rpn_item()
		: type(None)
	{}

	void validate_and_reduce_calculated_value()
	{
		if (number.denominator == 0)
		{
			type = NANValue;
		}
		else
		{
			type = CalculatedValue;
			number.Reduce();
		}
	}

	void SetOp(rpn_operation o)
	{
		type = Operand;
		op = o;
	}

	void SetSourceVal(int64_t val)
	{
		type = SourceValue;
		number.numerator = val;
		number.denominator = 1;
	}

	void SetCalculatedVal(int64_t numerator, int64_t denominator)
	{
		number.numerator = numerator;
		number.denominator = denominator;

		if (number.denominator == 0)
		{
			type = NANValue;
		}
		else
		{
			type = CalculatedValue;
			number.Reduce();
		}
	}

	bool IsNumber() const
	{
		return type == CalculatedValue || type == SourceValue;
	}

	item_type GetType() const
	{
		return type;
	}

	void SetErrorValue(item_type t)
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


template <int STACK_SIZE>
class rpn_stack
{
public:

	rpn_stack()
		: ptr(-1)
	{}

	rpn_item& push()
	{
		assert(ptr < STACK_SIZE - 1);
		return m_stack[++ptr];
	}

	void pop()
	{
		assert(ptr >= 0);
		ptr--;
	}

	const rpn_item& top()
	{
		return m_stack[ptr];
	}

	int64_t size()
	{
		return ptr + 1;
	}

	void clear()
	{
		ptr = -1;
	}

private:

	rpn_item m_stack[STACK_SIZE];
	int64_t ptr;
};


template<int EXP_LENGHT, int STACK_SIZE>
class rpn_expression
{
private:
	rpn_item m_expression[EXP_LENGHT];
	rpn_stack<STACK_SIZE> m_stack;

public:

	inline const rpn_item& get_stack_top()
	{
		assert(m_stack.size() > 0);
		return m_stack.top();
	}


	inline rpn_item& get_item(int64_t idx)
	{
		assert(idx >= 0 && idx < EXP_LENGHT);
		return m_expression[idx];
	}



	inline std::string to_string()
	{
		std::string resultStr;

		char acOpSymbols[] = { '+', '-', '*', '/', '^', '|' };

		if (m_stack.top().GetType() != rpn_item::None)
		{
			for (const rpn_item& item : m_expression)
			{
				switch (item.GetType())
				{
				case rpn_item::Operand:
					resultStr += acOpSymbols[item.op];
					break;
				case rpn_item::SourceValue:
					resultStr += std::to_string(item.number.numerator);
					break;

				default:
					assert(0);
				};
			}
		}

		return resultStr;
	}


	inline void evaluate()
	{
		//std::stack<rpn_item> stack;
		m_stack.clear();


		for (const rpn_item& item : m_expression)
		{
			switch (item.GetType())
			{
			case rpn_item::Operand:
			{
				assert(stack.size() >= 2);
				rpn_item val2 = m_stack.top();
				m_stack.pop();
				rpn_item val1 = m_stack.top();
				m_stack.pop();
				assert(val1.IsNumber());
				assert(val2.IsNumber());

				switch (item.op)
				{
				case EOP_PLUS:
				{
					rpn_item& result = m_stack.push();
					if (safe_add_rational_number(val1.number, val2.number, result.number))
					{
						result.validate_and_reduce_calculated_value();
					}
					else
					{
						result.SetErrorValue(rpn_item::Overflow);
					}
					break;
				}
				case EOP_MINUS:
				{
					rpn_item& result = m_stack.push();
					if (safe_sub_rational_number(val1.number, val2.number, result.number))
					{
						result.validate_and_reduce_calculated_value();
					}
					else
					{
						result.SetErrorValue(rpn_item::Overflow);
					}
					break;
				}
				case EOP_MUL:
				{
					rpn_item& result = m_stack.push();
					if (safe_mul_rational_number(val1.number, val2.number, result.number))
					{
						result.validate_and_reduce_calculated_value();
					}
					else
					{
						result.SetErrorValue(rpn_item::Overflow);
					}
					break;
				}
				case EOP_DIV:
				{
					rpn_item& result = m_stack.push();
					if (safe_div_rational_number(val1.number, val2.number, result.number))
					{
						result.validate_and_reduce_calculated_value();
					}
					else
					{
						result.SetErrorValue(rpn_item::Overflow);
					}
					break;
				}
				case EOP_POW:
				{
					if (val2.number.denominator == 1)
					{
						if (val2.number.numerator > 0)
						{
							int64_t nominator;
							int64_t denominator;

							if (safe_ipow(val1.number.numerator, val2.number.numerator, nominator) &&
								safe_ipow(val1.number.denominator, val2.number.numerator, denominator))
							{
								m_stack.push().SetCalculatedVal(nominator, denominator);
							}
							else
							{
								m_stack.push().SetErrorValue(rpn_item::Overflow);
							}
						}
						else
						{
							int64_t nominator;
							int64_t denominator;

							if (safe_ipow(val1.number.denominator, -val2.number.numerator, nominator) &&
								safe_ipow(val1.number.numerator, -val2.number.numerator, denominator))
							{
								m_stack.push().SetCalculatedVal(nominator, denominator);
							}
							else
							{
								m_stack.push().SetErrorValue(rpn_item::Overflow);
							}
						}
					}
					else
					{
						m_stack.push().SetErrorValue(rpn_item::ExponentNotInteger);
					}

					break;
				}
				case EOP_CONCAT:
				{
					if (val1.GetType() != rpn_item::SourceValue || val2.GetType() != rpn_item::SourceValue)
					{
						m_stack.push().SetErrorValue(rpn_item::BadConcat);
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
							m_stack.push().SetSourceVal(numerator);
						}
						else
						{
							m_stack.push().SetErrorValue(rpn_item::Overflow);
						}

					}
					break;
				}			

				default:
					assert(0);
				};

				break;
			}

			case rpn_item::SourceValue:
				m_stack.push().SetSourceVal(item.number.numerator);
				break;

			default:
				assert(0);
			};

			assert(stack.size() > 0);
			if (!m_stack.top().IsNumber())
			{
				break;
			}
		}

		assert(m_stack.size() > 0);
		assert((m_stack.size() == 1 && stack.top().IsNumber()) || m_stack.top().IsErrorValue());
	}
};