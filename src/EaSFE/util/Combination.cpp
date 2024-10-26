#include "EaSFE/util/Combination.h"

namespace EaSFE{
        long long Combination::Fact(int n, int e)
        {
            long long result = 1;
            for (int i = n; i > e; --i)
                result *= i;
            return result;
        }
        Combination::Combination(int featurenums, int featureselections)
        {
            this->featurenums = featurenums;
            this->featureselections = featureselections;
            sequence.resize(featureselections);
            
            totalcombinations = Fact(featurenums, featurenums-featureselections) / Fact(featureselections);
            leftcombinations = totalcombinations;
            for (int i = 0; i < featureselections; ++i)
                sequence[i] = i;
        }

        bool Combination::hasNext()
        {
            return leftcombinations > 0;
        }
        std::vector<int> Combination::getNext()
        {
            if (leftcombinations == 0)
                return std::vector<int>();
            if (leftcombinations == totalcombinations)
            {
                leftcombinations--;
                return sequence;
            }
            int i = featureselections - 1;
            while (i >= 0 && sequence[i] == featurenums - featureselections + i)
                --i;
            if (i < 0)
                return std::vector<int>();
            ++sequence[i];
            for (int j = i + 1; j < featureselections; ++j)
                sequence[j] = sequence[i] + j - i;
            --leftcombinations;
            return sequence;
        }
}

