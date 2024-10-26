#include "EaSFE/Filter.h"

namespace EaSFE{
	
	Filter::Filter() {}
	Filter::~Filter() { clear(); }
	void Filter::clear()
	{
		for (int i = 0;i < m_analyzedDataset.size();++ i)
		{
			auto& anyfea = m_analyzedDataset[i];
			if (m_isdiscrete[i])
			{
				anyfea->clear(false);
				delete anyfea;
				anyfea = nullptr;
			}
		}
		m_analyzedDataset.clear();
	}
	void Filter::init(const std::vector<PFeatureInfo>& analyzedDataset)
	{
		clear();
		m_analyzedDataset = analyzedDataset;
		m_isdiscrete.clear();
		m_isdiscrete.resize(m_analyzedDataset.size(), 0);
			
	}
	void Filter::discretizefeature(DataSet* dataset)
	{
		for (int i = 0; i < m_analyzedDataset.size(); ++i)
		{
			auto& featureinfo = m_analyzedDataset[i];
			if (featureinfo->getType() == OutType::Numeric)
			{
				auto discretizeoper = DiscretizerOperator();
				std::vector<PFeatureInfo> featureinfos;
				featureinfos.push_back(featureinfo);
				auto fops = FCOperators(featureinfos, std::vector<PFeatureInfo>(), &discretizeoper);
				featureinfo = generateFeature(dataset, &fops, true, true, false);
				m_isdiscrete[i] = 1;
					
			}
		}
	}
	MyDataType Filter::produceScore(DataSet* dataset, const PFCOperators& fcoas)
	{
		try
		{
			auto targetfeature = dataset->getTargetFeature()->getFeature();
			if (Property::getProperty()->getTargetClasses() <= 0 || targetfeature->getType() == FeatureType::Numeric) return -produceANOVAScore(dataset, fcoas);
			discretizefeature(dataset);
			if (targetfeature->getType() == FeatureType::Numeric)return 0.0;
			int targetindex = dataset->getDatasetIndex()[dataset->getID()];
			auto instances = dataset->getInstancesOfFeature();
			if (dataset->getNumID() > 1) instances = dataset->getInstancesOfFeature(3);
			for (int i = 0; i < instances; ++i)
			{
				string key;
				for (auto& ans : m_analyzedDataset)
				{
					key += std::to_string(*(reinterpret_cast<int*>(ans->getFeature()->getValue(i))));
				}
				auto label = *reinterpret_cast<int*> (targetfeature->getValue(i + targetindex));
				std::vector<int> labelvalue(targetfeature->getNumsOfValues(), 0);
				if (m_valuesperkey[key].size() == 0)
					m_valuesperkey[key] = labelvalue;
				m_valuesperkey[key][label] ++;
			}
			auto score = calculateIG(instances);
			return score;
		}
		catch (...)
		{
			LOG(ERROR) << "Filter produceScore Error!";
			return 0.0;
		}
	}

	MyDataType Filter::produceANOVAScore(DataSet* dataset, const PFCOperators& fcoas)
	{
		try
		{
			auto targetfeature = dataset->getTargetFeature()->getFeature();
			if (targetfeature->getType() != FeatureType::Numeric) {
				LOG(ERROR) << "Target feature is not numeric!";
				return 0.0;
			}
			auto instances = dataset->getInstancesOfFeature();
			int targetindex = dataset->getDatasetIndex()[dataset->getID()];

			if (dataset->getNumID() > 1) instances = dataset->getInstancesOfFeature(3);
			std::unordered_map<std::string, std::vector<MyDataType>> groups;
			for (int i = 0; i < instances; ++i)
			{
				std::string key;
				for (auto& ans : m_analyzedDataset)
				{
					key += std::to_string(*(reinterpret_cast<int*>(ans->getFeature()->getValue(i))));
				}
				auto label = *reinterpret_cast<MyDataType*> (targetfeature->getValue(i+targetindex));
				groups[key].push_back(label);
			}
			auto score = calculateANOVA(groups);
			return score;
		}
		catch (...)
		{
			LOG(ERROR) << "Filter produceScore Error!";
			return 0.0;
		}
	}

	MyDataType Filter::calculateANOVA(const std::unordered_map<std::string, std::vector<MyDataType>>& groups)
	{
		int n = 0; // total number of instances
		MyDataType ss_total = 0.0; // total sum of squares
		MyDataType ss_between = 0.0; // total sum of squares between groups
		MyDataType grand_mean = 0.0; // total mean
		std::unordered_map<std::string, MyDataType> groupmeans;
		long double sum = 0.0;
		for (const auto& pair : groups)
		{
			const auto& values = pair.second;
			MyDataType groupsum = std::accumulate(values.begin(), values.end(), 0.0);
			sum += groupsum;
			groupmeans[pair.first] = groupsum / values.size();
			n += values.size();
		}
		grand_mean = sum / n;
		for (const auto& pair : groups)
		{
			const auto& values = pair.second;
			MyDataType group_mean = groupmeans[pair.first];
			ss_between += values.size() * std::pow(group_mean - grand_mean, 2);
			for (MyDataType value : values)
			{
				ss_total += std::pow(value - grand_mean, 2);
			}
		}
		MyDataType ss_within = ss_total - ss_between; // the sum of squares within groups
		int df_between = groups.size() - 1; // the between-group degrees of freedom
		int df_within = n - groups.size(); // the within-group degrees of freedom
		MyDataType ms_between = ss_between / df_between; // the between-group mean square
		MyDataType ms_within = ss_within / df_within; // the within-group mean square
		MyDataType f = ms_between / ms_within; // the F-value
		//MyDataType p = 1 - MyMath<MyDataType>::f_cdf(f, df_between, df_within); // the p-value
		return f;
	}

	MyDataType Filter::calculateIG(int instances)
	{
		MyDataType ig = 0;
		for (auto& val : m_valuesperkey)
		{
			auto numofinstance = MyMath<int>::getSum(val.second);
			if (numofinstance < 1.0)continue;
			MyDataType tempig = 0.0;
			for (auto& va : val.second)
				if (va > 0)
				{
					tempig += -((va / (MyDataType)numofinstance) * log2(va / (MyDataType)numofinstance));
				}
			ig += ((MyDataType)numofinstance / instances) * tempig;
		}
		return ig;
	}

}//EaSFE