#pragma once
#include <math.h>
#include <cstdint>
#include <cstring>
#include "./Property.h"
#include "./MyException.h"
#include "EaSFE/FeatureInfo.h"
namespace EaSFE {
	class Gamma
	{
	public:
		static double gamma(double x);
		static double invGamma1pm1(double x);
		static double logGamma1p(double x);
		static double lanczos(double x);
		static double logGamma(double x);

	};

	class Beta
	{
	public:
		static double logGammaMinusLogGammaSum(double a, double b);
		static double deltaMinusDeltaSum(double a, double b);
		static double sumDeltaMinusDeltaSum(double p, double q);
		static double logGammaSum(double a, double b);
		static double logBeta(double p, double q);
	};

	class Precision
	{
	public:
		static  long long doubleToRawBits(double x);
		static bool equals(double x, double y, double eps);
		static bool equals(double x, double y, int maxUlps);
	};

	class ContinuedFraction
	{
	private:
		double m_b, m_a;
	public:
		ContinuedFraction(double b = 0.0, double a = 0.0);
		double getB(int n, double x);
		double getA(int n, double x);
		double evaluate(double x, double epsilon, int maxIterations);
	};


	template<class T>
	class MyMath
	{
	public:
#define minparallellength 1e4

		static T getMax(const std::vector<T>& vec)
		{
			auto maxvalueiter = max_element(vec.begin(), vec.end());
			return *maxvalueiter;
		}
		static T getMin(const std::vector<T>& vec)
		{
			auto minvalueiter = min_element(vec.begin(), vec.end());
			return *minvalueiter;

		}
		static T getMean(const std::vector<T>& vec)
		{
			if (vec.size() < 1)return static_cast<T>(0);
			auto sum = std::accumulate(std::begin(vec), std::end(vec), 0.0);
			return sum / vec.size();
		}
		static T getStd(const std::vector<T>& vec)
		{
			if (vec.size() <= 1)return static_cast<T>(0);
			auto mean = getMean(vec);
			MyDataType accum = 0.0;
			std::for_each(std::begin(vec), std::end(vec), [&](const T d) {
				accum += (d - mean) * (d - mean);
				});

			return sqrt(accum / vec.size());
		}
		static T getSum(const std::vector<T>& vec)
		{
			auto sum = std::accumulate(std::begin(vec), std::end(vec), static_cast<T>(0));
			return sum;
		}
		static double regularizedBeta(double x, double a, double b, double epsilon, int maxIterations)
		{
			double ret;
			if (!isnan(x) && !isnan(a) && !isnan(b) && !(x < 0.0) && !(x > 1.0) && !(a <= 0.0) && !(b <= 0.0))
			{
				if (x > (a + 1.0) / (a + b + 2.0))
				{
					ret = 1.0 - regularizedBeta(1.0 - x, b, a, epsilon, maxIterations);
				}
				else
				{
					ContinuedFraction fraction(b, a);
					try {
						auto logbeta = Beta::logBeta(a, b);
						auto feval = fraction.evaluate(x, epsilon, maxIterations);
						ret = exp(a * log(x) + b * log(1.0 - x) - log(a) - logbeta) * 1.0 / feval;
					}
					catch (MyExcept& ex)
					{
						cout << ex.getMsg() << endl;
						return 0.0;
					}
				}
			}
			else
			{
				ret = 0.0 * 0.0;
			}

			return ret;
		}
		static MyDataType getTtest(const std::vector<T>& vec, const std::vector<T>& vec2)
		{
			auto sumDifference = [=](const std::vector<T>& vec, const std::vector<T>& vec2)
				{
					const auto n = vec.size();
					if (n != vec2.size())
					{
						throw MyExcept("vec.size(" + std::to_string(vec.size()) + ") != vec2.size(" + std::to_string(vec2.size()) + ")!");
					}
					else
					{
						double result = 0.0;
						for (int i = 0; i < n; ++i)
							result += vec[i] - vec2[i];
						return result;
					}

				};
			auto meandiff = sumDifference(vec, vec2) / (double)vec.size();
			auto varianceDifference = [=](const std::vector<T>& vec, const std::vector<T>& vec2, double meanDifference)
				{
					double sum1 = 0.0;
					double sum2 = 0.0;
					double diff = 0.0;
					const int n = vec.size();
					if (n != vec2.size()) {
						throw MyExcept("vec.size(" + std::to_string(vec.size()) + ") != vec2.size(" + std::to_string(vec2.size()) + ")!");
					}
					else if (n < 2) {
						throw MyExcept("vec.size(" + std::to_string(vec.size()) + ") < 2!");
					}
					else {
						for (int i = 0; i < n; ++i) {
							diff = vec[i] - vec2[i];
							sum1 += (diff - meanDifference) * (diff - meanDifference);
							sum2 += diff - meanDifference;
						}

						return (sum1 - sum2 * sum2 / (double)n) / (double)(n - 1);
					}
				};
			auto variancediff = varianceDifference(vec, vec2, meandiff);

			auto tTest = [=](double m, double mu, double v, double n)
				{
					auto thet = [=](double m, double mu, double v, double n)
						{
							return (m - mu) / sqrt(v / n);
						};
					auto t = abs(thet(m, mu, v, n));
					auto cumulativeProbability = [=](double x, double degreesOfFreedom)
						{

							double ret;
							if (x == 0.0) {
								ret = 0.5;
							}
							else {
								double t = regularizedBeta(degreesOfFreedom / (degreesOfFreedom + x * x), 0.5 * degreesOfFreedom, 0.5, 1.0E-14, 2147483647);
								if (x < 0.0) {
									ret = 0.5 * t;
								}
								else {
									ret = 1.0 - 0.5 * t;
								}
							}

							return ret;
						};
					return 2.0 * cumulativeProbability(-t, n - 1.0);
				};
			auto pairttest = tTest(meandiff, 0.0, variancediff, (double)vec.size());
			return static_cast<MyDataType>(pairttest);
		}
		static MyDataType getChitest(const std::vector<std::vector<T>>& vec)
		{
			const int m = vec.size();
			if (m == 0) return 0.0;
			const int n = vec[0].size();
			std::vector<MyDataType> rowsum(m), colsum(n);
			MyDataType total = 0.0;
			for (int i = 0; i < m; ++i)
			{
				for (int j = 0; j < n; ++j)
				{
					rowsum[i] += vec[i][j];
					colsum[j] += vec[i][j];
					total += vec[i][j];
				}
			}
			MyDataType sumsq = 0.0;
			for (int i = 0; i < m; ++i)
			{
				for (int j = 0; j < n; ++j)
				{
					if (total == 0.0)return 0.0;
					MyDataType expected = rowsum[i] * colsum[j] / total;
					if (expected == 0.0)
						continue;
					sumsq += (vec[i][j] - expected) * (vec[i][j] - expected) / expected;
				}
			}
			return sumsq;
		}
		static std::vector<std::vector<MyDataType>> generateDiscreteAttributesCategoryIntersection(const std::vector<T>& vec, const std::vector<T>& vec2, const int n = Property::getProperty()->getDiscreteClass(), const int m = Property::getProperty()->getDiscreteClass())
		{
			std::vector<std::vector<MyDataType>> result(n, std::vector<MyDataType>(m));
			if (vec.size() != vec2.size())
			{
				LOG(ERROR) << "MyMath generateDiscreteAttributesCategoryIntersection Error length of vec and vec2 is not equal";
				return std::vector<std::vector<MyDataType>>();
			}
			for (int i = 0; i < vec.size(); ++i)
				result[vec[i]][vec2[i]]++;
			return result;
		}

		static T parallelSum(T* source, int length, int threadnums, int& reallength)
		{
			reallength = length;
			if (threadnums < 1) threadnums = 1;
			if (threadnums > length) threadnums = length;
			if (length < minparallellength) threadnums = 1;
			int lengthPerThread = length / threadnums;
			std::vector<T> results(threadnums, 0);
			std::vector<int> reallengths(threadnums, 0);
			const int ompthreadnums = threadnums;
#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? length : (tid + 1) * lengthPerThread;
				for (int i = start; i < end; i++)
				{
					if (isnan(source[i]) == false && isinf(source[i]) == false)
						results[tid] += source[i];
					else reallengths[tid]--;
				}
			}
			reallength = std::accumulate(reallengths.begin(), reallengths.end(), reallength);
			return std::accumulate(results.begin(), results.end(), 0);
		}

		static MyDataType parallelMean(T* source, int length, int threadnums, int &reallength)
		{
			auto sum = parallelSum(source, length, threadnums, reallength);
			return sum / reallength;
		}

		static MyDataType parallelStd(T* source, int length, int threadnums)
		{
			int reallength = length;
			if(reallength <= 1) return 1;
			auto mean = parallelMean(source, length, threadnums, reallength);
			if (threadnums < 1) threadnums = 1;
			if (threadnums > length) threadnums = length;
			if (length < minparallellength) threadnums = 1;
			int lengthPerThread = length / threadnums;
			std::vector<T> results(threadnums);
			const int ompthreadnums = threadnums;
#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? length : (tid + 1) * lengthPerThread;
				T accum = 0.0;
				for (int i = start; i < end; i++)
				{
					if (isnan(source[i]) == false && isinf(source[i]) == false)
						accum += (source[i] - mean) * (source[i] - mean);
				}
				results[tid] = accum;
			}
			auto accum = std::accumulate(results.begin(), results.end(), 0.0);
			return sqrt(accum / (reallength - 1));
		}

		static void parallelsetStd(T* source, T* target, MyDataType mean, MyDataType std, int length, int threadnums)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > length) threadnums = length;
			if (length < minparallellength) threadnums = 1;
			if (std == 0.0) return;
			int lengthPerThread = length / threadnums;
			const int ompthreadnums = threadnums;
			#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? length : (tid + 1) * lengthPerThread;
				for (int i = start; i < end; i++)
				{
					if (isnan(source[i]) == false && isinf(source[i]) == false)
						target[i] = (source[i] - mean) / std;
				}
			}
		}

		static T parallelMin(T* source, int length, int threadnums)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > length) threadnums = length;
			if (length < minparallellength) threadnums = 1;
			int lengthPerThread = length / threadnums;
			std::vector<T> results(threadnums);
			auto getMin = [&](T* source, int begin_, int end_)
			{
				bool hasvalue = false;
				T mymin;
				for (int i = begin_; i < end_; ++i)
				{
					if (isnan(source[i]) == false && isinf(source[i]) == false)
					{
						if (hasvalue == false)
						{
							mymin = source[i];
							hasvalue = true;
						}
						else
							mymin = std::min(mymin, source[i]);
					}
				}
				if (hasvalue == false) return static_cast<MyDataType>(NAN);
				return mymin;
			};
			const int ompthreadnums = threadnums;
			#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? length : (tid + 1) * lengthPerThread;
				results[tid] = getMin(source, start, end);
			}
			
			return getMin(results.data(), 0, threadnums);
		}

		static T parallelMax(T* source, int length, int threadnums)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > length) threadnums = length;
			if (length < minparallellength) threadnums = 1;
			int lengthPerThread = length / threadnums;
			std::vector<T> results(threadnums);
			auto getMax = [&](T* source, int begin_, int end_)
			{
				bool hasvalue = false;
				T mymax;
				for (int i = begin_; i < end_; ++i)
				{
					if (isnan(source[i]) == false && isinf(source[i]) == false)
					{
						if (hasvalue == false)
						{
							mymax = source[i];
							hasvalue = true;
						}
						else
							mymax = std::max(mymax, source[i]);
					}
				}
				if (hasvalue == false) return static_cast<MyDataType>(NAN);
				return mymax;
			};
			const int ompthreadnums = threadnums;
			#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? length : (tid + 1) * lengthPerThread;
				results[tid] = getMax(source, start, end);
			}
			return getMax(results.data(), 0, threadnums);
		}
		
		template<class Tg>
		static void parallelsetDiscretizer(T* source, Tg* target, T min, T max, T step, int n, int threadnums)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > n) threadnums = n;
			if (n < minparallellength) threadnums = 1;
			if (step == 0) return;
			int lengthPerThread = n / threadnums;
			const int ompthreadnums = threadnums;
#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? n : (tid + 1) * lengthPerThread;
				for (int i = start; i < end; i++)
				{
					if (isnan(source[i]) == false && isinf(source[i]) == false)
						target[i] = static_cast<Tg>((source[i] - min) / step);
				}
			}
		}

		static void parallelsetAdd(T* source1, T* source2, T* target, int length, int threadnums)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > length) threadnums = length;
			if (length < minparallellength) threadnums = 1;
			int lengthPerThread = length / threadnums;
			const int ompthreadnums = threadnums;
#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? length : (tid + 1) * lengthPerThread;
				for (int i = start; i < end; i++)
				{
					if (isnan(source1[i]) == false && isinf(source1[i]) == false && isnan(source2[i]) == false && isinf(source2[i]) == false)
						target[i] = source1[i] + source2[i];
				}
			}
		}

		static void parallelsetSubtract(T* source1, T* source2, T* target, int length, int threadnums)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > length) threadnums = length;
			if (length < minparallellength) threadnums = 1;
			int lengthPerThread = length / threadnums;
			const int ompthreadnums = threadnums;
			#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? length : (tid + 1) * lengthPerThread;
				for (int i = start; i < end; i++)
				{
					if (isnan(source1[i]) == false && isinf(source1[i]) == false && isnan(source2[i]) == false && isinf(source2[i]) == false)
						target[i] = source1[i] - source2[i];
				}
			}
		}

		static void parallelsetMultiply(T* source1, T* source2, T* target, int length, int threadnums)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > length) threadnums = length;
			if (length < minparallellength) threadnums = 1;
			int lengthPerThread = length / threadnums;
			const int ompthreadnums = threadnums;
			#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? length : (tid + 1) * lengthPerThread;
				for (int i = start; i < end; i++)
				{
					if (isnan(source1[i]) == false && isinf(source1[i]) == false && isnan(source2[i]) == false && isinf(source2[i]) == false)
						target[i] = source1[i] * source2[i];
				}
			}
		}

		static void parallelsetDivide(T* source1, T* source2, T* target, int length, int threadnums)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > length) threadnums = length;
			if (length < minparallellength) threadnums = 1;
			int lengthPerThread = length / threadnums;
			const int ompthreadnums = threadnums;
			#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? length : (tid + 1) * lengthPerThread;
				for (int i = start; i < end; i++)
				{
					if (isnan(source1[i]) == false && isinf(source1[i]) == false && isnan(source2[i]) == false && isinf(source2[i]) == false)
						if (source2[i] != 0) target[i] = source1[i] / source2[i];
				}
			}
		}

		static void parallelsetGroupValue(T* target, const std::vector<PFeatureInfo>& vec, std::unordered_map<string, MyDataType>*& p, int n, int threadnums, T missval)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > n) threadnums = n;
			if (n < minparallellength) threadnums = 1;
			int lengthPerThread = n / threadnums;
			const int ompthreadnums = threadnums;
#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? n : (tid + 1) * lengthPerThread;
				string key;
				for (int i = start; i < end; ++i)
				{
					for (auto& featureinfo : vec)
					{
						auto feature = featureinfo->getFeature();
						if(featureinfo->getType() == OutType::Discrete)
							key += std::to_string(*reinterpret_cast<int*> (feature->getValue(i)));
						else if (featureinfo->getType() == OutType::Numeric)
							key += std::to_string(*reinterpret_cast<MyDataType*> (feature->getValue(i)));
					}
					if (p->find(key) != p->end())
						target[i] = (*p)[key];
					else
						target[i] = missval;
					key.clear();
				}
			}
		}

		static void parallelsetGroupRankValue(T* source, T* target, const std::vector<PFeatureInfo>& vec, std::unordered_map<string, std::unordered_map<MyDataType, MyDataType>>*& p, int n, int threadnums, T missval)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > n) threadnums = n;
			if (n < minparallellength) threadnums = 1;
			int lengthPerThread = n / threadnums;
			const int ompthreadnums = threadnums;
#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? n : (tid + 1) * lengthPerThread;
				string key;
				for (int i = start; i < end; ++i)
				{
					for (auto& featureinfo : vec)
					{
						auto feature = featureinfo->getFeature();
						key += std::to_string(*reinterpret_cast<int*> (feature->getValue(i)));
					}
					if (p->find(key) != p->end())
						if(p->at(key).find(source[i]) != p->at(key).end())
							target[i] = p->at(key)[source[i]];
						else
							target[i] = missval;
					else
						target[i] = missval;
					key.clear();
				}
			}
		}

		static bool parallelisSingleVec(T* source, int n, int threadnums)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > n) threadnums = n;
			if (n < minparallellength) threadnums = 1;
			int lengthPerThread = n / threadnums;
			std::vector<bool> results(threadnums, true);
			const int ompthreadnums = threadnums;
			#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? n : (tid + 1) * lengthPerThread;
				for (int i = start + 1; i < end; i++)
				{
					if (source[i] != source[i - 1])
					{
						results[tid] = false;
						break;
					}
				}
			}
			return std::all_of(results.begin(), results.end(), [](bool b) {return b; });
		}

		static void parallelSetMaxMin(T* source, T* source2, T* target, int n, int threadnums, T missval, bool ismax)
		{
			if (threadnums < 1) threadnums = 1;
			if (threadnums > n) threadnums = n;
			if (n < minparallellength) threadnums = 1;
			int lengthPerThread = n / threadnums;
			std::vector<T> results(threadnums);
			const int ompthreadnums = threadnums;
			#pragma omp parallel num_threads(ompthreadnums)
			{
				int tid = omp_get_thread_num();
				int start = tid * lengthPerThread;
				int end = (tid == threadnums - 1) ? n : (tid + 1) * lengthPerThread;
				for (int i = start; i < end; i++)
				{
					if (isnan(source[i]) || isinf(source[i]))
					{
						if(isnan(source2[i]) == false && isinf(source2[i]) == false)
							target[i] = source2[i];
						else target[i] = missval;
					}
					else if (isnan(source2[i]) || isinf(source2[i]))
						target[i] = source[i];
					else{
						if (ismax)
							target[i] = std::max(source[i], source2[i]);
						else
							target[i] = std::min(source[i], source2[i]);
					}
				}
			}
		}
	};
}//EaSFE