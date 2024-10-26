#pragma once
#include <numeric>
#include "./DataSet.h"
#include "./util/DynamicBase.h"
#include "./util/MyMath.h"
namespace EaSFE {
    class FCOperator : public DynBase
    {
    protected:
        bool m_issecondfeature = false;
        int m_featureinstances = 0;
    public:
        friend ClearClass;
        virtual OutType getType() = 0;
        virtual OperatorType getOperatorType() = 0;
        virtual string getName() = 0;
        virtual int getNumClasses() = 0;
        virtual OutType requireType() = 0;
        void virtualfunc() { ; }
        virtual FCOperator* copy() = 0;
        virtual ~FCOperator() {};
        virtual void clear() = 0;
        virtual bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature) = 0;
        virtual void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0) = 0;
        virtual PFeatureInfo generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature) = 0;
    };

    class UnaryOperator : public FCOperator
    {
    public:
        friend ClearClass;
        virtual FCOperator* copy() ;
        virtual ~UnaryOperator() ;
        void clear();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        virtual bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        OperatorType getOperatorType();

    };

    class BinaryOperator : public FCOperator
    {
    public:
        friend ClearClass;
        virtual FCOperator* copy();
        virtual ~BinaryOperator();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        void clear();
        virtual bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        OperatorType getOperatorType();
        int getNumClasses();
        OutType requireType();
        string generateName(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };


    class GroupByTime : public FCOperator
    {
    protected:
        MyDataType m_timewindow{0};
        std::unordered_multimap<string, std::map<Date, MyDataType>>* m_keys{nullptr};
        std::unordered_map<string, bool>* m_haskeys{ nullptr };
    public:
        friend ClearClass;
        virtual FCOperator* copy() ;
        virtual ~GroupByTime();
        OutType getType();
        void setWindow(MyDataType timewindow);
        bool timeOverWindow(Date date1, Date date2);
        void init();
        void clear();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        virtual bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        OperatorType getOperatorType();
        int getNumClasses();
        OutType requireType();
        string generateName(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupByOpertor : public FCOperator
    {
    public:
        friend ClearClass;
        virtual FCOperator* copy() ;
        virtual ~GroupByOpertor() ;
        virtual void clear();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        virtual bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        OperatorType getOperatorType();
        int getNumClasses();
        OutType requireType();
        string generateName(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class StdOperator : public UnaryOperator
    {
    private:
        MyDataType m_std;
        MyDataType m_mean;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        FCOperator* copy();
        StdOperator();
        OutType getType();
        string getName();
        int getNumClasses();
        OutType requireType();
        static DynBase* CreateObject();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };


    class DiscretizerOperator : public UnaryOperator
    {
    private:
        int m_numclasses;
        MyDataType min_value;
        MyDataType max_value;
        MyDataType m_step;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        FCOperator* copy();
        DiscretizerOperator(int numclasses = Property::getProperty()->getDiscreteClass());
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        int getNumClasses();
        OutType requireType();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };


    class DayOfWeekOperator : public UnaryOperator
    {
    private:
        int m_numclasses;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        FCOperator* copy();
        DayOfWeekOperator();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        int getNumClasses();
        OutType requireType();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };


    class IsWeekendOperator : public UnaryOperator
    {
    private:
        int m_numclasses;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        FCOperator* copy();
        IsWeekendOperator(int numclasses = 2);
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        int getNumClasses();
        OutType requireType();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class CountOperator : public UnaryOperator
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        FCOperator* copy();
        CountOperator();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        int getNumClasses();
        OutType requireType();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class FloorOperator : public UnaryOperator
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        FCOperator* copy();
        FloorOperator();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        int getNumClasses();
        OutType requireType();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances);
        PFeatureInfo generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class LogOperator : public UnaryOperator
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        FCOperator *copy();
        LogOperator();
        static DynBase *CreateObject();
        OutType getType();
        string getName();
        int getNumClasses();
        OutType requireType();
        void preGenerateFeature(DataSet *&dataset, const std::vector<PFeatureInfo> &sourcefeature, const std::vector<PFeatureInfo> &targetfeature, bool issecondfeature, int featureinstances);
        PFeatureInfo generateFeature(DataSet *&dataset, const std::vector<PFeatureInfo> &sourcefeature, const std::vector<PFeatureInfo> &targetfeature);
        bool isMatch(const std::vector<PFeatureInfo> &sourcefeature, const std::vector<PFeatureInfo> &targetfeature);
    };
    
    class AddOperator : public BinaryOperator
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        AddOperator();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };


    class SubtractOperator : public BinaryOperator
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        SubtractOperator();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };


    class MultiplyOperator : public BinaryOperator
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        MultiplyOperator();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };


    class DivideOperator : public BinaryOperator
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        DivideOperator();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };


    class MaxOperator : public BinaryOperator
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        MaxOperator();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet *&dataset, const std::vector<PFeatureInfo> &sourcefeature, const std::vector<PFeatureInfo> &targetfeature, bool issecondfeature, int featureinstances);
        PFeatureInfo generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };


    class MinOperator : public BinaryOperator
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        MinOperator();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet *&dataset, const std::vector<PFeatureInfo> &sourcefeature, const std::vector<PFeatureInfo> &targetfeature, bool issecondfeature, int featureinstances);
        PFeatureInfo generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };


    class GroupByTimeCount : public GroupByTime
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByTimeCount();
        GroupByTimeCount(MyDataType time = 5);
        OutType getType();
        FCOperator* copy();
        static DynBase* CreateObject();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupByTimeMax : public GroupByTime
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByTimeMax();
        GroupByTimeMax(MyDataType time = 5);
        OutType getType();
        FCOperator* copy();
        static DynBase* CreateObject();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };
    
    class GroupByTimeMin : public GroupByTime
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByTimeMin();
        GroupByTimeMin(MyDataType time = 5);
        OutType getType();
        FCOperator* copy();
        static DynBase* CreateObject();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupByTimeSum : public GroupByTime
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByTimeSum();
        GroupByTimeSum(MyDataType time = 5);
        OutType getType();
        FCOperator* copy();
        static DynBase* CreateObject();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupByTimeMean : public GroupByTime
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByTimeMean();
        GroupByTimeMean(MyDataType time = 5);
        OutType getType();
        FCOperator* copy();
        static DynBase* CreateObject();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupByTimeStd : public GroupByTime
    {
    private:
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByTimeStd();
        GroupByTimeStd(MyDataType time = 5);
        OutType getType();
        FCOperator* copy();
        static DynBase* CreateObject();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };
    
    class GroupByCountOperator : public GroupByOpertor
    {
    private:
        std::unordered_map<string, MyDataType>* m_keys;
        //std::unordered_map<string, bool>* m_haskeys;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByCountOperator();
        GroupByCountOperator();
        void init();
        void clear();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupByMaxOperator : public GroupByOpertor
    {
    private:
        std::unordered_map<string, MyDataType>* m_keys;
        //std::unordered_map<string, bool>* m_haskeys;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByMaxOperator();
        GroupByMaxOperator();
        void init();
        void clear();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupByMinOperator : public GroupByOpertor
    {
    private:
        std::unordered_map<string, MyDataType>* m_keys;
        //std::unordered_map<string, bool>* m_haskeys;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByMinOperator();
        GroupByMinOperator();
        void init();
        void clear();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupBySumOperator : public GroupByOpertor
    {
    private:
        std::unordered_map<string, MyDataType>* m_keys;
        //std::unordered_map<string, bool>* m_haskeys;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupBySumOperator();
        GroupBySumOperator();
        void init();
        void clear();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupByStdOperator : public GroupByOpertor
    {
    private:
        std::unordered_map<string, MyDataType>* m_keys;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByStdOperator();
        GroupByStdOperator();
        void init();
        void clear();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupByMeanOperator : public GroupByOpertor
    {
    private:
        std::unordered_map<string, MyDataType>* m_keys;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        GroupByMeanOperator();
        ~GroupByMeanOperator();
        void init();
        void clear();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo  generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };

    class GroupByRankOperator : public GroupByOpertor
    {
    private:
        std::unordered_map<string, std::unordered_map<MyDataType, MyDataType>>* m_keys;
        static ClassInfo* m_cInfo;
    public:
        friend ClearClass;
        ~GroupByRankOperator();
        GroupByRankOperator();
        void init();
        void clear();
        FCOperator* copy();
        static DynBase* CreateObject();
        OutType getType();
        string getName();
        void preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature=false, int featureinstances=0);
        PFeatureInfo generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
        bool isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature);
    };
}//EaSFE
