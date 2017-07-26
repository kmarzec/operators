#pragma once

#include "rationalnumber.h"
#include "rpn.h"

//----------------------------------------------------------------------------------------------------------------------
//

struct expression
{
	rpn_operation aiOps[NUM_OPS];
	int64_t aiOpLocations[NUM_OP_SPOTS];
	rpn_expression<NUM_DIGITS + NUM_OPS, NUM_DIGITS> m_rpnExpression;


	inline void resolve_operations(int64_t iOpCombinationCode);

	inline void resolve_operation_locations(int64_t iOpLocationCode);

	inline void generate_rpn_expression();

	inline void evaluate();

	inline std::string to_string();

	inline const rpn_item& get_result();
};

//----------------------------------------------------------------------------------------------------------------------
//

#include "expression.inl"
