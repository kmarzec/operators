#pragma once


#define ENABLE_INTSAFE_SIGNED_FUNCTIONS
#include <intsafe.h>

//----------------------------------------------------------------------------------------------------------------------
//

inline bool safe_add(int64_t v1, int64_t v2, int64_t& result)
{
	return (LongLongAdd(v1, v2, &result) == S_OK);
}

//----------------------------------------------------------------------------------------------------------------------
//

inline bool safe_sub(int64_t v1, int64_t v2, int64_t& result)
{
	return (LongLongSub(v1, v2, &result) == S_OK);
}

//----------------------------------------------------------------------------------------------------------------------
//

inline bool safe_mul(int64_t v1, int64_t v2, int64_t& result)
{
	return (LongLongMult(v1, v2, &result) == S_OK);
}

//----------------------------------------------------------------------------------------------------------------------
//

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

//----------------------------------------------------------------------------------------------------------------------
//

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

//----------------------------------------------------------------------------------------------------------------------
//
