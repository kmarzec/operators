#pragma once


struct RationalNumber
{
	int64_t numerator;
	int64_t denominator;

	void Reduce()
	{
		if (denominator < 0)
		{
			numerator = -numerator;
			denominator = -denominator;
		}

		if (denominator != 1)
		{
			assert(denominator > 0);
			int64_t div = gcd(numerator < 0 ? -numerator : numerator, denominator);
			assert((numerator % div) == 0);
			assert((denominator % div) == 0);

			numerator /= div;
			denominator /= div;
		}
	}
};


inline bool safe_add_rational_number(const RationalNumber& val1, const RationalNumber& val2, RationalNumber& result)
{
	int64_t nominatorPart1;
	int64_t nominatorPart2;

	if (safe_mul(val1.numerator, val2.denominator, nominatorPart1) &&
		safe_mul(val2.numerator, val1.denominator, nominatorPart2) &&
		safe_add(nominatorPart1, nominatorPart2, result.numerator) &&
		safe_mul(val1.denominator, val2.denominator, result.denominator))
	{
		return true;
	}
	
	return false;
}

inline bool safe_sub_rational_number(const RationalNumber& val1, const RationalNumber& val2, RationalNumber& result)
{
	int64_t nominatorPart1;
	int64_t nominatorPart2;

	if (safe_mul(val1.numerator, val2.denominator, nominatorPart1) &&
		safe_mul(val2.numerator, val1.denominator, nominatorPart2) &&
		safe_sub(nominatorPart1, nominatorPart2, result.numerator) &&
		safe_mul(val1.denominator, val2.denominator, result.denominator))
	{
		return true;
	}

	return false;
}

inline bool safe_mul_rational_number(const RationalNumber& val1, const RationalNumber& val2, RationalNumber& result)
{
	if (safe_mul(val1.numerator, val2.numerator, result.numerator) &&
		safe_mul(val1.denominator, val2.denominator, result.denominator))
	{
		return true;
	}

	return false;
}