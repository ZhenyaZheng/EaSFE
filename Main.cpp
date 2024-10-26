#include <EaSFE/EaSFE.h>


int main(int argc, char* argv[])
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	string propertypath = "../config/property.json";
	if (argc > 1)
		propertypath = argv[1];
	else
	{
		if (propertypath.empty())
		{
			LOG(ERROR) << "Please input the property path!";
			exit(-1);
		}
	}
	
	EaSFE::Property::getProperty()->readProperty(propertypath);
	EaSFE::StopWatch stop;
	EaSFE::InitLog();
	stop.Start();
	LOG(INFO) << "Begin Load Data!";
	auto dataset = EaSFE::LoadTrainData();
	LOG(INFO) << dataset->getName() << " Begin EaSFE!";
	stop.Stop();
	LOG(INFO) << "before startFeatureConstruction cost :" << std::to_string(stop.Elapsed()) << " us.";
	EaSFE::DataSet* testdataset = nullptr;
	
	int maxnumsfeatures = EaSFE::Property::getProperty()->getMaxNumsFeatures();
	dataset = EaSFE::startFeatureConstruction(dataset, testdataset, maxnumsfeatures);
	EaSFE::saveData(dataset);
	EaSFE::saveData(testdataset);
	EaSFE::ClearClass::clear();
	return 0;
}
