#pragma once
#include "./Property.h"
namespace EaSFE {
    class DynBase;
    struct ClassInfo;
    bool Register(ClassInfo* ci);
    typedef DynBase* (*funCreateObject)();
    //Assistant class to create object dynamicly
    struct ClassInfo
    {
    public:
        friend ClearClass;
        string Type;
        funCreateObject Fun;
        ClassInfo(string type, funCreateObject fun)
        {
            Type = type;
            Fun = fun;
            Register(this);
        }
    };

    //The base class of dynamic created class.

    //If you want to create a instance of a class ,you must let

    //the class derive from the DynBase.

    class DynBase
    {
    public:
        friend ClearClass;
        static bool Register(ClassInfo* classInfo);
        static DynBase* CreateObject(string type);
        virtual void virtualfunc() = 0;
        
    private:
        static std::unordered_map<string, ClassInfo*> m_classInfoMap;
    };

}//EaSFE