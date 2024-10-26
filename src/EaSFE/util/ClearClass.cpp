#include "EaSFE/util/ClearClass.h"
#include "EaSFE/Tools.h"

namespace EaSFE
{
	void ClearClass::clear()
	{
		if (Property::getProperty()->getDistributedNodes() > 1)
		{
#ifdef USE_MPICH
			MPI_Finalize();
#else
			LOG(ERROR) << "ClearClass Please USE_MPICH = ON when CMake this project, or you cannot set Property::getProperty()->setDistributedNodes() > 1";
			exit(-1);
#endif
		}
		delete Property::m_instance;
		delete globalVar::m_instance;
		DynBase::m_classInfoMap.clear();
		delete StdOperator::m_cInfo;
		delete DiscretizerOperator::m_cInfo;
		delete DayOfWeekOperator::m_cInfo;
		delete IsWeekendOperator::m_cInfo;
		delete CountOperator::m_cInfo;
		delete FloorOperator::m_cInfo;
		delete LogOperator::m_cInfo;
		delete AddOperator::m_cInfo;
		delete SubtractOperator::m_cInfo;
		delete MultiplyOperator::m_cInfo;
		delete DivideOperator::m_cInfo;
		delete MaxOperator::m_cInfo;
		delete MinOperator::m_cInfo;
		delete GroupByCountOperator::m_cInfo;
		delete GroupBySumOperator::m_cInfo;
		delete GroupByMeanOperator::m_cInfo;
		delete GroupByMinOperator::m_cInfo;
		delete GroupByMaxOperator::m_cInfo;
		delete GroupByStdOperator::m_cInfo;
		delete GroupByRankOperator::m_cInfo;
		delete GroupByTimeCount::m_cInfo;
		delete GroupByTimeMax::m_cInfo;
		delete GroupByTimeMin::m_cInfo;
		delete GroupByTimeMean::m_cInfo;
		delete GroupByTimeSum::m_cInfo;
		delete GroupByTimeStd::m_cInfo;
	}
}