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

    void calculate_range_thread(int64_t start, int64_t end)
    {
        expression_t e;

        for (int64_t i = start; i < end; ++i)
        {
            e.resolve_expression(i);
            e.evaluate();

            if (m_config.printAllCombinations)
            {
                // TOFIX printf from threads

                std::string epxString = e.to_string();
                printf(epxString.c_str());
                if ((e.get_result().is_number()))
                {
                    if (e.get_result().number.denominator == 1)
                    {
                        printf(" = %I64d\n", e.get_result().number.numerator);
                    }
                    else
                    {
                        printf(" = %I64d/%I64d\n", e.get_result().number.numerator, e.get_result().number.denominator);
                    }
                }
                else
                {
                    printf(" = \n");
                }
            }

            if (m_config.printStats)
            {
                iTotal++;

                if (e.get_result().is_number())
                {
                    assert(e.get_result().number.denominator > 0);

                    if (e.get_result().number.numerator >= 0)
                    {
                        if (e.get_result().number.denominator == 1)
                        {
                            iIntsPositive++;

                            if (e.get_result().number.numerator < m_config.arraySize)
                            {
                                iIntsInRange++;
                            }
                        }
                        else
                        {
                            iFractionsPositive++;
                        }
                    }
                    else
                    {
                        if (e.get_result().number.denominator == 1)
                        {
                            iIntsNegative++;
                        }
                        else
                        {
                            iFractionsNegative++;
                        }
                    }
                }
                else
                {
                    assert(e.get_result().is_error_value());

                    switch (e.get_result().get_type())
                    {
                    case item_type::NANValue:
                        iErrorsNANValue++;
                        break;
                    case item_type::Overflow:
                        iErrorsOverflow++;
                        break;
                    case item_type::BadConcat:
                        iErrorsBadConcat++;
                        break;
                    case item_type::ExponentNotInteger:
                        iErrorsExponentNotInteger++;
                        break;
                    default:
                        assert(0);
                    }
                }
            }

            if (m_config.generateResultArray)
            {
                if ((e.get_result().is_number()) && e.get_result().number.denominator == 1)
                {
                    if (e.get_result().number.numerator >= 0 && e.get_result().number.numerator < m_config.arraySize)
                    {
                        results[e.get_result().number.numerator].second.lock();

                        if (results[e.get_result().number.numerator].first.get_result().get_type() == item_type::None)
                        {
                            results[e.get_result().number.numerator].first = e;
                        }

                        results[e.get_result().number.numerator].second.unlock();
                    }
                }
            }
        }
    };

    void calculate()
    {
        timer timer;

        expression_t::init();

        int64_t iNumTotalCombinations = expression_t::get_total_number_of_combinations();
        if (m_config.benchmarkMode)
        {
            iNumTotalCombinations = iNumTotalCombinations > m_config.benchmarkModeMaxCombinations ? m_config.benchmarkModeMaxCombinations : iNumTotalCombinations;
        }

        printf("NumDigits: %I64d\n", expression_t::get_num_digits());
        printf("NumOpSpots: %I64d\n", expression_t::get_num_operator_locations());
        printf("NumOps: %I64d\n", expression_t::get_num_operators());
        printf("iNumOpCombinations: %I64d\n", expression_t::get_num_op_combinations());
        printf("iNumOpLocationCombinations: %I64d\n", expression_t::get_num_op_locations());
        printf("iNumTotalCombinations: %I64d\n", iNumTotalCombinations);


        float fEstimatedMCPS = 29.0f;
        float fEstimatedTime = iNumTotalCombinations / (fEstimatedMCPS * 1000000.0f);
        printf("Estimated Time: %.2f\n", fEstimatedTime);


        std::vector<std::thread> vThreads;
        int64_t numThreads = m_config.multiThreaded ? std::thread::hardware_concurrency() : 1;
        int64_t combPerThread = iNumTotalCombinations / numThreads;


        printf("Threads: %I64d\n", numThreads);

        for (int t = 0; t < numThreads; ++t)
        {
            int64_t start = t * combPerThread;
            int64_t end = (t + 1) * combPerThread;
            if (t == numThreads - 1)
            {
                end = iNumTotalCombinations;
            }

            vThreads.emplace_back(&expression_traverser::calculate_range_thread, this, start, end);
        }

        if (m_config.printStats)
        {
            int print_progress_counter = 0;
            while (iTotal < iNumTotalCombinations)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                print_progress_counter++;
                if (print_progress_counter == 100)
                {
                    print_progress_counter = 0;
                    float percent_completed = (float)iTotal / (float)iNumTotalCombinations * 100.0f;
                    printf("%.2f%%...\n", percent_completed);
                }
            }
        }

        for (std::thread& thread : vThreads)
        {
            thread.join();
        }

        float time = timer.get_elapsed_time();

        if (m_config.generateResultArray)
        {
            for (int64_t i = 0; i < m_config.arraySize; ++i)
            {
                std::string epxString = results[i].first.to_string();
                printf("%I64d = %s\n", i, epxString.c_str());
            }
        }

        if (m_config.printStats)
        {
            printf("iIntsInRange = %I64d (%.2f%%)\n", (int64_t)iIntsInRange, (float)iIntsInRange / (float)iTotal * 100.0f);
            printf("iIntsPositive = %I64d (%.2f%%)\n", (int64_t)iIntsPositive, (float)iIntsPositive / (float)iTotal * 100.0f);
            printf("iIntsNegative = %I64d (%.2f%%)\n", (int64_t)iIntsNegative, (float)iIntsNegative / (float)iTotal * 100.0f);
            printf("iFractionsPositive = %I64d (%.2f%%)\n", (int64_t)iFractionsPositive, (float)iFractionsPositive / (float)iTotal * 100.0f);
            printf("iFractionsNegative = %I64d (%.2f%%)\n", (int64_t)iFractionsNegative, (float)iFractionsNegative / (float)iTotal * 100.0f);
            printf("iErrorsNANValue = %I64d (%.2f%%)\n", (int64_t)iErrorsNANValue, (float)iErrorsNANValue / (float)iTotal * 100.0f);
            printf("iErrorsOverflow = %I64d (%.2f%%)\n", (int64_t)iErrorsOverflow, (float)iErrorsOverflow / (float)iTotal * 100.0f);
            printf("iErrorsBadConcat = %I64d (%.2f%%)\n", (int64_t)iErrorsBadConcat, (float)iErrorsBadConcat / (float)iTotal * 100.0f);
            printf("iErrorsExponentNotInteger = %I64d (%.2f%%)\n", (int64_t)iErrorsExponentNotInteger, (float)iErrorsExponentNotInteger / (float)iTotal * 100.0f);

        }

        printf("Time: %.2f\n", time);
        float fMCPS = (float)iNumTotalCombinations / (time*1000000.0f);
        printf("Millions of combinations/s: %.2f\n", fMCPS);
    }

private:
    config_t m_config;
};

//----------------------------------------------------------------------------------------------------------------------
//

#include "expresion_traverser.inl"
