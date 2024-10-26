#ifndef THUNDERSVM_THUNDERSVM_H
#define THUNDERSVM_THUNDERSVM_H
#include <cstdlib>
#include "EaSFE/util/log.h"
#include <string>
#include <vector>
#include "math.h"
#include "util/common.h"
using std::string;
using std::vector;

#ifdef USE_DOUBLE
typedef double float_type;
typedef double kernel_type;
#else
typedef float float_type;
typedef float kernel_type;
#endif
#endif //THUNDERSVM_THUNDERSVM_H
