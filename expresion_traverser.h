#pragma once

//----------------------------------------------------------------------------------------------------------------------
//

class expression_traverser
{
public:

    struct config_t
    {
        bool printAllCombinations = false;
        bool multiThreaded = true;
        bool printStats = true;

        bool benchmarkMode;
        int64_t benchmarkModeMaxCombinations;

        bool generateResultArray;
        int64_t arraySize;
    };

    typedef expression<9> expression_t;


    expression_traverser(const config_t& config)
        : m_config(config)
        , results(m_config.arraySize)
    {}


    std::vector<std::pair<expression_t, spin_lock>> results;

    std::atomic<int64_t> iIntsInRange = 0;
    std::atomic<int64_t> iIntsPositive = 0;
    std::atomic<int64_t> iIntsNegative = 0;
    std::atomic<int64_t> iFractionsPositive = 0;
    std::atomic<int64_t> iFractionsNegative = 0;
    std::atomic<int64_t> iErrorsNANValue = 0;
    std::atomic<int64_t> iErrorsOverflow = 0;
    std::atomic<int64_t> iErrorsBadConcat = 0;
    std::atomic<int64_t> iErrorsExponentNotInteger = 0;
    std::atomic<int64_t> iTotal = 0;

    void calculate_range_thread(int64_t start, int64_t end);

    void calculate();

private:
    config_t m_config;
};

//----------------------------------------------------------------------------------------------------------------------
//

#include "expresion_traverser.inl"
