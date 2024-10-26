#include "EaSFE/util/DynamicBase.h"
namespace EaSFE {
    

    bool DynBase::Register(ClassInfo* classInfo)
    {
        m_classInfoMap[classInfo->Type] = classInfo;
        return true;
    }

    DynBase* DynBase::CreateObject(string type)
    {
        if (m_classInfoMap[type] != nullptr)
        {
            return m_classInfoMap[type]->Fun();
        }
        return nullptr;
    }

    bool Register(ClassInfo* ci)
    {
        return DynBase::Register(ci);
    }
    /*
    DerivedClass::~DerivedClass()
    {
        // ToDo: Add your specialized code here and/or call the base class
    }

    DerivedClass::DerivedClass()
    {}

    ClassInfo* DerivedClass::m_cInfo = new ClassInfo("DerivedClass", (funCreateObject)(DerivedClass::CreateObject));
    */
}//EaSFE