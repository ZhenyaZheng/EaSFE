#pragma once
#include <string>
#include <iostream>
#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <fstream>
#include <algorithm>

#include <random>
#include <queue>
#include <atomic>
#include <omp.h>
#include <thread>
#ifdef USE_MPICH
#include "mpi.h"
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX
// #include <filesystem>
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(WIN32)
#include "util/mydirent.h"
#else
#include <dirent.h>
#endif
#include "util/log.h"


using std::string, std::cout, std::exception, std::endl, std::ios, std::ofstream, std::ifstream, std::istream, std::ostream ;
namespace EaSFE {
#define MAX_NUM_CATEGORY 500
#define MAX_GBM_FEATURES 3000
#define MAX_GBM_IMPORTANCE 2000
#ifdef USE_DOUBLE
    typedef double MyDataType;
#else
    typedef float MyDataType;
#endif

    enum class OutType
    {
        Numeric, Discrete, Date, String
    };
    enum class OperatorType
    {
        Unary, Binary, GroupBy, GroupByTime
    };
    enum class FeatureType
    {
        Numeric, Discrete, Date, String
    };
    enum class DataType
    {
		CSV, Distribute, LibSVMCF, LibSVMRG, DIR
	};
    
}//EaSFE