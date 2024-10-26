#pragma once
#include "./FCOperators.h"
#include "./util/Combination.h"
#include "./util/StopWatch.h"
#include "./Filter.h"
#include <algorithm>
#include <atomic>
#include <charconv>
#include <omp.h>
//#include "./MemoryLeak.h"

namespace EaSFE {

#define ADDMEMORY 100
    void resetFCOperatorID(std::vector<PFCOperators>& pfcopers);
    void getDistributeAvg(std::vector<MyDataType>& allbestscores, bool isparallel=false, const PFCOperators& fcoper=nullptr);
	std::vector<std::vector<PFeatureInfo>>getFeatureCombination(DataSet* dataset, int i);
	bool getOperatorList(const std::vector<string>& opernames, std::vector<FCOperator*>& operatorlist);
	bool overlapexists(std::vector<PFeatureInfo>& sourcefeatures, std::vector<PFeatureInfo>& targetfeatures);
	std::vector<PFCOperators> getOperators(DataSet* dataset, int maxcombinations = 2, int opertype = 1, std::vector<PFeatureInfo> mustincludefeature = std::vector<PFeatureInfo>(), bool reducenumoffeature = false);
	PFeatureInfo generateFeature(DataSet* dataset, PFCOperators fcops, bool needPre=true, bool needgenerate=true, bool needfindindataset=true, bool filtersinglevec=false);
	bool addFeatureToDataset(DataSet* dataset, std::vector<PFCOperators> &operators, bool ismutilthread=false, bool isiter=true);
	char* initMyMemory(const int n);
	bool clearMyMemory(char*& ptr);
    string getNowTime();
    template<typename T>
    void autofcClear(T& vecs)
    {
        for (int i = 0; i < vecs.size(); ++i)
        {
            try {
                if (vecs[i])
                {
                    vecs[i]->clear();
                    delete vecs[i];
                    vecs[i] = nullptr;
                }
            }
            catch (exception& e)
            {
                LOG(WARNING) << "autofcClear error : " << e.what();
                continue;
            }
        }
        vecs.clear();
    }
	void GetFileNames(string path, std::vector<string>& filenames);
    void clearFeatureData(DataSet* dataset, std::vector<PFeatureInfo>& featureinfos);
    bool addFeatureToDataset(DataSet* dataset, std::vector<std::pair<PFeatureInfo, PFCOperators>>& featureinfos, bool ismutilthread = false);
	FCOperators* copyFCOperators(DataSet* dataset, FCOperators* fcos);
    
    class ClearClass;
    class globalVar
    {
        class FCOperID
        {
        public:
            string m_firstname;
            string m_secondname;
            std::vector<string> m_sourcename;
            std::vector<string> m_targetname;
        
            bool operator==(const FCOperID& fcoperid)
            {
                if (m_firstname == fcoperid.m_firstname && m_secondname == fcoperid.m_secondname && m_sourcename == fcoperid.m_sourcename && m_targetname == fcoperid.m_targetname)
                    return true;
                return false;
            }
            bool operator!=(const FCOperID& fcoperid) { return !(*this == fcoperid); }
            FCOperID() {}
            FCOperID(const string& name1, const string& name2, const std::vector<string>& name3, const std::vector<string>& name4) :m_firstname(name1), m_secondname(name2), m_sourcename(name3), m_targetname(name4) {}
        };
    public:
        friend ClearClass;
        std::shared_timed_mutex* getMutex();
        bool getIsReceive();
        void setIsReceive(bool isreceive);
        int getFCOperID();
        void setFCOperID(int fcoperid);
        std::shared_timed_mutex* getMutex2();
        bool getIsReceive2();
        void setIsReceive2(bool isreceive);
        int getFCOperID2();
        void setFCOperID2(int fcoperid);
        static globalVar* getglobalVar();
        static bool freeglobalVar();
        ~globalVar() { delete m_rwmutex; delete m_rwmutex2; m_rwmutex = nullptr; m_rwmutex2 = nullptr; }
    private:
        static globalVar* m_instance;
        std::shared_timed_mutex* volatile m_rwmutex = nullptr;
        int m_fcoperid = 0;
        bool m_isreceive = false;
        std::shared_timed_mutex* volatile m_rwmutex2 = nullptr;
        int m_fcoperid2 = 0;
        bool m_isreceive2 = false;
    };

}//EaSFE