#include "EaSFE/FCOperators.h"
namespace EaSFE {
   
    FCOperators::FCOperators(std::vector<PFeatureInfo> sourcefeatures, std::vector<PFeatureInfo> targetfeatures, FCOperator* operator_, FCOperator* operator_second , bool istargetclass)
    {
        m_sourcefeatures = sourcefeatures;
        m_targetfeatures = targetfeatures;
        m_operator = operator_;
        m_istargetclass = istargetclass;
        m_secondoperator = operator_second;
    }
    FCOperators::~FCOperators() {};
    std::vector<PFeatureInfo> FCOperators::getSourceFeatures()const
    {
        return m_sourcefeatures;
    }
    std::vector<PFeatureInfo> FCOperators::getTargetFeatures()const
    {
        return m_targetfeatures;
    }
    FCOperator* FCOperators::getOperator()const
    {
        return m_operator;
    }
    FCOperator* FCOperators::getSecondOperator()const
    {
        return m_secondoperator;
    }
    bool FCOperators::getIsTargetClass()const
    {
        return m_istargetclass;
    }
    string FCOperators::getName()const
    {
        string name = "\"";
        name += "{sources:[";
        for (auto& i : m_sourcefeatures)
        {
            name += i->getName() + ",";
        }
        name += "];targets:[";
        for (auto& i : m_targetfeatures)
        {
            name += i->getName() + ",";
        }
        name += "];operator:";
        name += m_operator->getName();
        if (m_secondoperator != nullptr)
        {
            name += ";secondoperator:" + m_secondoperator->getName();
        }
        name += "}\"";
        return name;
    }
    OutType FCOperators::getOutType()const
    {
        if (m_secondoperator)
            return m_secondoperator->getType();
        return m_operator->getType();
    }
    void FCOperators::setFScore(MyDataType score)
    {
        m_fscore = score;
    }
    MyDataType FCOperators::getFScore()const
    {
        return m_fscore;
    }
    void FCOperators::setWScore(MyDataType score)
    {
        m_wscore = score;
    }
    MyDataType FCOperators::getWScore()const
    {
        return m_wscore;
    }
    ostream& operator<< (ostream& out, const FCOperators* & fcos)
    {
        out << "name : " << fcos->getName() << endl << "operatorName :" << fcos->getOperator()->getName() << endl;
        out << "wscore : " << fcos->getWScore() << endl << "fscore : " << fcos->getFScore() << "\ntype : " << static_cast<int> (fcos->getOutType()) << endl;
        out << "SecondOpreatorName : " << (fcos->getSecondOperator() ? fcos->getSecondOperator()->getName() : "nullptr") << endl;
        return out;
    }
    int FCOperators::getHasCalc()
    {
        return m_hascalc;
    }
    void FCOperators::setHasCalc(int hascalc)
    {
        m_hascalc = hascalc;
    }
    string FCOperators::getSaveInfo()const
    {
        string result = "";
        result += std::to_string(m_sourcefeatures.size());
        for (auto& sfea : m_sourcefeatures)
            result += " " + sfea->getName();
        result += "\n" + std::to_string(m_targetfeatures.size());
        for (auto& tfea : m_targetfeatures)
            result += " " + tfea->getName();
        result += "\n" + m_operator->getName();
        result += "\n" + (m_secondoperator == nullptr ? "nullptr" : m_secondoperator->getName());
        result += "\n";
        return result;
    }
    int FCOperators::getFCOperID()const
    {
        return m_fcoperid;
    }
    void FCOperators::setFCOperID(int fcoperid)
    {
        m_fcoperid = fcoperid;
    }
    void FCOperators::clear()
    {
        delete m_operator;
        m_operator = nullptr;
        delete m_secondoperator;
        m_secondoperator = nullptr;
        m_sourcefeatures.clear();
        m_targetfeatures.clear();
    }
    bool FCOperators::operator==(const FCOperators& fcoperator)const
    {
        if (this->m_operator->getName() != fcoperator.getOperator()->getName())return false;
        if (this->m_secondoperator != nullptr)
        {
            if (fcoperator.getSecondOperator() == nullptr)return false;
            if (this->m_secondoperator->getName() != fcoperator.getSecondOperator()->getName())return false;
        }
        if (this->m_sourcefeatures.size() != fcoperator.getSourceFeatures().size())return false;
        else
        {
            for (int i = 0; i < this->m_sourcefeatures.size(); ++i)
                if (*m_sourcefeatures[i] != fcoperator.getSourceFeatures()[i])return false;
        }
        if (this->m_targetfeatures.size() != fcoperator.getTargetFeatures().size())return false;
        else
        {
            for (int i = 0; i < this->m_targetfeatures.size(); ++i)
                if (*m_targetfeatures[i] != fcoperator.getTargetFeatures()[i])return false;
        }
        return true;
    }
    bool FCOperators::operator!=(const FCOperators& fcoperator)const
    {
        return !(*this == fcoperator);
    }
}//EaSFE