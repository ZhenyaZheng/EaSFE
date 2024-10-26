#pragma once
#include <string>
namespace EaSFE {
    class MyExcept: public std::exception
    {
    public:
        MyExcept(string msg)
        {
            m_msg = msg;
        }
        string getMsg()
        {
            return m_msg + string(what());
        }
    private:
        string m_msg;
    };
}//EaSFE