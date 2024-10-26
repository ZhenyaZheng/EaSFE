#pragma once
#include "./DataSet.h"
#include "./util/MyMath.h"
#include "./FCOperator.h"
#include "./Tools.h"
namespace EaSFE{
	class Filter
	{
		std::unordered_map<string, std::vector<int>> m_valuesperkey;
		std::vector<PFeatureInfo> m_analyzedDataset;
		std::vector<int> m_isdiscrete;
	public:
		Filter();
		~Filter();
		void clear();
		void init(const std::vector<PFeatureInfo>& analyzedDataset);
		void discretizefeature(DataSet* dataset);
		MyDataType produceScore(DataSet* dataset, const PFCOperators & fcoas=nullptr);
		MyDataType produceANOVAScore(DataSet* dataset, const PFCOperators& fcoas);
		MyDataType calculateANOVA(const std::unordered_map<std::string, std::vector<MyDataType>>& groups);
		MyDataType calculateIG(int instances);
	};
}//EaSFE