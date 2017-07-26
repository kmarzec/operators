#pragma once

//----------------------------------------------------------------------------------------------------------------------
//

enum class rpn_operation
{
	Add,
	Sub,
	Mul,
	Div,
	Pow,
	Concat,

	__Count
};

typedef std::underlying_type<rpn_operation>::type rpn_operation_type;

//----------------------------------------------------------------------------------------------------------------------
//

enum class item_type
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

//----------------------------------------------------------------------------------------------------------------------
//

struct rpn_item
{

private:
	item_type type;

public:

	rational number;

	rpn_operation op;

	rpn_item()
		: type(item_type::None)
	{}

	void validate_and_reduce_calculated_value()
	{
		if (number.denominator == 0)
		{
			type = item_type::NANValue;
		}
		else
		{
			type = item_type::CalculatedValue;
			number.reduce();
		}
	}

	void set_operation(rpn_operation o)
	{
		type = item_type::Operand;
		op = o;
	}

	void set_source_value(int64_t val)
	{
		type = item_type::SourceValue;
		number.numerator = val;
		number.denominator = 1;
	}

	void set_calculated_value(int64_t numerator, int64_t denominator)
	{
		number.numerator = numerator;
		number.denominator = denominator;

		if (number.denominator == 0)
		{
			type = item_type::NANValue;
		}
		else
		{
			type = item_type::CalculatedValue;
			number.reduce();
		}
	}

	bool is_number() const
	{
		return type == item_type::CalculatedValue || type == item_type::SourceValue;
	}

	item_type get_type() const
	{
		return type;
	}

	void set_error_value(item_type t)
	{
		type = t;
		assert(is_error_value());
	}

	bool is_error_value() const
	{
		return
			type == item_type::NANValue ||
			type == item_type::Overflow ||
			type == item_type::BadConcat ||
			type == item_type::ExponentNotInteger;
	}
};

//----------------------------------------------------------------------------------------------------------------------
//

template <int STACK_SIZE>
class rpn_stack
{
public:
	inline rpn_stack();

	inline rpn_item& push();

	inline void pop();

	inline const rpn_item& top();

	inline int64_t size();

	inline void clear();

private:

	rpn_item m_stack[STACK_SIZE];
	int64_t m_ptr;
};

//----------------------------------------------------------------------------------------------------------------------
//

template<int EXP_LENGHT, int STACK_SIZE>
class rpn_expression
{
private:
	rpn_item m_expression[EXP_LENGHT];
	rpn_stack<STACK_SIZE> m_stack;

public:
	inline const rpn_item& get_stack_top();

	inline rpn_item& get_item(int64_t idx);

	inline std::string to_string();

	inline void evaluate();
};

//----------------------------------------------------------------------------------------------------------------------
//

#include "rpn.inl"