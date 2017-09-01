

#define NUMBER_OF_DIGITS 9
#define NUMBER_OF_OPS 8

#define m_number_of_operator_locatations = (NUMBER_OF_DIGITS - 1);
#define m_number_of_operators = (NUMBER_OF_DIGITS - 1);




int iNumOpLocationCombinations = 40320;
int iNumOpCombinations = 1679616;
int iNumTotalCombinations = 67722117120;


int rpn_operation__Count = 6;


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

void reduce(struct rational* r)
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

#define RPNTYPE_None 0
#define RPNTYPE_SourceValue 1
#define RPNTYPE_CalculatedValue 2
#define RPNTYPE_Operand 3
#define RPNTYPE_NANValue 4


struct rpn_item
{
    int type;
    int op;
    struct rational number;
};


void set_operation(struct rpn_item* t, int o)
{
    t->type = RPNTYPE_Operand;
    t->op = o;
}

void set_source_value(struct rpn_item* t, long val)
{
    t->type = RPNTYPE_SourceValue;
    t->number.numerator = val;
    t->number.denominator = 1;
}

void set_calculated_value(struct rpn_item* t, long numerator, long denominator)
{
    t->number.numerator = numerator;
    t->number.denominator = denominator;

    if (t->number.denominator == 0)
    {
        t->type = RPNTYPE_NANValue;
    }
    else
    {
        t->type = RPNTYPE_CalculatedValue;
        reduce(&t->number);
    }
}

///--------------------------------------
///

struct rpn_expression
{
    struct rpn_item m_expression[NUMBER_OF_DIGITS + NUMBER_OF_OPS];
};

///--------------------------------------
///

struct expression
{
    int aiOps[m_number_of_operators];
    int64_t aiOpLocations[m_number_of_operator_locatations];
};


void resolve_expression(struct expression* t, long combination_code)
{
    long iOpCombinationCode = combination_code % iNumOpCombinations;
    long iOpLocationCode = combination_code / iNumOpCombinations;

    resolve_operations(iOpCombinationCode);
    resolve_operation_locations(iOpLocationCode);
    generate_rpn_expression();
}

///--------------------------------------
///

__kernel void hello(__global int* ops, __global int* opsloc)
{
   // int NUMBER_OF_DIGITS = 9;
   

  

    /////////////////////

    int id = get_global_id(0);


    long iOpCombinationCode = id % iNumOpCombinations;
    long iOpLocationCode = id / iNumOpCombinations;


    // resolve ops
    for (int n = 0; n < m_number_of_operators; ++n)
    {
        ops[m_number_of_operators*id + n] = iOpCombinationCode % rpn_operation__Count;
        iOpCombinationCode /= rpn_operation__Count;
    }


    // resolve ops loc

    for (int n = 0; n < m_number_of_operator_locatations; ++n)
    {
        opsloc[m_number_of_operator_locatations*id + n] = 0;
    }

    opsloc[m_number_of_operator_locatations*id]++;
    for (int n = 2; n <= m_number_of_operators; ++n)
    {
        int iOpLocation = iOpLocationCode % n;
        opsloc[m_number_of_operator_locatations*id + iOpLocation]++;
        iOpLocationCode /= n;
    }


    //int id = get_global_id(0);
    // v3[id] = v1[id] + v2[id]; 
}