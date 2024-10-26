#include "EaSFE/FCOperator.h"
#include "EaSFE/Tools.h"

INITIALIZE_EASYLOGGINGPP

namespace EaSFE {
	Property* Property::m_instance = nullptr;
	globalVar* globalVar::m_instance = nullptr;
	std::unordered_map< string, ClassInfo*> DynBase::m_classInfoMap = std::unordered_map< string, ClassInfo*>();
	ClassInfo* StdOperator::m_cInfo = new ClassInfo("StdOperator", (funCreateObject)(StdOperator::CreateObject));
	ClassInfo* DiscretizerOperator::m_cInfo = new ClassInfo("DiscretizerOperator", (funCreateObject)(DiscretizerOperator::CreateObject));
	ClassInfo* DayOfWeekOperator::m_cInfo = new ClassInfo("DayOfWeekOperator", (funCreateObject)(DayOfWeekOperator::CreateObject));
	ClassInfo* IsWeekendOperator::m_cInfo = new ClassInfo("IsWeekendOperator", (funCreateObject)(IsWeekendOperator::CreateObject));
	ClassInfo* CountOperator::m_cInfo = new ClassInfo("CountOperator", (funCreateObject)(CountOperator::CreateObject));
	ClassInfo* FloorOperator::m_cInfo = new ClassInfo("FloorOperator", (funCreateObject)(FloorOperator::CreateObject));
	ClassInfo* LogOperator::m_cInfo = new ClassInfo("LogOperator", (funCreateObject)(LogOperator::CreateObject));
	ClassInfo* AddOperator::m_cInfo = new ClassInfo("AddOperator", (funCreateObject)(AddOperator::CreateObject));
	ClassInfo* SubtractOperator::m_cInfo = new ClassInfo("SubtractOperator", (funCreateObject)(SubtractOperator::CreateObject));
	ClassInfo* MultiplyOperator::m_cInfo = new ClassInfo("MultiplyOperator", (funCreateObject)(MultiplyOperator::CreateObject));
	ClassInfo* DivideOperator::m_cInfo = new ClassInfo("DivideOperator", (funCreateObject)(DivideOperator::CreateObject));
	ClassInfo* MaxOperator::m_cInfo = new ClassInfo("MaxOperator", (funCreateObject)(MaxOperator::CreateObject));
	ClassInfo* MinOperator::m_cInfo = new ClassInfo("MinOperator", (funCreateObject)(MinOperator::CreateObject));
	ClassInfo* GroupByCountOperator::m_cInfo = new ClassInfo("GroupByCountOperator", (funCreateObject)(GroupByCountOperator::CreateObject));
	ClassInfo* GroupBySumOperator::m_cInfo = new ClassInfo("GroupBySumOperator", (funCreateObject)(GroupBySumOperator::CreateObject));
	ClassInfo* GroupByMeanOperator::m_cInfo = new ClassInfo("GroupByMeanOperator", (funCreateObject)(GroupByMeanOperator::CreateObject));
	ClassInfo* GroupByMinOperator::m_cInfo = new ClassInfo("GroupByMinOperator", (funCreateObject)(GroupByMinOperator::CreateObject));
	ClassInfo* GroupByMaxOperator::m_cInfo = new ClassInfo("GroupByMaxOperator", (funCreateObject)(GroupByMaxOperator::CreateObject));
	ClassInfo* GroupByStdOperator::m_cInfo = new ClassInfo("GroupByStdOperator", (funCreateObject)(GroupByStdOperator::CreateObject));
	ClassInfo* GroupByRankOperator::m_cInfo = new ClassInfo("GroupByRankOperator", (funCreateObject)(GroupByRankOperator::CreateObject));
	ClassInfo* GroupByTimeCount::m_cInfo = new ClassInfo("GroupByTimeCount", (funCreateObject)(GroupByTimeCount::CreateObject));
	ClassInfo* GroupByTimeMax::m_cInfo = new ClassInfo("GroupByTimeMax", (funCreateObject)(GroupByTimeMax::CreateObject));
	ClassInfo* GroupByTimeMin::m_cInfo = new ClassInfo("GroupByTimeMin", (funCreateObject)(GroupByTimeMin::CreateObject));
	ClassInfo* GroupByTimeSum::m_cInfo = new ClassInfo("GroupByTimeSum", (funCreateObject)(GroupByTimeSum::CreateObject));
	ClassInfo* GroupByTimeMean::m_cInfo = new ClassInfo("GroupByTimeMean", (funCreateObject)(GroupByTimeMean::CreateObject));
	ClassInfo* GroupByTimeStd::m_cInfo = new ClassInfo("GroupByTimeStd", (funCreateObject)(GroupByTimeStd::CreateObject));
}//EaSFE