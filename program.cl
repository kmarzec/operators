


__kernel void hello(__global int* ops, __global int* opsloc)
{
    int NUMBER_OF_DIGITS = 9;
    int m_number_of_operator_locatations = NUMBER_OF_DIGITS - 1;
    int m_number_of_operators = NUMBER_OF_DIGITS - 1;




    int iNumOpLocationCombinations = 40320;
    int iNumOpCombinations = 1679616;
    int iNumTotalCombinations = 67722117120;

    int rpn_operation__Count = 6;

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