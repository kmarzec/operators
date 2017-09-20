
//#pragma OPENCL EXTENSION cl_intel_printf : enable

#define NUMBER_OF_DIGITS 9
#define NUMBER_OF_OPS 8

#define m_number_of_operator_locatations (NUMBER_OF_DIGITS - 1)
#define m_number_of_operators (NUMBER_OF_DIGITS - 1)




#define iNumOpLocationCombinations 40320
#define iNumOpCombinations 1679616
#define iNumTotalCombinations 67722117120


#define rpn_operation_Add 0
#define rpn_operation_Sub 1
#define rpn_operation_Mul 2
#define rpn_operation_Div 3
#define rpn_operation_Pow 4
#define rpn_operation_Concat 5
#define rpn_operation__Count 6


#define STACK_SIZE NUMBER_OF_DIGITS


bool safe_mul(long a, long b, long* res)
{
    *res = a * b;

    if (a != 0)
    {
        long x = (*res) / a;

        printf("\ta: %ld, b: %ld, res: %ld, x: %ld\n", a, b, *res, x);

        if (x != b)
        {
            return false;
        }
    }

    return true;
}


void ipow(long base, long exponent, long* result)
{
   // printf("ipow base: %ld, exp: %ld\n", base, exponent);

    long _base = base;
    long _exponent = exponent;

    *result = 1L;
    //return;

    while (exponent)
    {
        if (exponent & 1L)
        {
            //long prevResult = (*result);

            //(*result) *= base;

            if(!safe_mul(*result, base, result))
            //if (*result < 0 ||  *result / base != prevResult)
            {
                printf("ipow OVERFLOW1 base: %ld, exp: %ld, ipow res: %ld\n", _base, _exponent, *result);
                (*result) = 1L;
                return;
            }
        }

        exponent >>= 1;

        //long prevBase = base;

        //base *= base;

        //if (base < 0 || base / prevBase != prevBase)
        if (!safe_mul(base, base, &base))
        {
            printf("ipow OVERFLOW2 base: %ld, exp: %ld, ipow res: %ld\n", _base, _exponent, *result);
            (*result) = 1L;

            return;
        }
    }

    printf("ipow base: %ld, exp: %ld, ipow res: %ld\n", _base, _exponent, *result);
}

//----------------------------------------------------------------------------------------------------------------------
//


long gcd(long u, long v)
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
            long t = v; v = u; u = t;
        }  // Swap u and v.
        v = v - u;                       // Here v >= u.
    } while (v != 0);

    /* restore common factors of 2 */
    return u << shift;
}


///----------------------------------------

struct rational
{
    long numerator;
    long denominator;
};

void rational_reduce(struct rational* r)
{
    if (r->denominator < 0)
    {
        r->numerator = -r->numerator;
        r->denominator = -r->denominator;
    }

    if (r->denominator != 1)
    {
        long div = gcd(r->numerator < 0 ? -r->numerator : r->numerator, r->denominator);

        r->numerator /= div;
        r->denominator /= div;
    }
}

///--------------------------------------
///


void add_rational_number(struct rational* val1, struct rational* val2, struct rational* result)
{
    long nominatorPart1;
    long nominatorPart2;

    nominatorPart1 = val1->numerator * val2->denominator;
    nominatorPart2 = val2->numerator * val1->denominator;
    result->numerator = nominatorPart1 + nominatorPart2;
    result->denominator = val1->denominator * val2->denominator;
}

//----------------------------------------------------------------------------------------------------------------------
//

void sub_rational_number(struct rational* val1, struct rational* val2, struct rational* result)
{
    long nominatorPart1;
    long nominatorPart2;

    nominatorPart1 = val1->numerator * val2->denominator;
    nominatorPart2 = val2->numerator * val1->denominator;
    result->numerator = nominatorPart1 - nominatorPart2;
    result->denominator = val1->denominator * val2->denominator;
}

//----------------------------------------------------------------------------------------------------------------------
//

void mul_rational_number(struct rational* val1, struct rational* val2, struct rational* result)
{
    result->numerator = val1->numerator * val2->numerator;
    result->denominator = val1->denominator * val2->denominator;
}

//----------------------------------------------------------------------------------------------------------------------
//

void div_rational_number(struct rational* val1, struct rational* val2, struct rational* result)
{
    result->numerator = val1->numerator * val2->denominator;
    result->denominator = val1->denominator * val2->numerator;
}

//----------------------------------------------------------------------------------------------------------------------
//

void pow_rational_number(struct rational* val1, struct rational* val2, struct rational* result)
{
    if (val2->numerator > 0)
    {
        ipow(val1->numerator, val2->numerator, &result->numerator);
        ipow(val1->denominator, val2->numerator, &result->denominator);
    }
    else
    {
        ipow(val1->denominator, -val2->numerator, &result->numerator);
        ipow(val1->numerator, -val2->numerator, &result->denominator);
    }
}

///--------------------------------------
///

#define item_type_None 0
#define item_type_SourceValue 1
#define item_type_CalculatedValue 2
#define item_type_Operand 3
#define item_type_NANValue 4
#define item_type_BadConcat 5
#define item_type_ExponentNotInteger 6

struct rpn_item
{
    int type;
    int op;
    struct rational number;
};

void rpn_item_validate_and_reduce_calculated_value(struct rpn_item* t)
{
    if (t->number.denominator == 0)
    {
        t->type = item_type_NANValue;
    }
    else
    {
        t->type = item_type_CalculatedValue;
        rational_reduce(&t->number);
    }
}

void rpn_item_set_operation(struct rpn_item* t, int o)
{
    t->type = item_type_Operand;
    t->op = o;
}

void rpn_item_set_source_value(struct rpn_item* t, long val)
{
    t->type = item_type_SourceValue;
    t->number.numerator = val;
    t->number.denominator = 1;
}

void rpn_item_set_calculated_value(struct rpn_item* t, long numerator, long denominator)
{
    t->number.numerator = numerator;
    t->number.denominator = denominator;

    if (t->number.denominator == 0)
    {
        t->type = item_type_NANValue;
    }
    else
    {
        t->type = item_type_CalculatedValue;
        rational_reduce(&t->number);
    }
}

void rpn_item_set_error_value(struct rpn_item* t, int error_type)
{
    t->type = error_type;
}

bool rpn_item_is_number(struct rpn_item* t)
{
    return t->type == item_type_CalculatedValue || t->type == item_type_SourceValue;
}

///--------------------------------------
///

struct rpn_stack
{
    struct rpn_item m_stack[STACK_SIZE];
    int m_ptr;
};

struct rpn_item* rpn_stack_push(struct rpn_stack* t)
{
    return &t->m_stack[++t->m_ptr];
}

//----------------------------------------------------------------------------------------------------------------------
//

void rpn_stack_pop(struct rpn_stack* t)
{
    t->m_ptr--;
}

//----------------------------------------------------------------------------------------------------------------------
//

struct rpn_item rpn_stack_top(struct rpn_stack* t)
{
    return t->m_stack[t->m_ptr];
}

//----------------------------------------------------------------------------------------------------------------------
//

int rpn_stack_size(struct rpn_stack* t)
{
    return t->m_ptr + 1;
}

//----------------------------------------------------------------------------------------------------------------------
//

void rpn_stack_clear(struct rpn_stack* t)
{
    t->m_ptr = -1;
}

///--------------------------------------
///


struct rpn_expression
{
    struct rpn_item m_expression[NUMBER_OF_DIGITS + NUMBER_OF_OPS];
    struct rpn_stack m_stack;
};

void rpn_expression_evaluate(struct rpn_expression* t)
{
    rpn_stack_clear(&t->m_stack);

   // for (const rpn_item& item : m_expression)
    for (int i = 0; i < NUMBER_OF_DIGITS + NUMBER_OF_OPS; ++i)
    {
        struct rpn_item* item = &t->m_expression[i];

        switch (item->type)
        {
        case item_type_Operand:
        {
            struct rpn_item val2 = rpn_stack_top(&t->m_stack);
            rpn_stack_pop(&t->m_stack);
            struct rpn_item val1 = rpn_stack_top(&t->m_stack);
            rpn_stack_pop(&t->m_stack);

            switch (item->op)
            {
            case rpn_operation_Add:
            {
                struct rpn_item* result = rpn_stack_push(&t->m_stack);
                add_rational_number(&val1.number, &val2.number, &result->number);
                rpn_item_validate_and_reduce_calculated_value(result);
                break;
            }
            case rpn_operation_Sub:
            {
                struct rpn_item* result = rpn_stack_push(&t->m_stack);
                sub_rational_number(&val1.number, &val2.number, &result->number);
                rpn_item_validate_and_reduce_calculated_value(result);
                break;
            }
            case rpn_operation_Mul:
            {
                struct rpn_item* result = rpn_stack_push(&t->m_stack);
                mul_rational_number(&val1.number, &val2.number, &result->number);
                rpn_item_validate_and_reduce_calculated_value(result);
                break;
            }
            case rpn_operation_Div:
            {
                struct rpn_item* result = rpn_stack_push(&t->m_stack);
                div_rational_number(&val1.number, &val2.number, &result->number);
                rpn_item_validate_and_reduce_calculated_value(result);
                break;
            }
            case rpn_operation_Pow:
            {
                if (val2.number.denominator == 1)
                {
                    struct rpn_item* result = rpn_stack_push(&t->m_stack);
                    pow_rational_number(&val1.number, &val2.number, &result->number);
                    rpn_item_validate_and_reduce_calculated_value(result);
                }
                else
                {
                    struct rpn_item* result = rpn_stack_push(&t->m_stack);
                    rpn_item_set_error_value(result, item_type_ExponentNotInteger);
                }
                break;
            }
            case rpn_operation_Concat:
            {
                if (val1.type != item_type_SourceValue || val2.type != item_type_SourceValue)
                {
                    struct rpn_item* result = rpn_stack_push(&t->m_stack);
                    rpn_item_set_error_value(result, item_type_BadConcat);
                }
                else
                {
                    long left = val1.number.numerator;
                    long right = val2.number.numerator;

                    long mult = 10L;
                    while ((right / mult))
                    {
                        mult *= 10L;
                    }

                    long numerator = right + left * mult;

                    struct rpn_item* result = rpn_stack_push(&t->m_stack);
                    rpn_item_set_source_value(result, numerator);
                }
                break;
            }

            };

            break;
        }

        case item_type_SourceValue:
        {
            struct rpn_item* value = rpn_stack_push(&t->m_stack);
            rpn_item_set_source_value(value, item->number.numerator);
            break;
        }

        };

        struct rpn_item top = rpn_stack_top(&t->m_stack);
        if (!rpn_item_is_number(&top))
        {
            break;
        }
    }

}

///--------------------------------------
///

struct expression
{
    int aiOps[m_number_of_operators];
    int aiOpLocations[m_number_of_operator_locatations];
    struct rpn_expression m_rpnExpression;
};

void expression_resolve_operations(struct expression* t, long iOpCombinationCode)
{
    for (int n = 0; n < m_number_of_operators; ++n)
    {
        t->aiOps[n] = iOpCombinationCode % rpn_operation__Count;
        iOpCombinationCode /= rpn_operation__Count;
    }
}

void expression_resolve_operation_locations(struct expression* t, long iOpLocationCode)
{
    for (int n = 0; n < m_number_of_operator_locatations; ++n)
    {
        t->aiOpLocations[n] = 0;
    }

    t->aiOpLocations[0]++;
    for (int n = 2; n <= m_number_of_operators; ++n)
    {
        int iOpLocation = iOpLocationCode%n;
        t->aiOpLocations[iOpLocation]++;
        iOpLocationCode /= n;
    }
}

void expression_generate_rpn_expression(struct expression* t)
{
    long iCharPtr = NUMBER_OF_DIGITS + m_number_of_operators - 1;
    long iNumDigits = 0;
    long iOpIdx = 0;

    while (iCharPtr >= 0)
    {
        if (iNumDigits < m_number_of_operator_locatations && t->aiOpLocations[iNumDigits] > 0)
        {
            //m_rpnExpression.m_expression[iCharPtr].set_operation(aiOps[iOpIdx++]);        
            rpn_item_set_operation(&t->m_rpnExpression.m_expression[iCharPtr], t->aiOps[iOpIdx++]);
            t->aiOpLocations[iNumDigits]--;
        }
        else
        {
            //m_rpnExpression.m_expression[iCharPtr].set_source_value(NUMBER_OF_DIGITS - iNumDigits);
            rpn_item_set_source_value(&t->m_rpnExpression.m_expression[iCharPtr], NUMBER_OF_DIGITS - iNumDigits);
            iNumDigits++;
        }

        iCharPtr--;
    }
}

void expression_resolve_expression(struct expression* t, long combination_code)
{
    long iOpCombinationCode = combination_code % iNumOpCombinations;
    long iOpLocationCode = combination_code / iNumOpCombinations;

    expression_resolve_operations(t, iOpCombinationCode);
    expression_resolve_operation_locations(t, iOpLocationCode);
    expression_generate_rpn_expression(t);
}

void expression_evaluate(struct expression* t)
{
    rpn_expression_evaluate(&t->m_rpnExpression);
}

///--------------------------------------
///

__kernel void hello(__global long* results)
{
    long id = get_global_id(0);

    //printf("hello: %d\n", id);

    struct expression e;
    expression_resolve_expression(&e, id);
    expression_evaluate(&e);

    results[id] = e.aiOpLocations[0];
}