#pragma once
#include <vector>
namespace EaSFE {
    class Combination
    {
        int featurenums, featureselections;
        std::vector<int> sequence;
        long long totalcombinations;
        long long leftcombinations;
    public:
        static long long Fact(int n, int e=0);
        Combination(int featurenums, int featureselections);

        bool hasNext();
        std::vector<int> getNext();
    };
}//EaSFE