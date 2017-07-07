#pragma once


#define ENABLE_INTSAFE_SIGNED_FUNCTIONS
#include <intsafe.h>


#define USE_SAFE_INT 1

class safe_int64_t
{
public:
	safe_int64_t()
		: m_val(0)
		, m_overflow(false)
	{}

	safe_int64_t(const int64_t& val)
		: m_val(val)
		, m_overflow(false)
	{}

	inline bool IsValid()
	{
		return !m_overflow;
	}

	inline operator int64_t()
	{
		return m_val;
	}

	inline safe_int64_t operator + (const safe_int64_t& rhs)
	{
		safe_int64_t sum;

		if (m_overflow || rhs.m_overflow)
		{
			sum.m_overflow = true;
		}
		else
		{
			sum.m_overflow = (LongLongAdd(m_val, rhs.m_val, &sum.m_val) != S_OK);
		}

		return sum;
	}

	inline safe_int64_t operator - (const safe_int64_t& rhs)
	{
		safe_int64_t sum;

		if (m_overflow || rhs.m_overflow)
		{
			sum.m_overflow = true;
		}
		else
		{
			sum.m_overflow = (LongLongSub(m_val, rhs.m_val, &sum.m_val) != S_OK);
		}

		return sum;
	}

	inline safe_int64_t operator * (const safe_int64_t& rhs)
	{
		safe_int64_t sum;

		if (m_overflow || rhs.m_overflow)
		{
			sum.m_overflow = true;
		}
		else
		{
			sum.m_overflow = (LongLongMult(m_val, rhs.m_val, &sum.m_val) != S_OK);
		}

		return sum;
	}

private:
	int64_t m_val;
	bool m_overflow;
};



inline bool safe_add(int64_t v1, int64_t v2, int64_t& result)
{
	return (LongLongAdd(v1, v2, &result) == S_OK);
}

inline bool safe_sub(int64_t v1, int64_t v2, int64_t& result)
{
	return (LongLongSub(v1, v2, &result) == S_OK);
}

inline bool safe_mul(int64_t v1, int64_t v2, int64_t& result)
{
	return (LongLongMult(v1, v2, &result) == S_OK);
}


inline uint64_t gcd(uint64_t u, uint64_t v)
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

#if USE_SAFE_INT

inline bool safe_ipow(int64_t base, int64_t exp, int64_t& result)
{
	assert(exp >= 0);

	result = 1LL;

	while (exp)
	{
		if (exp & 1LL)
		{
			if(!safe_mul(result, base, result))
			{
				return false;
			}
		}

		exp >>= 1;

		if(!safe_mul(base, base, base))
		{
			return false;
		}
	}

	return true;
}

#else

inline HRESULT ipow(int64_t base, int64_t exp, int64_t* result)
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

#endif