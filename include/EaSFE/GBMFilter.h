#pragma once
#include "LightGBM/c_api.h"
#include "DataSet.h"
#include "FCOperators.h"
#include "Tools.h"

namespace EaSFE
{
    class GBMFilter
    {
    public:
        GBMFilter(DataSet* fcdataset, std::vector<std::pair<PFeatureInfo, PFCOperators>> & featurefcops);
        ~GBMFilter();
        void produceScore(int numsfeatures);
        void clear();
    private:
        DatasetHandle generateLightGBMDatasetFromSample();
        DatasetHandle generateLightGBMDatasetFromMat();

        DatasetHandle AddLightGBMDatasetFromSample(const vector<int>& featureindexs = vector<int>());
        DatasetHandle AddLightGBMDatasetFromMat(const vector<int>& featureindexs = vector<int>());

        DatasetHandle generateLightGBMDataset(int type = 1);
        DatasetHandle AddLightGBMDataset(const vector<int>& featureindexs=vector<int>(), int type = 1);
        std::pair<DatasetHandle, DatasetHandle> generateLightGBMDatasetFromOne(int featureindex, const vector<int>& instanceindex, const vector<int>& valinstanceindex);
        void InitDataSet();
        BoosterHandle InitBooster(DatasetHandle train_data, DatasetHandle valid_data) ;
        void getParams(std::string &paramsstr);
        void produceScore(const vector<int>& trainrows, const vector<int>& valrows, vector<int>& featureindex, vector<double>& scores);
        void produceScore(vector<int>& featureindex, vector<double>& scores);
        bool isBetter(double score);
        void removeFeatures(vector<int>& featureindexs, vector<double>& scores, bool isrepeat = true, int leftfeaturenum = -1);
        void checkStatus(int flag, const std::string & message);
        DatasetHandle getSubDataset(DatasetHandle dataset, const std::vector<int>& instanceindex);
        bool earlyStop(double score, int earlystopround, int& earlystopcount, double& bestscore);
    DataSet* m_fcdataset;
    std::vector<std::pair<PFeatureInfo, PFCOperators>> & m_featurefcops;
    // DatasetHandle m_train_data = nullptr;
    // DatasetHandle m_valid_data = nullptr;
    double m_bestscore = 0;
    vector<double> m_init_scores;
    };

}
