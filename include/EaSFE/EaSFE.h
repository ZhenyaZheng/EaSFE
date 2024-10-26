#pragma once
#include "./Filter.h"
#include "./util/ClearClass.h"
#include "Load.h"
#include "GBMFilter.h"
namespace EaSFE {
    DataSet* clearProcessesfortestdata();
    DataSet* getValData();
	DataSet* LoadTrainData();
    void selectFeatureFromScore(double* scores, std::vector<std::pair<PFeatureInfo, PFCOperators>> & featurefcops, int orinumfeatures, int leftfeatures = -1); 
	DataSet* clearProcessesfortestdata();
	DataSet* startFeatureConstruction(DataSet* &dataset, DataSet* &testdataset, int numsfeatures = 0);
	void filterFeatureByIG(DataSet* dataset, std::vector<PFCOperators>& fcopers, std::vector<std::pair<PFeatureInfo, PFCOperators>>&, int leftfeatures = -1);
	void saveData(DataSet*& dataset);
	void InitLog();
}//EaSFE
