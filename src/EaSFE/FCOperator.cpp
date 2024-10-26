#include "EaSFE/FCOperator.h"
#include "FCOperator.h"
namespace EaSFE {
    
    FCOperator* UnaryOperator::copy() { return nullptr; };
    UnaryOperator::~UnaryOperator() {};
    void UnaryOperator::clear() {}
    void UnaryOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances) 
    {
        m_issecondfeature = issecondfeature; m_featureinstances = featureinstances;
    }
    bool UnaryOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (sourcefeature.size() != 1 || targetfeature.size() != 0)return false;
        return true;
    }
    OperatorType UnaryOperator::getOperatorType() { return OperatorType::Unary; }

   

    
    FCOperator* BinaryOperator::copy() { return nullptr; };
    BinaryOperator::~BinaryOperator() {};
    void BinaryOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances) 
    {
        m_issecondfeature = issecondfeature; m_featureinstances = featureinstances;
    }
    void BinaryOperator::clear() {}
    bool BinaryOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (sourcefeature.size() != 1 || targetfeature.size() != 1)return false;
        if (sourcefeature[0]->getType() != OutType::Numeric || targetfeature[0]->getType() != OutType::Numeric)
            return false;
        return true;
    }
    OperatorType BinaryOperator::getOperatorType() { return OperatorType::Binary; }
    int BinaryOperator::getNumClasses() { return -1; }
    OutType BinaryOperator::requireType() { return  OutType::Numeric; }
    string BinaryOperator::generateName(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        string name = "";
        name += "{sources:[";
        for (auto& i : sourcefeature)
        {
            name += i->getName() + ",";
        }
        name += "];targets:[";
        for (auto& i : targetfeature)
        {
            name += i->getName() + ",";
        }
        name += "];}";

        return name;
    }
   


    FCOperator* GroupByTime::copy() { return nullptr; };
    GroupByTime::~GroupByTime() {}
    OutType GroupByTime::getType() { return OutType::Numeric; }
    void GroupByTime::setWindow(MyDataType timewindow) {m_timewindow = timewindow ; }
    bool GroupByTime::timeOverWindow(Date date1, Date date2)
    {
        int sec = date1.getDiffSecond(date2);
        if (m_timewindow * 60 < sec)
            return true;
        return false;
    }
    void GroupByTime::init()
    {
        if (m_keys == nullptr)
            m_keys = new std::unordered_multimap<string, std::map<Date, MyDataType>>;
        if (m_haskeys == nullptr) m_haskeys = new std::unordered_map<string, bool>;
    }
    void GroupByTime::clear()
    {
        if (m_keys != nullptr)
            m_keys->clear();
        if (m_haskeys != nullptr) m_haskeys->clear();
        delete m_keys;
        delete m_haskeys;
        m_keys = nullptr;
        m_haskeys = nullptr;
    }
    void GroupByTime::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        init();
        m_issecondfeature = issecondfeature; m_featureinstances = featureinstances;
        std::vector<PFeatureInfo> discretefeatrues;
        PFeatureInfo datefeature = nullptr;
        for (int i = 0; i < sourcefeature.size(); ++i)
        {
            if (sourcefeature[i]->getType() == OutType::Discrete)
                discretefeatrues.push_back(sourcefeature[i]);
            else datefeature = sourcefeature[i];
        }
        PFeatureInfo featureinfo = nullptr;
        Feature* feature = nullptr;
        if (datefeature)
            feature = datefeature->getFeature();
        int n = m_featureinstances;
        string key = "";
        auto dateindexs = reinterpret_cast<DateFeature*>(feature)->getDateKey();
        auto targetfea = targetfeature[0]->getFeature();
        for (auto& dateindex : *dateindexs)
        {
            auto date = dateindex.first;
            auto indexs = dateindex.second;
            for (int j = 0; j < indexs.size(); ++j)
            {
                auto index = indexs[j];
                for (int i = 0; i < discretefeatrues.size(); i++)
                {
                    featureinfo = discretefeatrues[i];
                    feature = featureinfo->getFeature();
                    key += std::to_string(*reinterpret_cast<int*> (feature->getValue(index)));
                }
                MyDataType val = *reinterpret_cast<MyDataType*>(targetfea->getValue(index));
                if (!(*m_haskeys)[key])
                {
                    m_keys->insert({ key, std::map<Date, MyDataType>({{date, val}}) });
                    (*m_haskeys)[key] = 1;
                }
                else
                {
                    auto iterkeys = m_keys->equal_range(key);
                    bool insertflag = 0;
                    for (auto iter = iterkeys.first; iter != iterkeys.second; ++iter)
                    {
                        auto firstdate = iter->second.begin()->first;
                        if (timeOverWindow(date, firstdate))continue;
                        else iter->second.emplace(date, val), insertflag = 1;
                    }
                    if (!insertflag)
                        m_keys->insert({ key, std::map<Date, MyDataType>({{date, val}}) });
                }
                key.clear();
            }
        }

    }
    bool GroupByTime::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (sourcefeature.size() < 2 || targetfeature.size() != 1)return false;
        if (targetfeature[0]->getType() != OutType::Numeric)return false;
        int datefeatures = 0;
        for (int i = 0; i < sourcefeature.size(); i++)
        {
            if (sourcefeature[i]->getType() != OutType::Discrete && sourcefeature[i]->getType() != OutType::Date)return false;
            if (sourcefeature[i]->getType() == OutType::Date) datefeatures++;
        }
        if (datefeatures != 1)return false;
        return true;
    }
    OperatorType GroupByTime::getOperatorType() { return OperatorType::GroupByTime; }
    int GroupByTime::getNumClasses() { return -1; }
    OutType GroupByTime::requireType() { return  OutType::Numeric; }
    string GroupByTime::generateName(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        string name = "";
        name += "{sources:[";
        for (auto& i : sourcefeature)
        {
            name += i->getName() + "&";
        }
        name += "];targets:[";
        for (auto& i : targetfeature)
        {
            name += i->getName() + "&";
        }
        name += "];}";

        return name;
    }
    

    
    FCOperator* GroupByOpertor::copy() { return nullptr; };
    GroupByOpertor::~GroupByOpertor() {};
    void GroupByOpertor::clear() {}
    void GroupByOpertor::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances) 
    {
        m_issecondfeature = issecondfeature; m_featureinstances = featureinstances;
    }
    bool GroupByOpertor::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (sourcefeature.size() < 1 || targetfeature.size() != 1)return false;
        if (targetfeature[0]->getType() != OutType::Numeric)return false;
        for (int i = 0; i < sourcefeature.size(); i++)
        {
            if (sourcefeature[i]->getType() != OutType::Discrete )return false;
        }
        return true;
    }
    OperatorType GroupByOpertor::getOperatorType() { return OperatorType::GroupBy; }
    int GroupByOpertor::getNumClasses() { return -1; }
    OutType GroupByOpertor::requireType() { return  OutType::Numeric; }
    string GroupByOpertor::generateName(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        string name = "";
        name += "{sources:[";
        for (auto& i : sourcefeature)
        {
            name += i->getName() + "&";
        }
        name += "];targets:[";
        for (auto& i : targetfeature)
        {
            name += i->getName() + "&";
        }
        name += "];}";

        return name;
    }


   
    FCOperator* StdOperator::copy() { return new StdOperator(); }
    StdOperator::StdOperator() :m_std(1.0), m_mean(0.0) {}
    OutType StdOperator::getType() { return OutType::Numeric; }
    string StdOperator::getName() { return "StdOperator"; }
    int StdOperator::getNumClasses() { return -1; }
    OutType StdOperator::requireType() { return OutType::Numeric; }
    DynBase* StdOperator::CreateObject() { return new StdOperator(); }
    void StdOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        UnaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int threads = Property::getProperty()->getThreadNum();
        int reallength = 0;
        m_mean = MyMath<MyDataType>::parallelMean(reinterpret_cast<MyDataType*> (feature->getValues()), featureinstances, threads, reallength);
        if (isnan(m_mean) || !isfinite(m_mean))
        {
            LOG(WARNING) << "isnan or  !isfinite";
        }
        
        m_std = MyMath<MyDataType>::parallelStd(reinterpret_cast<MyDataType*> (feature->getValues()), featureinstances, threads);
        if (isnan(m_std) || !isfinite(m_std))
        {
            LOG(WARNING) << "isnan or  !isfinite";
        }
    }
    PFeatureInfo StdOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        int threads = Property::getProperty()->getThreadNum();
        Feature* newfeature = new NumericFeature(N);
        
        MyMath<MyDataType>::parallelsetStd(reinterpret_cast<MyDataType*> (feature->getValues()), reinterpret_cast<MyDataType*> (newfeature->getValues()), m_mean, m_std, n, threads);
        string name = "StdOperator(";
        name += featureinfo->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool StdOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!UnaryOperator::isMatch(sourcefeature, targetfeature))return false;
        if (sourcefeature[0]->getType() != OutType::Numeric)return false;
        return true;
    }
  


    
    FCOperator* DiscretizerOperator::copy() { return new DiscretizerOperator(this->m_numclasses); }
    DiscretizerOperator::DiscretizerOperator(int numclasses) :m_numclasses(numclasses), min_value(0.0), max_value(0.0), m_step(0.0) {}
    DynBase* DiscretizerOperator::CreateObject() { return new DiscretizerOperator(Property::getProperty()->getDiscreteClass()); }
    OutType DiscretizerOperator::getType() { return OutType::Discrete; }
    string DiscretizerOperator::getName() { return "DiscretizerOperator"; }
    int DiscretizerOperator::getNumClasses() { return m_numclasses; }
    OutType DiscretizerOperator::requireType() { return OutType::Numeric; }
    void DiscretizerOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        UnaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
        if (m_numclasses > featureinstances / 2) m_numclasses = featureinstances / 2;
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int threads = Property::getProperty()->getThreadNum();
        min_value = MyMath<MyDataType>::parallelMin(reinterpret_cast<MyDataType*> (feature->getValues()), featureinstances, threads);
        max_value = MyMath<MyDataType>::parallelMax(reinterpret_cast<MyDataType*> (feature->getValues()), featureinstances, threads);
        if (min_value == max_value)
			m_step = 0.0;
		else if (isnan(min_value) || isnan(max_value) || !isfinite(min_value) || !isfinite(max_value))
            m_step = 0.0;
        m_step = (max_value - min_value) / m_numclasses;
    }
    PFeatureInfo DiscretizerOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances(); // real feature instances may be different from N due to chunking, but should set new feature instances as N
        Feature* newfeature = new DiscreteFeature(N);
        if (m_step != 0.0)
        {
            int threads = Property::getProperty()->getThreadNum();
            MyMath<MyDataType>::parallelsetDiscretizer(reinterpret_cast<MyDataType*> (feature->getValues()), reinterpret_cast<int*> (newfeature->getValues()), min_value, max_value, m_step, n, threads);
        }
        
        string name = "DiscretizerOperator(";
        name += featureinfo->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Discrete, name, m_issecondfeature);
    }
    bool DiscretizerOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!UnaryOperator::isMatch(sourcefeature, targetfeature))return false;
        if (sourcefeature[0]->getType() != OutType::Numeric)return false;
        return true;
    }


    
    FCOperator* DayOfWeekOperator::copy() { return new DayOfWeekOperator(); }
    DayOfWeekOperator::DayOfWeekOperator() :m_numclasses(7) {}
    DynBase* DayOfWeekOperator::CreateObject() { return new DayOfWeekOperator(); }
    OutType DayOfWeekOperator::getType() { return OutType::Discrete; }
    string DayOfWeekOperator::getName() { return "DayOfWeekOperator"; }
    int DayOfWeekOperator::getNumClasses() { return m_numclasses; }
    OutType DayOfWeekOperator::requireType() { return OutType::Date; }
    void DayOfWeekOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        UnaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo DayOfWeekOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new DiscreteFeature(N);
        int* valp2 = new int;
        for (int i = 0; i < n; i++)
        {
            Date* valp = reinterpret_cast<Date*> (feature->getValue(i));

            *valp2 = valp->getDayOfWeek();
            newfeature->setValue(i, reinterpret_cast<void*>(valp2));

            valp = nullptr;
        }
        delete valp2;
        string name = "DayOfWeekOperator(";
        name += featureinfo->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Discrete, name, m_issecondfeature);
    }
    bool DayOfWeekOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!UnaryOperator::isMatch(sourcefeature, targetfeature))return false;
        if (sourcefeature[0]->getType() != OutType::Date)return false;
        return true;
    }


   
    FCOperator* IsWeekendOperator::copy() { return new IsWeekendOperator(); }
    IsWeekendOperator::IsWeekendOperator(int numclasses) { m_numclasses = numclasses; }
    DynBase* IsWeekendOperator::CreateObject() { return new IsWeekendOperator(); }
    OutType IsWeekendOperator::getType() { return OutType::Discrete; }
    string IsWeekendOperator::getName() { return "IsWeekendOperator"; }
    int IsWeekendOperator::getNumClasses() { return m_numclasses; }
    OutType IsWeekendOperator::requireType() { return OutType::Date; }
    void IsWeekendOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        UnaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo IsWeekendOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new DiscreteFeature(N);
        int* valp2 = new int;
        for (int i = 0; i < n; i++)
        {
            Date* valp = reinterpret_cast<Date*> (feature->getValue(i));

            *valp2 = valp->isWeekend();
            newfeature->setValue(i, reinterpret_cast<void*>(valp2));

            valp = nullptr;
        }
        delete valp2;
        string name = "IsWeekendOperator(";
        name += featureinfo->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Discrete, name, m_issecondfeature);
    }
    bool IsWeekendOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!UnaryOperator::isMatch(sourcefeature, targetfeature))return false;
        if (sourcefeature[0]->getType() != OutType::Date)return false;
        return true;
    }



    FCOperator* CountOperator::copy() { return new CountOperator(); }
    CountOperator::CountOperator(){}
    DynBase* CountOperator::CreateObject() { return new CountOperator(); }
    OutType CountOperator::getType() { return OutType::Numeric; }
    string CountOperator::getName() { return "CountOperator"; }
    int CountOperator::getNumClasses() { return -1; }
    OutType CountOperator::requireType() { return OutType::Discrete; }
    void CountOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        UnaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo CountOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        string key;
        std::unordered_map<string, MyDataType> keyscount;
        for (int j = 0; j < n; j++)
        {
            if(featureinfo->getType() == OutType::Discrete)
                key = std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            else if (featureinfo->getType() == OutType::Numeric)
                key = std::to_string(*reinterpret_cast<MyDataType*> (feature->getValue(j)));
            if (keyscount.find(key) == keyscount.end())
                keyscount.insert({ key, 1 });
            else
                keyscount.at(key)++;
        }
        int threads = Property::getProperty()->getThreadNum();
        auto* pkeyscount = &keyscount;
        MyMath<MyDataType>::parallelsetGroupValue(reinterpret_cast<MyDataType*> (newfeature->getValues()),sourcefeature, pkeyscount, n, threads, 0);
        string name = "CountOperator(";
        name += featureinfo->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool CountOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!UnaryOperator::isMatch(sourcefeature, targetfeature))return false;
        if (sourcefeature[0]->getType() != OutType::Discrete && sourcefeature[0]->getType() != OutType::Numeric)return false;
        return true;
    }


    FCOperator* FloorOperator::copy() { return new FloorOperator(); }
    FloorOperator::FloorOperator() {}
    DynBase* FloorOperator::CreateObject() { return new FloorOperator(); }
    OutType FloorOperator::getType() { return OutType::Numeric; }
    string FloorOperator::getName() { return "FloorOperator"; }
    int FloorOperator::getNumClasses() { return -1; }
    OutType FloorOperator::requireType() { return OutType::Numeric; }
    void FloorOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        UnaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo FloorOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        for (int i = 0; i < n; i++)
        {
            MyDataType* valp = reinterpret_cast<MyDataType*> (feature->getValue(i));
            MyDataType val = floor(*valp);
            newfeature->setValue(i, reinterpret_cast<void*>(&val));
        }
        string name = "FloorOperator(";
        name += featureinfo->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool FloorOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!UnaryOperator::isMatch(sourcefeature, targetfeature))return false;
        if (sourcefeature[0]->getType() != OutType::Numeric)return false;
        return true;
    }



    FCOperator* LogOperator::copy() { return new LogOperator(); }
    LogOperator::LogOperator() {}
    DynBase* LogOperator::CreateObject() { return new LogOperator(); }
    OutType LogOperator::getType() { return OutType::Numeric; }
    string LogOperator::getName() { return "LogOperator"; }
    int LogOperator::getNumClasses() { return -1; }
    OutType LogOperator::requireType() { return OutType::Numeric; }
    void LogOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        UnaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo LogOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        for (int i = 0; i < n; i++)
        {
            MyDataType* valp = reinterpret_cast<MyDataType*> (feature->getValue(i));
            MyDataType val = log(*valp);
            if(isnan(val) || !isfinite(val)) val = 0.0;
            newfeature->setValue(i, reinterpret_cast<void*>(&val));
        }
        string name = "LogOperator(";
        name += featureinfo->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool LogOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!UnaryOperator::isMatch(sourcefeature, targetfeature))return false;
        if (sourcefeature[0]->getType() != OutType::Numeric)return false;
        return true;
    }
    


    AddOperator::AddOperator() {}
    FCOperator* AddOperator::copy() { return new AddOperator(); }
    DynBase* AddOperator::CreateObject() { return new AddOperator(); }
    OutType AddOperator::getType() { return OutType::Numeric; }
    string AddOperator::getName() { return "AddOperator"; }
    void AddOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        BinaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo AddOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo1 = sourcefeature[0];
        PFeatureInfo featureinfo2 = targetfeature[0];
        Feature* feature1 = featureinfo1->getFeature();
        Feature* feature2 = featureinfo2->getFeature();
        int n = m_featureinstances;
        int N = feature1->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        MyMath<MyDataType>::parallelsetAdd(reinterpret_cast<MyDataType*> (feature1->getValues()), reinterpret_cast<MyDataType*> (feature2->getValues()), reinterpret_cast<MyDataType*> (newfeature->getValues()), n, threads);
        string name = "AddOperator(";
        name += featureinfo1->getName() + "&" + featureinfo2->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool AddOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!BinaryOperator::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }


   
    SubtractOperator::SubtractOperator() {}
    FCOperator* SubtractOperator::copy() { return new SubtractOperator(); }
    DynBase* SubtractOperator::CreateObject() { return new SubtractOperator(); }
    OutType SubtractOperator::getType() { return OutType::Numeric; }
    string SubtractOperator::getName() { return "SubtractOperator"; }
    void SubtractOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        BinaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo SubtractOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo1 = sourcefeature[0];
        PFeatureInfo featureinfo2 = targetfeature[0];
        Feature* feature1 = featureinfo1->getFeature();
        Feature* feature2 = featureinfo2->getFeature();
        int N = feature1->getInstances();
        int n = m_featureinstances;
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        MyMath<MyDataType>::parallelsetSubtract(reinterpret_cast<MyDataType*> (feature1->getValues()), reinterpret_cast<MyDataType*> (feature2->getValues()), reinterpret_cast<MyDataType*> (newfeature->getValues()), n, threads);
        string name = "SubtractOperator(";
        name += featureinfo1->getName() + "&" + featureinfo2->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool SubtractOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!BinaryOperator::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }


    
    MultiplyOperator::MultiplyOperator() {}
    FCOperator* MultiplyOperator::copy() { return new MultiplyOperator(); }
    DynBase* MultiplyOperator::CreateObject() { return new MultiplyOperator(); }
    OutType MultiplyOperator::getType() { return OutType::Numeric; }
    string MultiplyOperator::getName() { return "MultiplyOperator"; }
    void MultiplyOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        BinaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo MultiplyOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo1 = sourcefeature[0];
        PFeatureInfo featureinfo2 = targetfeature[0];
        Feature* feature1 = featureinfo1->getFeature();
        Feature* feature2 = featureinfo2->getFeature();
        int n = m_featureinstances;
        int N = feature1->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        MyMath<MyDataType>::parallelsetMultiply(reinterpret_cast<MyDataType*> (feature1->getValues()), reinterpret_cast<MyDataType*> (feature2->getValues()), reinterpret_cast<MyDataType*> (newfeature->getValues()), n, threads);
        string name = "MultiplyOperator(";
        name += featureinfo1->getName() + "&" + featureinfo2->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool MultiplyOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!BinaryOperator::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }


    
    DivideOperator::DivideOperator() {}
    FCOperator* DivideOperator::copy() { return new DivideOperator(); }
    DynBase* DivideOperator::CreateObject() { return new DivideOperator(); }
    OutType DivideOperator::getType() { return OutType::Numeric; }
    string DivideOperator::getName() { return "DivideOperator"; }
    void DivideOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        BinaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo DivideOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo1 = sourcefeature[0];
        PFeatureInfo featureinfo2 = targetfeature[0];
        Feature* feature1 = featureinfo1->getFeature();
        Feature* feature2 = featureinfo2->getFeature();
        int n = m_featureinstances;
        int N = feature1->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        MyMath<MyDataType>::parallelsetDivide(reinterpret_cast<MyDataType*> (feature1->getValues()), reinterpret_cast<MyDataType*> (feature2->getValues()), reinterpret_cast<MyDataType*> (newfeature->getValues()), n, threads);
        string name = "DivideOperator(";
        name += featureinfo1->getName() + "&" + featureinfo2->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool DivideOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!BinaryOperator::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }



    FCOperator* MaxOperator::copy() { return new MaxOperator(); }
    MaxOperator::MaxOperator() {}
    DynBase* MaxOperator::CreateObject() { return new MaxOperator(); }
    OutType MaxOperator::getType() { return OutType::Numeric; }
    string MaxOperator::getName() { return "MaxOperator"; }
    void MaxOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        BinaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo MaxOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo1 = sourcefeature[0];
        PFeatureInfo featureinfo2 = targetfeature[0];
        Feature* feature1 = featureinfo1->getFeature();
        Feature* feature2 = featureinfo2->getFeature();
        int n = m_featureinstances;
        int N = feature1->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        MyDataType missval = std::numeric_limits<MyDataType>::min();
        MyMath<MyDataType>::parallelSetMaxMin(reinterpret_cast<MyDataType*> (feature1->getValues()), reinterpret_cast<MyDataType*> (feature2->getValues()), reinterpret_cast<MyDataType*> (newfeature->getValues()), n, threads, missval, true);
        string name = "MaxOperator(";
        name += featureinfo1->getName() + "&" + featureinfo2->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool MaxOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!BinaryOperator::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }



    FCOperator* MinOperator::copy() { return new MinOperator(); }
    MinOperator::MinOperator() {}
    DynBase* MinOperator::CreateObject() { return new MinOperator(); }
    OutType MinOperator::getType() { return OutType::Numeric; }
    string MinOperator::getName() { return "MinOperator"; }
    void MinOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        BinaryOperator::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo MinOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo1 = sourcefeature[0];
        PFeatureInfo featureinfo2 = targetfeature[0];
        Feature* feature1 = featureinfo1->getFeature();
        Feature* feature2 = featureinfo2->getFeature();
        int n = m_featureinstances;
        int N = feature1->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        MyDataType missval = std::numeric_limits<MyDataType>::max();
        MyMath<MyDataType>::parallelSetMaxMin(reinterpret_cast<MyDataType*> (feature1->getValues()), reinterpret_cast<MyDataType*> (feature2->getValues()), reinterpret_cast<MyDataType*> (newfeature->getValues()), n, threads, missval, false);
        string name = "MinOperator(";
        name += featureinfo1->getName() + "&" + featureinfo2->getName() + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool MinOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!BinaryOperator::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }
   
    OutType GroupByTimeCount::getType() { return OutType::Numeric; }
    GroupByTimeCount::~GroupByTimeCount() { clear(); }
    FCOperator* GroupByTimeCount::copy() { return new GroupByTimeCount(m_timewindow); }
    GroupByTimeCount::GroupByTimeCount(MyDataType time ) {
        GroupByTime::m_keys = nullptr;
        GroupByTime::m_haskeys = nullptr;
        GroupByTime::m_timewindow = time;
        init();
    }
    DynBase* GroupByTimeCount::CreateObject() { return new GroupByTimeCount(); }
    string GroupByTimeCount::getName() { return "GroupByTimeCount"; }
    void GroupByTimeCount::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        GroupByTime::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo GroupByTimeCount::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        std::vector<PFeatureInfo> discretefeatrues;
        PFeatureInfo datefeature = nullptr;
        for (int i = 0; i < sourcefeature.size(); ++i)
        {
            if (sourcefeature[i]->getType() == OutType::Discrete)
                discretefeatrues.push_back(sourcefeature[i]);
            else datefeature = sourcefeature[i];
        }
        PFeatureInfo featureinfo = nullptr;
        Feature* feature = nullptr;
        if (datefeature)
            feature = datefeature->getFeature();
        else
        {
            LOG(ERROR) << "GroupByTimeCount generateFeature Error: no DateFeature!";
            return new FeatureInfo();
        }
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        string key;
        MyDataType* valp = new MyDataType;
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < discretefeatrues.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            auto date = *reinterpret_cast<Date*>(datefeature->getFeature()->getValue(j));
            auto iterkeys = m_keys->equal_range(key);
            bool findflag = 0;
            for (auto iter = iterkeys.first; iter != iterkeys.second; ++iter)
            {
                auto firstdate = iter->second.begin()->first;
                if (timeOverWindow(date, firstdate))continue;
                else *valp = static_cast<MyDataType>(iter->second.size()),findflag = 1;
            }
            if (!findflag)*valp = 0;
            newfeature->setValue(j, reinterpret_cast<void*>(valp));
            key.clear();
        }
        delete valp;

        clear();
        string name = "GroupByTimeCount_" + std::to_string(m_timewindow) + "(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByTimeCount::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByTime::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }



    OutType GroupByTimeMax::getType() { return OutType::Numeric; }
    GroupByTimeMax::~GroupByTimeMax() { clear(); }
    FCOperator* GroupByTimeMax::copy() { return new GroupByTimeMax(m_timewindow); }
    GroupByTimeMax::GroupByTimeMax(MyDataType time) {
        GroupByTime::m_keys = nullptr;
        GroupByTime::m_haskeys = nullptr;
        GroupByTime::m_timewindow = time;
        init();
    }
    DynBase* GroupByTimeMax::CreateObject() { return new GroupByTimeCount(); }
    string GroupByTimeMax::getName() { return "GroupByTimeMax"; }
    void GroupByTimeMax::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        GroupByTime::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo GroupByTimeMax::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        std::vector<PFeatureInfo> discretefeatrues;
        PFeatureInfo datefeature = nullptr;
        for (int i = 0; i < sourcefeature.size(); ++i)
        {
            if (sourcefeature[i]->getType() == OutType::Discrete)
                discretefeatrues.push_back(sourcefeature[i]);
            else datefeature = sourcefeature[i];
        }
        PFeatureInfo featureinfo = nullptr;
        Feature* feature = nullptr;
        if (datefeature)
            feature = datefeature->getFeature();
        else
        {
            LOG(ERROR) << "GroupByTimeMax generateFeature Error: no DateFeature!";
            return new FeatureInfo();
        }
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        string key;
        MyDataType* valp = new MyDataType;
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < discretefeatrues.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            auto date = *reinterpret_cast<Date*>(datefeature->getFeature()->getValue(j));
            auto iterkeys = m_keys->equal_range(key);
            bool findflag = 0;
            for (auto iter = iterkeys.first; iter != iterkeys.second; ++iter)
            {
                auto mapval = iter->second;
                auto firstdate = mapval.begin()->first;
                if (timeOverWindow(date, firstdate))continue;
                else *valp = static_cast<MyDataType>((max_element(mapval.begin(), mapval.end(), [&](const std::pair<Date, MyDataType>& a, const std::pair<Date, MyDataType>& b) {return a.second < b.second; }))->second), findflag = 1;
            }
            if (!findflag)*valp = 0;
            newfeature->setValue(j, reinterpret_cast<void*>(valp));
            key.clear();
        }
        delete valp;

        clear();
        string name = "GroupByTimeMax_" + std::to_string(m_timewindow) + "(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByTimeMax::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByTime::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }
    
    
    OutType GroupByTimeMin::getType() { return OutType::Numeric; }
    GroupByTimeMin::~GroupByTimeMin() { clear(); }
    FCOperator* GroupByTimeMin::copy() { return new GroupByTimeMin(m_timewindow); }
    GroupByTimeMin::GroupByTimeMin(MyDataType time) {
        GroupByTime::m_keys = nullptr;
        GroupByTime::m_haskeys = nullptr;
        GroupByTime::m_timewindow = time;
        init();
    }
    DynBase* GroupByTimeMin::CreateObject() { return new GroupByTimeMin(); }
    string GroupByTimeMin::getName() { return "GroupByTimeMin"; }
    void GroupByTimeMin::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        GroupByTime::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo GroupByTimeMin::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        std::vector<PFeatureInfo> discretefeatrues;
        PFeatureInfo datefeature = nullptr;
        for (int i = 0; i < sourcefeature.size(); ++i)
        {
            if (sourcefeature[i]->getType() == OutType::Discrete)
                discretefeatrues.push_back(sourcefeature[i]);
            else datefeature = sourcefeature[i];
        }
        PFeatureInfo featureinfo = nullptr;
        Feature* feature = nullptr;
        if (datefeature)
            feature = datefeature->getFeature();
        else
        {
            LOG(ERROR) << "GroupByTimeMin generateFeature Error: no DateFeature!";

            return new FeatureInfo();
        }
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        string key;
        MyDataType* valp = new MyDataType;
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < discretefeatrues.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            auto date = *reinterpret_cast<Date*>(datefeature->getFeature()->getValue(j));
            auto iterkeys = m_keys->equal_range(key);
            bool findflag = 0;
            for (auto iter = iterkeys.first; iter != iterkeys.second; ++iter)
            {
                auto mapval = iter->second;
                auto firstdate = mapval.begin()->first;
                if (timeOverWindow(date, firstdate))continue;
                else *valp = static_cast<MyDataType>((min_element(mapval.begin(), mapval.end(), [&](const std::pair<Date, MyDataType>& a, const std::pair<Date, MyDataType>& b) {return a.second < b.second; }))->second), findflag = 1;
            }
            if (!findflag)*valp = 0;
            newfeature->setValue(j, reinterpret_cast<void*>(valp));
            key.clear();
        }
        delete valp;

        clear();
        string name = "GroupByTimeMin_" + std::to_string(m_timewindow) + "(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByTimeMin::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByTime::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }

   
    OutType GroupByTimeSum::getType() { return OutType::Numeric; }
    GroupByTimeSum::~GroupByTimeSum() { clear(); }
    FCOperator* GroupByTimeSum::copy() { return new GroupByTimeSum(m_timewindow); }
    GroupByTimeSum::GroupByTimeSum(MyDataType time) {
        GroupByTime::m_keys = nullptr;
        GroupByTime::m_haskeys = nullptr;
        GroupByTime::m_timewindow = time;
        init();
    }
    DynBase* GroupByTimeSum::CreateObject() { return new GroupByTimeSum(); }
    string GroupByTimeSum::getName() { return "GroupByTimeSum"; }
    void GroupByTimeSum::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        GroupByTime::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo GroupByTimeSum::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        std::vector<PFeatureInfo> discretefeatrues;
        PFeatureInfo datefeature = nullptr;
        for (int i = 0; i < sourcefeature.size(); ++i)
        {
            if (sourcefeature[i]->getType() == OutType::Discrete)
                discretefeatrues.push_back(sourcefeature[i]);
            else datefeature = sourcefeature[i];
        }
        PFeatureInfo featureinfo = nullptr;
        Feature* feature = nullptr;
        if (datefeature)
            feature = datefeature->getFeature();
        else
        {
            LOG(ERROR) << "GroupByTimeSum generateFeature Error: no DateFeature!";
            return new FeatureInfo();
        }
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        string key;
        MyDataType* valp = new MyDataType;
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < discretefeatrues.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            auto date = *reinterpret_cast<Date*>(datefeature->getFeature()->getValue(j));
            auto iterkeys = m_keys->equal_range(key);
            bool findflag = 0;
            for (auto iter = iterkeys.first; iter != iterkeys.second; ++iter)
            {
                auto mapval = iter->second;
                auto firstdate = mapval.begin()->first;
                if (timeOverWindow(date, firstdate))continue;
                findflag = 1;
                MyDataType sum = 0.0;
                std::for_each(mapval.begin(), mapval.end(), [&](const std::pair<Date, MyDataType>& a) {sum += a.second; });
                *valp = sum;
            }
            if (!findflag)*valp = 0;
            newfeature->setValue(j, reinterpret_cast<void*>(valp));
            key.clear();
        }
        delete valp;

        clear();
        string name = "GroupByTimeSum_" + std::to_string(m_timewindow) + "(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByTimeSum::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByTime::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }



    OutType GroupByTimeMean::getType() { return OutType::Numeric; }
    GroupByTimeMean::~GroupByTimeMean() { clear(); }
    FCOperator* GroupByTimeMean::copy() { return new GroupByTimeMean(m_timewindow); }
    GroupByTimeMean::GroupByTimeMean(MyDataType time ) {
        GroupByTime::m_keys = nullptr;
        GroupByTime::m_haskeys = nullptr;
        GroupByTime::m_timewindow = time;
        init();
    }
    DynBase* GroupByTimeMean::CreateObject() { return new GroupByTimeMean(); }
    string GroupByTimeMean::getName() { return "GroupByTimeCount"; }
    void GroupByTimeMean::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        GroupByTime::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo GroupByTimeMean::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        std::vector<PFeatureInfo> discretefeatrues;
        PFeatureInfo datefeature = nullptr;
        for (int i = 0; i < sourcefeature.size(); ++i)
        {
            if (sourcefeature[i]->getType() == OutType::Discrete)
                discretefeatrues.push_back(sourcefeature[i]);
            else datefeature = sourcefeature[i];
        }
        PFeatureInfo featureinfo = nullptr;
        Feature* feature = nullptr;
        if (datefeature)
            feature = datefeature->getFeature();
        else
        {
            LOG(ERROR) << "GroupByTimeMean generateFeature Error: no DateFeature!";
            return new FeatureInfo();
        }
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        string key;
        MyDataType* valp = new MyDataType;
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < discretefeatrues.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            auto date = *reinterpret_cast<Date*>(datefeature->getFeature()->getValue(j));
            auto iterkeys = m_keys->equal_range(key);
            bool findflag = 0;
            for (auto iter = iterkeys.first; iter != iterkeys.second; ++iter)
            {
                auto mapval = iter->second;
                auto firstdate = mapval.begin()->first;
                if (timeOverWindow(date, firstdate))continue;
                findflag = 1;
                MyDataType sum = 0.0;
                std::for_each(mapval.begin(), mapval.end(), [&](const std::pair<Date, MyDataType>& a) {sum += a.second; });
                *valp = sum / mapval.size();
            }
            if (!findflag)*valp = 0;
            newfeature->setValue(j, reinterpret_cast<void*>(valp));
            key.clear();
        }
        delete valp;

        clear();
        string name = "GroupByTimeMean_" + std::to_string(m_timewindow) + "(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByTimeMean::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByTime::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }
 


    OutType GroupByTimeStd::getType() { return OutType::Numeric; }
    GroupByTimeStd::~GroupByTimeStd() { clear(); }
    FCOperator* GroupByTimeStd::copy() { return new GroupByTimeStd(m_timewindow); }
    GroupByTimeStd::GroupByTimeStd(MyDataType time) {
        GroupByTime::m_keys = nullptr;
        GroupByTime::m_haskeys = nullptr;
        GroupByTime::m_timewindow = time;
        init();
    }
    DynBase* GroupByTimeStd::CreateObject() { return new GroupByTimeStd(); }
    string GroupByTimeStd::getName() { return "GroupByTimeStd"; }
    void GroupByTimeStd::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        GroupByTime::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
    }
    PFeatureInfo GroupByTimeStd::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        std::vector<PFeatureInfo> discretefeatrues;
        PFeatureInfo datefeature = nullptr;
        for (int i = 0; i < sourcefeature.size(); ++i)
        {
            if (sourcefeature[i]->getType() == OutType::Discrete)
                discretefeatrues.push_back(sourcefeature[i]);
            else datefeature = sourcefeature[i];
        }
        PFeatureInfo featureinfo = nullptr;
        Feature* feature = nullptr;
        if (datefeature)
            feature = datefeature->getFeature();
        else
        {
            LOG(ERROR) << "GroupByTimeStd generateFeature Error: no DateFeature!";
            return new FeatureInfo();
        }
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        string key;
        MyDataType* valp = new MyDataType;
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < discretefeatrues.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            auto date = *reinterpret_cast<Date*>(datefeature->getFeature()->getValue(j));
            auto iterkeys = m_keys->equal_range(key);
            bool findflag = 0;
            for (auto iter = iterkeys.first; iter != iterkeys.second; ++iter)
            {
                auto mapval = iter->second;
                auto firstdate = mapval.begin()->first;
                if (timeOverWindow(date, firstdate))continue;
                findflag = 1;
                if (mapval.size() <= 1) *valp = 0;
                MyDataType sum = 0.0;
                std::for_each(mapval.begin(), mapval.end(), [&](const std::pair<Date, MyDataType>& a) {sum += a.second ; });
                auto mean = sum / mapval.size();
                MyDataType sumstd = 0.0;
                std::for_each(std::begin(mapval), std::end(mapval), [&](const std::pair<Date, MyDataType> & d) {
                    sumstd += (d.second - mean) * (d.second - mean);
                    });
                *valp = sqrt(sumstd / mapval.size());
            }
            if (!findflag)*valp = 0;
            newfeature->setValue(j, reinterpret_cast<void*>(valp));
            key.clear();
        }
        delete valp;

        clear();
        string name = "GroupByTimeStd_" + std::to_string(m_timewindow) + "(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByTimeStd::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByTime::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }
    


    void GroupByCountOperator::init()
    {
        if (m_keys == nullptr)
            m_keys = new std::unordered_map<string, MyDataType>;
            
    }
    void GroupByCountOperator::clear()
    {
        if (m_keys != nullptr)
            m_keys->clear();
        //if (m_haskeys != nullptr) m_haskeys->clear();
        delete m_keys;
        //delete m_haskeys;
        m_keys = nullptr;
        //m_haskeys = nullptr;
    }
    GroupByCountOperator::~GroupByCountOperator() { clear(); }
    FCOperator* GroupByCountOperator::copy() { return new GroupByCountOperator(); }
    GroupByCountOperator::GroupByCountOperator() :m_keys(nullptr) { init(); }
    DynBase* GroupByCountOperator::CreateObject() { return new GroupByCountOperator(); }
    OutType GroupByCountOperator::getType() { return OutType::Numeric; }
    string GroupByCountOperator::getName() { return "GroupByCountOperator"; }
    void GroupByCountOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        init();
            
        GroupByOpertor::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        string key = "";
        auto tfeature = targetfeature[0]->getFeature();
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < sourcefeature.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::move(std::to_string(*reinterpret_cast<int*> (feature->getValue(j))));
            }
            auto val = *reinterpret_cast<MyDataType*>(tfeature->getValue(j));
            if (isnan(val) || isinf(val))
            {
                key.clear();
                continue;
            }

            (*m_keys)[key] += 1;
            
            key.clear();
                
        }
    }
    PFeatureInfo GroupByCountOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        MyMath<MyDataType>::parallelsetGroupValue(reinterpret_cast<MyDataType*> (newfeature->getValues()), sourcefeature, m_keys, n, threads, 0);
        clear();
        string name = "GroupByCountOperator(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByCountOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByOpertor::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }
   


    GroupByMaxOperator::~GroupByMaxOperator() { clear(); }
    void GroupByMaxOperator::init() {
        if (m_keys == nullptr)
            m_keys = new std::unordered_map<string, MyDataType>;
        //if (m_haskeys == nullptr) m_haskeys = new std::unordered_map<string, bool>;
    }
    void GroupByMaxOperator::clear()
    {
        if (m_keys != nullptr)
            m_keys->clear();
        //if (m_haskeys != nullptr) m_haskeys->clear();
        delete m_keys;
        //delete m_haskeys;
        m_keys = nullptr;
        //m_haskeys = nullptr;
    }
    FCOperator* GroupByMaxOperator::copy() { return new GroupByMaxOperator(); }
    GroupByMaxOperator::GroupByMaxOperator() :m_keys(nullptr) { init(); }
    DynBase* GroupByMaxOperator::CreateObject() { return new GroupByMaxOperator(); }
    OutType GroupByMaxOperator::getType() { return OutType::Numeric; }
    string GroupByMaxOperator::getName() { return "GroupByMaxOperator"; }
    void GroupByMaxOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        init();
        GroupByOpertor::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        string key = "";
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < sourcefeature.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            featureinfo = targetfeature[0];
            feature = featureinfo->getFeature();
            MyDataType valp = *reinterpret_cast<MyDataType*> (feature->getValue(j));
            if (isnan(valp) || isinf(valp))
            {
                key.clear();
                continue;
            }
            if (m_keys->find(key) == m_keys->end()) (*m_keys)[key] = valp;
			else (*m_keys)[key] = std::max((*m_keys)[key], valp);
            key.clear();
        }
    }
    PFeatureInfo GroupByMaxOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        int reallength = n;
        MyDataType missval = MyMath<MyDataType>::parallelMean(reinterpret_cast<MyDataType*> (targetfeature[0]->getFeature()->getValues()), n, threads, reallength);
        MyMath<MyDataType>::parallelsetGroupValue(reinterpret_cast<MyDataType*> (newfeature->getValues()), sourcefeature, m_keys, n, threads, missval);
        clear();
        string name = "GroupByMaxOperator(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByMaxOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByOpertor::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }
   


    GroupByMinOperator::~GroupByMinOperator() { clear(); }
    void GroupByMinOperator::init() {
        if (m_keys == nullptr)
            m_keys = new std::unordered_map<string, MyDataType>;
        //if (m_haskeys == nullptr)  m_haskeys = new std::unordered_map<string, bool>;
    }
    void GroupByMinOperator::clear()
    {
        if (m_keys != nullptr)
            m_keys->clear();
        //if (m_haskeys != nullptr) m_haskeys->clear();
        delete m_keys;
        //delete m_haskeys;
        m_keys = nullptr;
        //m_haskeys = nullptr;
    }
    FCOperator* GroupByMinOperator::copy() { return new GroupByMinOperator(); }
    GroupByMinOperator::GroupByMinOperator() :m_keys(nullptr) { init(); }
    DynBase* GroupByMinOperator::CreateObject() { return new GroupByMinOperator(); }
    OutType GroupByMinOperator::getType() { return OutType::Numeric; }
    string GroupByMinOperator::getName() { return "GroupByMinOperator"; }
    void GroupByMinOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        init();
        GroupByOpertor::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        string key = "";
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < sourcefeature.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::move(std::to_string(*reinterpret_cast<int*> (feature->getValue(j))));
            }
            featureinfo = targetfeature[0];
            feature = featureinfo->getFeature();
            MyDataType valp = *reinterpret_cast<MyDataType*> (feature->getValue(j));
            if (isnan(valp) || isinf(valp))
            {
                key.clear();
                continue;
            }
            if (m_keys->find(key) == m_keys->end()) (*m_keys)[key] = valp;
			else (*m_keys)[key] = std::min((*m_keys)[key], valp);
            key.clear();
        }

    }
    PFeatureInfo GroupByMinOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        int reallength = n;
        MyDataType missval = MyMath<MyDataType>::parallelMean(reinterpret_cast<MyDataType*> (targetfeature[0]->getFeature()->getValues()), n, threads, reallength);
        MyMath<MyDataType>::parallelsetGroupValue(reinterpret_cast<MyDataType*> (newfeature->getValues()), sourcefeature, m_keys, n, threads, missval);
        clear();
        string name = "GroupByMinOperator(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByMinOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByOpertor::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }
    


    GroupBySumOperator::~GroupBySumOperator() { clear(); }
    void GroupBySumOperator::init() {
        if (m_keys == nullptr)
            m_keys = new std::unordered_map<string, MyDataType>;
        //if (m_haskeys == nullptr) m_haskeys = new std::unordered_map<string, bool>;
    }
    void GroupBySumOperator::clear()
    {
        if (m_keys != nullptr)
            m_keys->clear();
        //if (m_haskeys != nullptr) m_haskeys->clear();
        delete m_keys;
        //delete m_haskeys;
        m_keys = nullptr;
        //m_haskeys = nullptr;
    }
    FCOperator* GroupBySumOperator::copy() { return new GroupBySumOperator(); }
    GroupBySumOperator::GroupBySumOperator() :m_keys(nullptr){ init(); }
    DynBase* GroupBySumOperator::CreateObject() { return new GroupBySumOperator(); }
    OutType GroupBySumOperator::getType() { return OutType::Numeric; }
    string GroupBySumOperator::getName() { return "GroupBySumOperator"; }
    void GroupBySumOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        init();
        GroupByOpertor::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        string key = "";
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < sourcefeature.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            featureinfo = targetfeature[0];
            feature = featureinfo->getFeature();
            MyDataType valp = *reinterpret_cast<MyDataType*> (feature->getValue(j));
            if (isnan(valp) || isinf(valp))
            {
                key.clear();
                continue;
            }
            (*m_keys)[key] += valp;
            key.clear();
        }
    }
    PFeatureInfo GroupBySumOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        int reallength = n;
        MyDataType missval = MyMath<MyDataType>::parallelMean(reinterpret_cast<MyDataType*> (targetfeature[0]->getFeature()->getValues()), n, threads, reallength);
        MyMath<MyDataType>::parallelsetGroupValue(reinterpret_cast<MyDataType*> (newfeature->getValues()), sourcefeature, m_keys, n, threads, missval);
        clear();
        string name = "GroupBySumOperator(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupBySumOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByOpertor::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }
    


    GroupByStdOperator::~GroupByStdOperator() { clear(); }
    void GroupByStdOperator::init() {
        if (m_keys == nullptr)
            m_keys = new std::unordered_map<string, MyDataType>;
        //if (m_haskeys == nullptr)  m_haskeys = new std::unordered_map<string, bool>;
    }
    void GroupByStdOperator::clear()
    {
        if (m_keys != nullptr)
            m_keys->clear();
        //if (m_haskeys != nullptr) m_haskeys->clear();
        delete m_keys;
        //delete m_haskeys;
        m_keys = nullptr;
        //m_haskeys = nullptr;
    }
    FCOperator* GroupByStdOperator::copy() { return new GroupByStdOperator; }
    GroupByStdOperator::GroupByStdOperator() :m_keys(nullptr) { init(); }
    DynBase* GroupByStdOperator::CreateObject() { return new GroupByStdOperator(); }
    OutType GroupByStdOperator::getType() { return OutType::Numeric; }
    string GroupByStdOperator::getName() { return "GroupByStdOperator"; }
    void GroupByStdOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        init();
        GroupByOpertor::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        std::unordered_map<string, std::vector<MyDataType>> keys;
        string key = "";
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < sourcefeature.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            featureinfo = targetfeature[0];
            feature = featureinfo->getFeature();
            MyDataType valp = *reinterpret_cast<MyDataType*> (feature->getValue(j));
            if (isnan(valp) || isinf(valp))
            {
                key.clear();
                continue;
            }
            keys[key].push_back(valp);
            key.clear();
        }
        int threads = Property::getProperty()->getThreadNum();
        for (auto& keyval : keys)
        {
            int length = keyval.second.size();
            auto valp = MyMath<MyDataType>::parallelStd(&keyval.second[0], length, threads);
            (*m_keys)[keyval.first] = valp;
        }
    }
    PFeatureInfo GroupByStdOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        int reallength = n;
        MyDataType missval = MyMath<MyDataType>::parallelMean(reinterpret_cast<MyDataType*> (targetfeature[0]->getFeature()->getValues()), n, threads, reallength);
        MyMath<MyDataType>::parallelsetGroupValue(reinterpret_cast<MyDataType*> (newfeature->getValues()), sourcefeature, m_keys, n, threads, missval);
        clear();
        string name = "GroupByStdOperator(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByStdOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByOpertor::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }
   


    FCOperator* GroupByMeanOperator::copy() { return new GroupByMeanOperator(); }
    GroupByMeanOperator::GroupByMeanOperator() :m_keys(nullptr) { init(); }
    GroupByMeanOperator::~GroupByMeanOperator() { clear(); }
    void GroupByMeanOperator::init() {
        if (m_keys == nullptr)
            m_keys = new std::unordered_map<string, MyDataType>;
    }
    void GroupByMeanOperator::clear()
    {
        if (m_keys != nullptr)
            m_keys->clear();
        delete m_keys;
        m_keys = nullptr;
    }
    DynBase* GroupByMeanOperator::CreateObject() { return new GroupByMeanOperator(); }
    OutType GroupByMeanOperator::getType() { return OutType::Numeric; }
    string GroupByMeanOperator::getName() { return "GroupByMeanOperator"; }
    void GroupByMeanOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        init();
        GroupByOpertor::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        std::unordered_map<string, std::vector<MyDataType>> keys;
        string key = "";
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < sourcefeature.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            featureinfo = targetfeature[0];
            feature = featureinfo->getFeature();
            MyDataType valp = *reinterpret_cast<MyDataType*> (feature->getValue(j));
            if (isnan(valp) || isinf(valp))
            {
                key.clear();
                continue;
            }
            keys[key].push_back(valp);
            key.clear();
        }
        int threads = Property::getProperty()->getThreadNum();
        for (auto& keyval : keys)
        {
            int length = keyval.second.size();
            auto valp = MyMath<MyDataType>::parallelMean(&keyval.second[0], length, threads, length);
            (*m_keys)[keyval.first] = valp;
        }
    }
    PFeatureInfo GroupByMeanOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        int reallength = n;
        MyDataType missval = MyMath<MyDataType>::parallelMean(reinterpret_cast<MyDataType*> (targetfeature[0]->getFeature()->getValues()), n, threads, reallength);
        MyMath<MyDataType>::parallelsetGroupValue(reinterpret_cast<MyDataType*> (newfeature->getValues()), sourcefeature, m_keys, n, threads, missval);
        clear();
        string name = "GroupByMeanOperator(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByMeanOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByOpertor::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }




    FCOperator* GroupByRankOperator::copy() { return new GroupByRankOperator(); }
    GroupByRankOperator::GroupByRankOperator() :m_keys(nullptr) { init(); }
    GroupByRankOperator::~GroupByRankOperator() { clear(); }
    void GroupByRankOperator::init() {
        if (m_keys == nullptr)
            m_keys = new std::unordered_map<string, std::unordered_map<MyDataType, MyDataType>>;
    }
    void GroupByRankOperator::clear()
    {
        if (m_keys != nullptr)
            m_keys->clear();
        delete m_keys;
        m_keys = nullptr;
    }
    DynBase* GroupByRankOperator::CreateObject() { return new GroupByRankOperator(); }
    OutType GroupByRankOperator::getType() { return OutType::Numeric; }
    string GroupByRankOperator::getName() { return "GroupByRankOperator"; }
    void GroupByRankOperator::preGenerateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature, bool issecondfeature, int featureinstances)
    {
        init();
        GroupByOpertor::preGenerateFeature(dataset, sourcefeature, targetfeature, issecondfeature, featureinstances);
        PFeatureInfo featureinfo = sourcefeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        std::unordered_map<string, std::vector<MyDataType>> keys;
        string key = "";
        for (int j = 0; j < n; j++)
        {
            for (int i = 0; i < sourcefeature.size(); i++)
            {
                featureinfo = sourcefeature[i];
                feature = featureinfo->getFeature();
                key += std::to_string(*reinterpret_cast<int*> (feature->getValue(j)));
            }
            featureinfo = targetfeature[0];
            feature = featureinfo->getFeature();
            MyDataType valp = *reinterpret_cast<MyDataType*> (feature->getValue(j));
            if (isnan(valp) || isinf(valp))
            {
                key.clear();
                continue;
            }
            keys[key].push_back(valp);
            key.clear();
        }
        int threads = Property::getProperty()->getThreadNum();
        for (auto& keyval : keys)
        {
            std::vector<MyDataType>& val = keyval.second;
            std::sort(val.begin(), val.end());
            int length = val.size();
            int duplicate = 1;
            for (int i = 0; i < length; i++)
            {
                if(i > 0 && val[i] == val[i - 1]) duplicate++;
                else duplicate = 1;
                if(m_keys->find(keyval.first) == m_keys->end())
                    (*m_keys)[keyval.first] = std::unordered_map<MyDataType, MyDataType>();
                if((*m_keys)[keyval.first].find(val[i]) == (*m_keys)[keyval.first].end())
                    (*m_keys)[keyval.first][val[i]] = i + 1;
                else // mean value
                    (*m_keys)[keyval.first][val[i]] = ((*m_keys)[keyval.first][val[i]] * (duplicate - 1) + i + 1) / duplicate;
            }
            for (auto &val: (*m_keys)[keyval.first])
                val.second = val.second / length;
        }
    }
    PFeatureInfo GroupByRankOperator::generateFeature(DataSet*& dataset, const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        PFeatureInfo featureinfo = targetfeature[0];
        Feature* feature = featureinfo->getFeature();
        int n = m_featureinstances;
        int N = feature->getInstances();
        Feature* newfeature = new NumericFeature(N);
        
        int threads = Property::getProperty()->getThreadNum();
        int reallength = n;
        MyMath<MyDataType>::parallelsetGroupRankValue(reinterpret_cast<MyDataType*> (feature->getValues()), reinterpret_cast<MyDataType*> (newfeature->getValues()), sourcefeature, m_keys, n, threads, 0);
        clear();
        string name = "GroupByRankOperator(";
        name += generateName(sourcefeature, targetfeature) + ")";
        return new FeatureInfo(newfeature, sourcefeature, targetfeature, OutType::Numeric, name, m_issecondfeature);
    }
    bool GroupByRankOperator::isMatch(const std::vector<PFeatureInfo>& sourcefeature, const std::vector<PFeatureInfo>& targetfeature)
    {
        if (!GroupByOpertor::isMatch(sourcefeature, targetfeature))return false;
        return true;
    }

}//EaSFE
