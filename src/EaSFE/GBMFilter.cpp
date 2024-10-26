#include "EaSFE/GBMFilter.h"

namespace EaSFE
{
    GBMFilter::GBMFilter(DataSet* fcdataset, std::vector<std::pair<PFeatureInfo, PFCOperators>> & featurefcops):m_fcdataset(fcdataset), m_featurefcops(featurefcops) {}
    GBMFilter::~GBMFilter() {clear();}
    DatasetHandle GBMFilter::generateLightGBMDatasetFromSample()
    {
        return nullptr;
    }
    
    DatasetHandle GBMFilter::generateLightGBMDatasetFromMat()
    {
        auto numclass = Property::getProperty()->getTargetClasses();
        MyDataType *datavalues;
        auto tempdataset = m_fcdataset;
        auto numinstances= tempdataset->getInstancesOfFeature();
        
        auto orifeatureinfos = tempdataset->getFeatures();
        CHECK_GE(orifeatureinfos.size(), 1);
        int featurenums = orifeatureinfos.size();
        datavalues = new MyDataType[featurenums*numinstances];
        string categorical_features = " categorical_feature=";
		
        for(int i = 0; i < orifeatureinfos.size(); ++i)
        {
            orifeatureinfos[i] = tempdataset->getFeatureFromOld(orifeatureinfos[i]);
            auto featurevalues = orifeatureinfos[i]->getFeature()->getValues();
            if(orifeatureinfos[i]->getType() == OutType::Discrete)
            {
                categorical_features += std::to_string(i) + ",";
                for(int j = 0;j < numinstances; ++j)
                    datavalues[i*numinstances + j] = static_cast<MyDataType>(reinterpret_cast<int*>(featurevalues)[j]);
            }
            else
                memcpy(datavalues + i*numinstances, reinterpret_cast<MyDataType*>(featurevalues), numinstances * sizeof(MyDataType));
            tempdataset->clearFeatureData(orifeatureinfos[i]);
        }
       
        if (categorical_features != "categorical_feature=")
            categorical_features.pop_back(), categorical_features += " ";
        else categorical_features = "";
        string parameters;
        getParams(parameters);
        parameters += categorical_features;
        DatasetHandle dataset;
        auto datatype = sizeof(MyDataType) == 4 ? C_API_DTYPE_FLOAT32 : C_API_DTYPE_FLOAT64;
        int flag = LGBM_DatasetCreateFromMat(datavalues, datatype, numinstances, featurenums, 0, parameters.c_str(), nullptr, &dataset);
        int targetindex = tempdataset->getDatasetIndex()[tempdataset->getID()];
        
        auto featurevalues = tempdataset->getTargetFeature()->getFeature()->getValues();
        MyDataType *targetvalues = new MyDataType[numinstances];
        if(tempdataset->getTargetFeature()->getType() == OutType::Discrete)
        {
            for (int i = 0; i < numinstances; ++i)
                targetvalues[i] = static_cast<MyDataType>(reinterpret_cast<int*>(featurevalues)[i+targetindex]);
        }
        else
            memcpy(targetvalues, reinterpret_cast<MyDataType*>(featurevalues) + targetindex, numinstances * sizeof(MyDataType));
        int hasinit = 1;
        double* init_scores = m_init_scores.size() == 0 ? nullptr : m_init_scores.data();
        if (init_scores == nullptr) hasinit = 0;
        numclass = numclass <= 2 ? 1 : numclass;
        flag = LGBM_DatasetInitStreaming(dataset, 0, hasinit, 0, numclass, 60, -1);
        flag = LGBM_DatasetPushRowsWithMetadata(dataset, datavalues, datatype, numinstances, featurenums, 0, targetvalues, nullptr, init_scores, nullptr, 0);
        flag = LGBM_DatasetMarkFinished(dataset);
        checkStatus(flag, "GBMFilter generateLightGBMDatasetFromMat LGBM_DatasetCreateFromSampledColumn");
       
        try{
            delete[] datavalues;
            delete[] targetvalues;
            targetvalues = nullptr;
            datavalues = nullptr;
        }catch(std::exception & e)
        {
            LOG(WARNING) << e.what();
        }
        return dataset;
    }

    DatasetHandle GBMFilter::AddLightGBMDatasetFromSample(const vector<int>& featureindexs)
    {
        return nullptr;
    }

    DatasetHandle GBMFilter::AddLightGBMDatasetFromMat(const vector<int>& featureindexs)
    {
        auto numclass = Property::getProperty()->getTargetClasses();
        MyDataType* datavalues;
        auto tempdataset = m_fcdataset;
        auto numinstances = tempdataset->getInstancesOfFeature();
        auto orifeatureinfos = tempdataset->getFeatures();
        CHECK_GE(orifeatureinfos.size(), 1);
        int featurenums = orifeatureinfos.size() + featureindexs.size();
        datavalues = new MyDataType[featurenums * numinstances];
        string categorical_features = " categorical_feature=";

        for (int i = 0; i < orifeatureinfos.size(); ++i)
        {
            orifeatureinfos[i] = tempdataset->getFeatureFromOld(orifeatureinfos[i]);
            auto featurevalues = orifeatureinfos[i]->getFeature()->getValues();
            if (orifeatureinfos[i]->getType() == OutType::Discrete)
            {
                categorical_features += std::to_string(i) + ",";
                for (int j = 0; j < numinstances; ++j)
                    datavalues[i * numinstances + j] = static_cast<MyDataType>(reinterpret_cast<int*>(featurevalues)[j]);
            }
            else
                memcpy(datavalues + i * numinstances, reinterpret_cast<MyDataType*>(featurevalues), numinstances * sizeof(MyDataType));
            tempdataset->clearFeatureData(orifeatureinfos[i]);
        }


        vector<PFeatureInfo> featureinfos;
        for (auto& featureindex : featureindexs)
        {
            CHECK_GE(m_featurefcops.size() - 1, featureindex);
            if (m_featurefcops[featureindex].first != nullptr)
                featureinfos.push_back(m_featurefcops[featureindex].first);
            else
            {
                featureinfos.push_back(generateFeature(tempdataset, m_featurefcops[featureindex].second));
                m_featurefcops[featureindex].first = featureinfos.back();
            }
        }


        for (int i = 0; i < featureinfos.size(); ++i)
        {
            int featureindex = orifeatureinfos.size() + i;
            auto featurevalues = featureinfos[i]->getFeature()->getValues();
            if (featureinfos[i]->getType() == OutType::Discrete)
            {
                categorical_features += std::to_string(featureindex) + ",";
                for (int j = 0; j < numinstances; ++j)
                    datavalues[(featureindex)*numinstances + j] = static_cast<MyDataType>(reinterpret_cast<int*>(featurevalues)[j]);
            }
            else
                memcpy(datavalues + (featureindex)*numinstances, reinterpret_cast<MyDataType*>(featurevalues), numinstances * sizeof(MyDataType));
        }
        if (categorical_features != "categorical_feature=")
            categorical_features.pop_back(), categorical_features += " ";
        else categorical_features = "";
        string parameters;
        getParams(parameters);
        parameters += categorical_features;
        DatasetHandle dataset;
        auto datatype = sizeof(MyDataType) == 4 ? C_API_DTYPE_FLOAT32 : C_API_DTYPE_FLOAT64;
        int flag = LGBM_DatasetCreateFromMat(datavalues, datatype, numinstances, featurenums, 0, parameters.c_str(), nullptr, &dataset);
        int targetindex = tempdataset->getDatasetIndex()[tempdataset->getID()];

        auto featurevalues = tempdataset->getTargetFeature()->getFeature()->getValues();
        MyDataType* targetvalues = new MyDataType[numinstances];
        if (tempdataset->getTargetFeature()->getType() == OutType::Discrete)
        {
            for (int i = 0; i < numinstances; ++i)
                targetvalues[i] = static_cast<MyDataType>(reinterpret_cast<int*>(featurevalues)[i + targetindex]);
        }
        else
            memcpy(targetvalues, reinterpret_cast<MyDataType*>(featurevalues) + targetindex, numinstances * sizeof(MyDataType));
        int hasinit = 1;
        double* init_scores = m_init_scores.size() == 0 ? nullptr : m_init_scores.data();
        if (init_scores == nullptr) hasinit = 0;
        numclass = numclass <= 2 ? 1 : numclass;
        flag = LGBM_DatasetInitStreaming(dataset, 0, hasinit, 0, numclass, 20, -1);
        flag = LGBM_DatasetPushRowsWithMetadata(dataset, datavalues, datatype, numinstances, featurenums, 0, targetvalues, nullptr, init_scores, nullptr, 0);
        flag = LGBM_DatasetMarkFinished(dataset);
        checkStatus(flag, "GBMFilter generateLightGBMDatasetFromMat LGBM_DatasetCreateFromSampledColumn");

        try {
            delete[] datavalues;
            delete[] targetvalues;
            targetvalues = nullptr;
            datavalues = nullptr;
        }
        catch (std::exception& e)
        {
            LOG(WARNING) << e.what();
        }
        return dataset;
    }
    


    DatasetHandle GBMFilter::generateLightGBMDataset(int type)
    {
        if(type == 0)
            return generateLightGBMDatasetFromSample();
        else if(type == 1)
            return generateLightGBMDatasetFromMat();
        return nullptr;
    }

    DatasetHandle GBMFilter::AddLightGBMDataset(const vector<int>& featureindexs, int type)
    {
        if (type == 0)
			return AddLightGBMDatasetFromSample(featureindexs);
		else if (type == 1)
			return AddLightGBMDatasetFromMat(featureindexs);
        return nullptr;
    }


    std::pair<DatasetHandle, DatasetHandle> GBMFilter::generateLightGBMDatasetFromOne(int featureindex, const vector<int>& instanceindex, const vector<int>& valinstanceindex)
    {
        MyDataType *datavalues;
        auto tempdataset = m_fcdataset;
        auto numinstances= m_fcdataset->getInstancesOfFeature();
        CHECK_GE(m_featurefcops.size(), featureindex+1);
        int featurenums = 1;
        datavalues = new MyDataType[numinstances];
        string categorical_features = " categorical_feature=";
        PFeatureInfo featureinfos = nullptr;
        auto& fcops = m_featurefcops[featureindex].second;
        
        featureinfos = m_featurefcops[featureindex].first;
        if(featureinfos == nullptr)
            m_featurefcops[featureindex].first = featureinfos = generateFeature(tempdataset, fcops);
        auto featurevalues = featureinfos->getFeature()->getValues();
        if(featureinfos->getType() == OutType::Discrete)
        {
            categorical_features += std::to_string(0) + ",";
            for(int j = 0;j < numinstances; ++j)
                datavalues[j] = static_cast<MyDataType>(reinterpret_cast<int*>(featurevalues)[j]);
        }
        else
            memcpy(datavalues, reinterpret_cast<MyDataType*>(featurevalues), numinstances * sizeof(MyDataType));
        if (categorical_features != "categorical_feature=")
            categorical_features.pop_back(), categorical_features += " ";
        else categorical_features = "";
        string parameters;
        getParams(parameters);
        parameters += categorical_features;
        DatasetHandle dataset;
        auto datatype = sizeof(MyDataType) == 4 ? C_API_DTYPE_FLOAT32 : C_API_DTYPE_FLOAT64;
        int flag = LGBM_DatasetCreateFromMat(datavalues, datatype, numinstances, featurenums, 0, parameters.c_str(), nullptr, &dataset);
        int targetindex = tempdataset->getDatasetIndex()[tempdataset->getID()];
        
        auto targetfeaturevalues = tempdataset->getTargetFeature()->getFeature()->getValues();
        MyDataType *targetvalues = new MyDataType[numinstances];
        if(tempdataset->getTargetFeature()->getType() == OutType::Discrete)
        {
            for (int i = 0; i < numinstances; ++i)
                targetvalues[i] = static_cast<MyDataType>(reinterpret_cast<int*>(targetfeaturevalues)[i+targetindex]);
        }
        else
            for (int i = 0; i < numinstances; ++i)
                targetvalues[i] = reinterpret_cast<MyDataType*>(targetfeaturevalues)[i+targetindex];
        auto numclass = Property::getProperty()->getTargetClasses();
        int hasinit = 1;
        double* init_scores = m_init_scores.data();
        numclass = numclass <= 2 ? 1 : numclass;
        flag = LGBM_DatasetInitStreaming(dataset, 0, hasinit, 0, numclass, 20, -1);
        flag = LGBM_DatasetPushRowsWithMetadata(dataset, datavalues, datatype, numinstances, 1, 0, targetvalues, nullptr, init_scores, nullptr, 0);
        flag = LGBM_DatasetMarkFinished(dataset);
        checkStatus(flag, "GBMFilter generateLightGBMDatasetFromOne LGBM_DatasetPushRowsWithMetadata");
        try{
            delete[] datavalues;
            datavalues = nullptr;
            delete[] targetvalues;
            targetvalues = nullptr;
        }catch(std::exception & e)
        {
            LOG(WARNING) << e.what();
        }
        auto traindataset = getSubDataset(dataset, instanceindex);
        auto valdataset = getSubDataset(dataset, valinstanceindex);
        LGBM_DatasetFree(dataset);
        return std::make_pair(traindataset, valdataset);
    }

    void GBMFilter::InitDataSet()
    {
        clear();
        LOG(INFO) << "Start to init the dataset for the model";
        DatasetHandle data = generateLightGBMDataset();
        int instancesoftrain = m_fcdataset->m_trainindex.size();
        int instancesofvalid = m_fcdataset->m_valindex.size();
        auto train_data = getSubDataset(data, m_fcdataset->m_trainindex);
        auto valid_data = getSubDataset(data, m_fcdataset->m_valindex);
        LGBM_DatasetFree(data);
        auto booster = InitBooster(train_data, valid_data);
        int is_finished = 0;
        int out_len = 0;
        LGBM_BoosterGetEvalCounts(booster, &out_len);

        double * out_results = new double[out_len];
        int earlystopcount = 0;
        const auto &lightgbmparams = Property::getProperty()->getLightGBMParams();
        double bestscore = 1e10;
        double score = 0;
        int numclass = Property::getProperty()->getTargetClasses();
        for(int iter =0;iter < 10000;++ iter)
        {
            int flag = LGBM_BoosterUpdateOneIter(booster, &is_finished);
            checkStatus(flag, "GBMFilter InitDataSet BoosterUpdateOneIter");
            if (is_finished) break;
            LGBM_BoosterGetEval(booster, 1, &out_len, out_results);
            score = out_results[0];
            if(earlyStop(score, 200, earlystopcount, bestscore)) break;
        }
        m_bestscore = bestscore;
        delete[] out_results;
        int numinstances = m_fcdataset->getInstancesOfFeature();
        numclass = numclass <= 2 ? 1 : numclass;
        out_results = new double[numclass * numinstances];
        int64_t out_len2 = 0, out_len3 = 0;
        LGBM_BoosterGetPredict(booster, 0, &out_len2, out_results);
        m_init_scores.resize(numclass * numinstances);
        for (const auto i: m_fcdataset->m_trainindex)
        {
            for (int j = 0; j < numclass; ++j)
            {
                int index = j * numinstances + i;
				m_init_scores[index] = out_results[index];
			}
        }
        LGBM_BoosterGetPredict(booster, 1, &out_len3, out_results);
        for (const auto i : m_fcdataset->m_valindex)
        {
            for (int j = 0; j < numclass; ++j)
            {
                int index = j * numinstances + i;
				m_init_scores[index] = out_results[index];
			}
		}
        delete [] out_results;
        LGBM_BoosterFree(booster);
        LGBM_DatasetFree(train_data);
        LGBM_DatasetFree(valid_data);
        booster = nullptr;
        LOG(INFO) << "The init score of the model: " << m_bestscore;
    }

    DatasetHandle GBMFilter::getSubDataset(DatasetHandle dataset, const std::vector<int>& instanceindex)
    {
        if(dataset == nullptr)
            return nullptr;
        DatasetHandle subdataset;
        string parameters("");
        int flag = LGBM_DatasetGetSubset(dataset, instanceindex.data(), instanceindex.size(), parameters.c_str(), &subdataset);
        checkStatus(flag, "GBMFilter getSubDataset LGBM_DatasetGetSubset");
        return subdataset;
    }

    bool GBMFilter::isBetter(double score)
    {
        // for regression, we use rmse, the smaller the better
        // for binary classification, we use 1 - auc, the larger the better
        // for multi-class classification, we use multi_error, the smaller the better
        return score < m_bestscore;
           
    }

    void GBMFilter::produceScore(const vector<int>& instacesindex, const vector<int>& instacesindexval, vector<int>& featureindexs, vector<double>& scores)
    {
        scores.resize(featureindexs.size());
        int numclass = Property::getProperty()->getTargetClasses();
        omp_set_nested(1);
// #pragma omp parallel for num_threads(1)
        for(int i = 0;i < featureindexs.size();++ i)
        {
            if(i % 200 == 0)
                LOG(INFO) << "Start to produce score for the " << i <<"-th / "<< featureindexs.size() <<  " feature";
            if(i % 10 == 0)
                LOG(DEBUG) << "Start to produce score for the " << i <<"-th / "<< featureindexs.size() <<  " feature";
            int featureindex = featureindexs[i];
            
            auto dataset = generateLightGBMDatasetFromOne(featureindex, instacesindex, instacesindexval);
            
            auto &subtrain = dataset.first;
            auto &subvalid = dataset.second;
            auto booster = InitBooster(subtrain, subvalid);
            int is_finished = 0;
            int out_len = 0;
            LGBM_BoosterGetEvalCounts(booster, &out_len);

            double * out_results = new double[out_len];
            int earlystopcount = 0;
            const auto &lightgbmparams = Property::getProperty()->getLightGBMParams();
            int iterations = lightgbmparams.find("num_iterations") == lightgbmparams.end() ? 100 : std::stoi(lightgbmparams.find("num_iterations")->second);
            int earlystopround = lightgbmparams.find("early_stopping_round") == lightgbmparams.end() ? 3 : std::stoi(lightgbmparams.find("early_stopping_round")->second);
            double bestscore(std::numeric_limits<double>::max());
            double score = 0;
            for(int iter =0;iter < iterations;++ iter)
            {
                int flag = LGBM_BoosterUpdateOneIter(booster, &is_finished);
                checkStatus(flag, "GBMFilter produceScore BoosterUpdateOneIter");
                if (is_finished) break;
                if (subvalid != nullptr)
                    LGBM_BoosterGetEval(booster, 1, &out_len, out_results);
                else
                    LGBM_BoosterGetEval(booster, 0, &out_len, out_results);
                score = out_results[0];
                if(earlyStop(score, earlystopround, earlystopcount, bestscore))break;
            }
            scores[i] = bestscore;
            delete[] out_results;
            // LOG(INFO) << "END to produce score for the " << i << "-th / " << featureindexs.size() << " feature";
            LGBM_BoosterFree(booster);
            LGBM_DatasetFree(subtrain);
            LGBM_DatasetFree(subvalid);
        }
        LOG(INFO) << "Finish the produce score for the features: " << featureindexs.size();
        // sort the feature index by the score
        vector<std::pair<int, double>> temp;
        for(int i = 0;i < featureindexs.size();++ i)
            temp.push_back(std::make_pair(featureindexs[i], scores[i]));
        sort(temp.begin(), temp.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b){return a.second < b.second;});
        
        for(int i = 0;i < temp.size();++ i)
        {
            featureindexs[i] = temp[i].first;
            scores[i] = temp[i].second;
        }
    }

    void GBMFilter::removeFeatures(vector<int>& featureindexs, vector<double>& scores, bool isrepeat, int leftfeaturenum)
    {
        CHECK_EQ(featureindexs.size(), scores.size());
        if(isrepeat)
        {
            vector<int> tempindexs;
            vector<double> tempscores;
            vector<bool> isused(featureindexs.size(), false);
            for(int i = 0;i < featureindexs.size();++ i)
            {
                if(i > 0 && abs(scores[i]-scores[i-1]) < 1e-10) continue;
                tempindexs.push_back(featureindexs[i]);
                tempscores.push_back(scores[i]);
                isused[i] = true;            
            }
            if(leftfeaturenum > 0 && tempindexs.size() < leftfeaturenum)
            {
                for (int i = 0; i < featureindexs.size() && tempindexs.size() < leftfeaturenum; ++i)
                {
                    if(isused[i]) continue;
                    tempindexs.push_back(featureindexs[i]);
                    tempscores.push_back(scores[i]);
                    isused[i] = true;
                }
            }
            CHECK_GE(tempindexs.size(), leftfeaturenum);
            featureindexs = tempindexs;
            scores = tempscores;
        }
        else
        {
            // sort the feature index by the score
            vector<std::pair<int, double>> temp;
            for(int i = 0;i < featureindexs.size();++ i)
                temp.push_back(std::make_pair(featureindexs[i], scores[i]));
            sort(temp.begin(), temp.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b){return a.second > b.second;});
            for(int i = 0;i < temp.size();++ i)
            {
                featureindexs[i] = temp[i].first;
                scores[i] = temp[i].second;
            }
        }
    }

    void GBMFilter::produceScore(vector<int>& featureindexs, vector<double>& scores)
    {
        scores.resize(featureindexs.size());
        int numclass = Property::getProperty()->getTargetClasses();
        auto dataset = AddLightGBMDataset(featureindexs, 1);
        auto subtrain = getSubDataset(dataset, m_fcdataset->m_trainindex);
        auto subvalid = getSubDataset(dataset, m_fcdataset->m_valindex);
        LGBM_DatasetFree(dataset);
        auto booster = InitBooster(subtrain, subvalid);
        int is_finished = 0;
        int out_len = 0;
        LGBM_BoosterGetEvalCounts(booster, &out_len);
        double* out_results = new double[out_len];
        int earlystopcount = 0;
        double bestscore = (std::numeric_limits<double>::max());
        double score = 0;
        int bestiter = 0;
        for(int iter =0;iter < 1000;++ iter)
        {
            int flag = LGBM_BoosterUpdateOneIter(booster, &is_finished);
            checkStatus(flag, "GBMFilter produceScore BoosterUpdateOneIter");
            if (is_finished) break;
            if (subvalid != nullptr)
                LGBM_BoosterGetEval(booster, 1, &out_len, out_results);
            else
                LGBM_BoosterGetEval(booster, 0, &out_len, out_results);
            score = out_results[0];
            if (earlyStop(score, 50, earlystopcount, bestscore))break;
            if(abs(score - bestscore) < 1e-20)bestiter = iter + 1;
        }
        LOG(INFO) << "The best score of the model: " << bestscore;
        delete [] out_results;
        int featurenums;
        int orifeaturenums = m_fcdataset->getFeatureSize(false);
        LGBM_BoosterGetNumFeature(booster, &featurenums);
        out_results = new double[featurenums];
        auto importance_types = atoi(Property::getProperty()->getLightGBMParams().find("important_type")->second.c_str());
        int importance_type = importance_types & 1;
        int isnotbestiter_type = importance_types & 2;
        if(isnotbestiter_type)bestiter = 0;
        LOG(DEBUG) << "the importance type: " << importance_type << " the best iter: " << bestiter;
        LGBM_BoosterFeatureImportance(booster, bestiter, importance_type, out_results);
        CHECK_EQ(featurenums - orifeaturenums, featureindexs.size());
        memcpy(scores.data(), out_results + orifeaturenums, featureindexs.size() * sizeof(double));
        delete[] out_results;
        LGBM_BoosterFree(booster);
        LGBM_DatasetFree(subtrain);
        LGBM_DatasetFree(subvalid);
    }

    void GBMFilter::produceScore(int numsfeatures)
    {
        InitDataSet();
        vector<int> featureindexs;
        for(int i = 0;i < m_featurefcops.size(); ++i)
            featureindexs.push_back(i);
        vector<double> scores;
        int q = Property::getProperty()->getFlod();
        int featurenums = featureindexs.size();
        int totalinstancestrain = m_fcdataset->m_trainindex.size();
        int totalinstancesval = m_fcdataset->m_valindex.size();
        vector<int> instacesindex(m_fcdataset->m_trainindex);
        
        vector<int> instacesindexval(m_fcdataset->m_valindex);

        int calcimportancefeatures = Property::getProperty()->getMaxImportanceFeatures();
        if(calcimportancefeatures <= 0) calcimportancefeatures = MAX_GBM_IMPORTANCE;
        int prefeaturenums = featurenums;
        int importance_types = atoi(Property::getProperty()->getLightGBMParams().find("important_type")->second.c_str());
        int isnotimportance = importance_types & 4;
        int firstselect = 1;
        double ratio = 0.5;
        while( (featurenums > calcimportancefeatures || firstselect) && q > 0)
        {
            firstselect = 0;
            int rows = std::min(totalinstancestrain / q, totalinstancestrain);
            q >>= 1;
            vector<int> instacesindextraintemp(instacesindex.begin(), instacesindex.begin() + rows);
            std::sort(instacesindextraintemp.begin(), instacesindextraintemp.end());
            int rowsval = static_cast<int>((double)totalinstancesval / totalinstancestrain * rows);
            vector<int> instacesindexvaltemp(instacesindexval.begin(), instacesindexval.begin() + rowsval);
            std::sort(instacesindexvaltemp.begin(), instacesindexvaltemp.end());
            CHECK_EQ(featureindexs.size(), featurenums);

            LOG (INFO) << "Start to select features, the number of features: " << featureindexs.size() << " the number of instances: " << totalinstancestrain << " the number of rows: " << rows;
            q = max(q, 0);
            produceScore(instacesindextraintemp, instacesindexvaltemp, featureindexs, scores);
            
            removeFeatures(featureindexs, scores, true, std::min(static_cast<int>(featureindexs.size()), calcimportancefeatures));
            featurenums = featureindexs.size();
            if (featurenums < calcimportancefeatures || q == 0)break;
            if(prefeaturenums == featurenums) break;
            if (featurenums > calcimportancefeatures)
            {
				int leftfeaturenum = std::max(static_cast<int>(featurenums * ratio), calcimportancefeatures);
                featureindexs.resize(leftfeaturenum);
                scores.resize(leftfeaturenum);
				featurenums = featureindexs.size();
			}
            prefeaturenums = featurenums;
        }
        if(featureindexs.size() > calcimportancefeatures)
        {
            featureindexs.resize(calcimportancefeatures);
            scores.resize(calcimportancefeatures);
        }
        
        if(!isnotimportance)
        {
            produceScore(featureindexs, scores);
            removeFeatures(featureindexs, scores, false);
        }
        featureindexs.resize(numsfeatures);
        scores.resize(numsfeatures);
        // output scores
        // string scoresout = "seleted scores: ";
        // for(auto & score : scores) scoresout += std::to_string(score) + " ";
        LOG(INFO) << "seleted scores: " << scores;
        // scoresout = "seleted featurename: ";
        // for(auto & featureindex : featureindexs) scoresout += m_featurefcops[featureindex].first->getName() + ", ";
        LOG(INFO) << "seleted featurename: " << featureindexs;
        std::vector<std::pair<PFeatureInfo, PFCOperators>> newfeaturefcops;
        vector<bool > isused(m_featurefcops.size(), false);
        for(int i = 0;i < featureindexs.size();++ i)
        {
            newfeaturefcops.push_back(m_featurefcops[featureindexs[i]]);
            isused[featureindexs[i]] = true;
        }
        for (int i = 0; i < m_featurefcops.size(); ++i)
        {
            auto & featureinfo= m_featurefcops[i].first;
            if(!isused[i] && featureinfo != nullptr)
            {
                featureinfo->clear();
                delete featureinfo;
                featureinfo = nullptr;
            }
        }
        m_featurefcops = newfeaturefcops;
    }

    void GBMFilter::getParams(std::string &parameters)
    {
        auto numclass = Property::getProperty()->getTargetClasses();
        auto &lightgbmparams = Property::getProperty()->getLightGBMParams();
        if (numclass > 2)
            parameters += "objective=multiclass metric=multi_error num_class=" + std::to_string(numclass);
        else if(numclass == 2)
            parameters += "objective=binary metric=binary_logloss";
        else
            parameters += "objective=regression metric=rmse";
        for(auto & param : lightgbmparams)
        {
            if (param.first != "important_type")parameters += " " + param.first + "=" + param.second;
            if (param.first == "num_threads") LGBM_SetMaxThreads(std::stoi(param.second.c_str()));
            parameters += " " + param.first + "=" + param.second;
        }
    }

    BoosterHandle GBMFilter::InitBooster(DatasetHandle train_data, DatasetHandle valid_data)
    {
        BoosterHandle booster = nullptr;
        string parameters;
        getParams(parameters);
        int flag = LGBM_BoosterCreate(train_data, parameters.c_str(), &booster);    
        checkStatus(flag, "GBMFilter InitBooster BoosterCreate");
        if(valid_data != nullptr)
        {
            flag = LGBM_BoosterAddValidData(booster, valid_data);
            checkStatus(flag, "GBMFilter produceScore BoosterAddValidData");
        }
        return booster;
    }

    void GBMFilter::checkStatus(int flag, const std::string & message) {
        if (flag != 0) {
            LOG(ERROR) << message << " error : " << flag;
            exit(flag);
        }
    }

    void GBMFilter::clear()
    {
    }

    bool GBMFilter::earlyStop(double score, int earlystopround, int& earlystopcount, double& bestscore)
    {
        if (earlystopround <= 0)
            return false;
        if (bestscore - score > 1e-5)
        {
            earlystopcount = 0;
            bestscore = std::min(score, bestscore);
            return false;
        }
        else
        {
            earlystopcount++;
            if (earlystopcount >= earlystopround)
            {
                LOG(DEBUG) << "The early stop of the model, and the early stop round is: " << earlystopcount;
                return true;
            }
            return false;
        }
    }

}