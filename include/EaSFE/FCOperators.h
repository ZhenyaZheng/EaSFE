#pragma once
#include "./FCOperator.h"
namespace EaSFE {
    class FCOperators
    {
    public:
        FCOperators(std::vector<PFeatureInfo> sourcefeatures, std::vector<PFeatureInfo> targetfeatures, FCOperator* operator_, FCOperator* operator_second = nullptr, bool istargetclass = false);
        ~FCOperators();
        std::vector<PFeatureInfo> getSourceFeatures()const;
        std::vector<PFeatureInfo> getTargetFeatures()const;
        FCOperator* getOperator()const;
        FCOperator* getSecondOperator()const;
        bool getIsTargetClass()const;
        string getName()const;
        OutType getOutType()const;
        void setFScore(MyDataType score);
        MyDataType getFScore()const;
        void setWScore(MyDataType score);
        MyDataType getWScore()const;
        friend ostream& operator<< (ostream& out, const FCOperators*& fcos);
        void setHasCalc(int hascalc);
        int getHasCalc();
        string getSaveInfo()const;
        int getFCOperID()const;
        void setFCOperID(int fcoperid);
        void clear();
        bool operator==(const FCOperators & fcoperator)const;
        bool operator!=(const FCOperators & fcoperator)const;
    private:
        FCOperator* m_operator;
        FCOperator* m_secondoperator;
        std::vector<PFeatureInfo> m_sourcefeatures;
        std::vector<PFeatureInfo> m_targetfeatures;
        
        MyDataType m_fscore = 0.0;
        MyDataType m_wscore = 0.0;
        bool m_istargetclass;
        int m_hascalc = 0;// 01 filterhascalc 10 wrapperhascalc 11 bothhascalc 00 nonehascalc
        int m_fcoperid = 0;
    };
    using PFCOperators = FCOperators*;
}//EaSFE