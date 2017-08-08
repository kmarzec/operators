#pragma once

#include "rationalnumber.h"
#include "rpn.h"

//----------------------------------------------------------------------------------------------------------------------
//

template <int NUMBER_OF_DIGITS>
class expression
{
private:    
    static const int64_t m_number_of_operator_locatations = NUMBER_OF_DIGITS - 1;
    static const int64_t m_number_of_operators = NUMBER_OF_DIGITS - 1;
    
    static int64_t iNumOpLocationCombinations;
    static int64_t iNumOpCombinations;
    static int64_t iNumTotalCombinations;

	rpn_operation aiOps[m_number_of_operators];
	int64_t aiOpLocations[m_number_of_operator_locatations];
	rpn_expression<NUMBER_OF_DIGITS + m_number_of_operators, NUMBER_OF_DIGITS> m_rpnExpression;

    inline void resolve_operations(int64_t iOpCombinationCode);

    inline void resolve_operation_locations(int64_t iOpLocationCode);

    inline void generate_rpn_expression();

public:
    inline static void init();

 
    inline void resolve_expression(int64_t combination_code);


	inline void evaluate();

	inline std::string to_string();

	inline const rpn_item& get_result();

    static inline int64_t get_num_op_locations() { return iNumOpCombinations; }

    static inline int64_t get_num_op_combinations() { return iNumOpCombinations; }

    static inline int64_t get_total_number_of_combinations() { return iNumTotalCombinations; }

    static inline int64_t get_num_digits() { return NUMBER_OF_DIGITS; }

    static inline int64_t get_num_operators() { return m_number_of_operators; }

    static inline int64_t get_num_operator_locations() { return m_number_of_operator_locatations; }

};

//----------------------------------------------------------------------------------------------------------------------
//

#include "expression.inl"
