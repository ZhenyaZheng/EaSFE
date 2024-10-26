#include "EaSFE/Tools.h"


namespace EaSFE {

    void resetFCOperatorID(std::vector<PFCOperators>& pfcopers)
    {
        for (int i = 0; i < pfcopers.size(); ++i)
            pfcopers[i]->setFCOperID(i);
    }

    void getDistributeAvg(std::vector<MyDataType>& allbestscores, bool isparallel, const PFCOperators & fcops)
    {
        for (int index = 0; index < allbestscores.size(); ++index)
        {
            MyDataType score = allbestscores[index];
#ifdef USE_MPICH
            int num_process = 0, process_id = 0;
            MPI_Comm_size(MPI_COMM_WORLD, &num_process);
            MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
            if (isparallel)
            {
                globalVar::getglobalVar()->getMutex()->lock();
                auto fcoper_id = globalVar::getglobalVar()->getFCOperID();
                bool is_receive = globalVar::getglobalVar()->getIsReceive();
                if (!process_id) fcoper_id = fcops->getFCOperID();
                if (!process_id || (process_id && !is_receive))
                {
                    MPI_Bcast(&fcoper_id, 1, MPI_INT, 0, MPI_COMM_WORLD);
                    globalVar::getglobalVar()->setFCOperID(fcoper_id);
                    is_receive = true;
                    globalVar::getglobalVar()->setIsReceive(is_receive);
                }
                if (process_id)
                {
                    while (fcops->getFCOperID() != fcoper_id)
                    {
                        globalVar::getglobalVar()->getMutex()->unlock();
                        globalVar::getglobalVar()->getMutex()->lock();
                        fcoper_id = globalVar::getglobalVar()->getFCOperID();
                        is_receive = globalVar::getglobalVar()->getIsReceive();
                        if (!is_receive)
                        {
                            MPI_Bcast(&fcoper_id, 1, MPI_INT, 0, MPI_COMM_WORLD);
                            globalVar::getglobalVar()->setFCOperID(fcoper_id);
                            is_receive = true;
                            globalVar::getglobalVar()->setIsReceive(is_receive);
                        }
                    }
                }

            }
            

            MyDataType* nums = new MyDataType[num_process];
            memset(nums, 0, sizeof(MyDataType) * num_process);
            MPI_Barrier(MPI_COMM_WORLD);
            if (sizeof(MyDataType) == sizeof(float))
                MPI_Gather(&score, 1, MPI_FLOAT, nums, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
            else MPI_Gather(&score, 1, MPI_DOUBLE, nums, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            MyDataType sum = 0;
            if (!process_id)
            {
                for (int i = 0; i < num_process; ++i)
                    sum += nums[i];
                score = sum / num_process;
            }
            if (sizeof(MyDataType) == sizeof(float))
                MPI_Bcast(&score, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
            else MPI_Bcast(&score, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            delete[] nums;
            allbestscores[index] = score;
            if (isparallel)
            {
                globalVar::getglobalVar()->setIsReceive(false);
                globalVar::getglobalVar()->getMutex()->unlock();
                MPI_Barrier(MPI_COMM_WORLD);
            }
#else
            LOG(ERROR) << "getDistributeAvg Please USE_MPICH = ON when CMake this project, or you cannot set Property::getProperty()->setDistributedNodes() > 1";
            exit(-1);
#endif // USE_MPICH
        }
    }

    void GetFileNames(string path, std::vector<string>& filenames)
    {
        DIR* pDir;
        struct dirent* ptr;
        if (!(pDir = opendir(path.c_str()))) {
            // LOG(INFO) << path << " folder doesn't Exist!";
            return;
        }
        while ((ptr = readdir(pDir)) != 0) {
            if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
                filenames.push_back(ptr->d_name);
            }
        }
        closedir(pDir);
        std::sort(filenames.begin(), filenames.end());
    }

    std::vector<std::vector<PFeatureInfo>>getFeatureCombination(DataSet* dataset, int num)
    {
        std::vector<std::vector<PFeatureInfo>> result;
        if (num == 0)
        {
            std::vector<PFeatureInfo> temp;
            result.push_back(temp);
            return result;
        }
        Combination abat = Combination(dataset->getFeatureSize(false), num);
        while (abat.hasNext())
        {
            std::vector<int> temp = abat.getNext();
            std::vector<PFeatureInfo> temp2;
            for (int i = 0; i < temp.size(); ++i)
            {
                PFeatureInfo temp3 = dataset->getFeature(temp[i]);
                temp2.push_back(temp3);
            }
            result.push_back(temp2);
            if (result.size() > 1000000)
				break;
        }
        return result;
    }

    bool getOperatorList(const std::vector<string>& opernames, std::vector<FCOperator*>& operatorlist)
    {
        try
        {
            for (int i = 0; i < opernames.size(); ++i)
            {
                FCOperator* instance = nullptr;
                if (opernames[i].find("GroupByTime")!= -1)
                {
                    MyDataType time = 0;
                    try {
                        auto index = opernames[i].find('_');
                        if (index != -1)
                        {
                            time = static_cast<MyDataType>(stoi(opernames[i].substr(index + 1, opernames[i].size())));
                            instance = reinterpret_cast<FCOperator*>(DynBase::CreateObject(opernames[i].substr(0, index)));
                            reinterpret_cast<GroupByTime*>(instance)->setWindow(time);
                        }
                        else instance = reinterpret_cast<FCOperator*>(DynBase::CreateObject(opernames[i]));
                    }
                    catch (exception& e)
                    {
                        time = 0;
                        LOG(ERROR) << "getOperatorList Error: " << e.what();
                        continue;
                    }

                }
                else  instance = reinterpret_cast<FCOperator*>(DynBase::CreateObject(opernames[i]));
                operatorlist.push_back(instance);
            }
        }
        catch (MyExcept& e)
        {
            LOG(ERROR) << e.getMsg();
            return false;
        }
        return true;
    }

    bool overlapexists(std::vector<PFeatureInfo>& sourcefeatures, std::vector<PFeatureInfo>& targetfeatures)
    {
        std::unordered_map<string, int> sourcesans;
        for (auto& i : sourcefeatures)
        {
            sourcesans[i->getName()]++;
            for (auto& ians : i->getSourceFeatures())
                sourcesans[ians->getName()]++;
            for (auto& ians : i->getTargetFeatures())
                sourcesans[ians->getName()]++;
        }
        for (auto& i : targetfeatures)
        {
            if (sourcesans[i->getName()] > 0)return true;
            sourcesans[i->getName()]++;
            for (auto& ians : i->getSourceFeatures())
            {
                if (sourcesans[ians->getName()] > 0)return true;
                sourcesans[ians->getName()]++;
            }
            for (auto& ians : i->getTargetFeatures())
            {
                if (sourcesans[ians->getName()] > 0)return true;
                sourcesans[ians->getName()]++;
            }
        }
        return false;
    }

    std::vector<PFeatureInfo> getTopRankingDiscreteFeature(DataSet* dataset, int numofreturn)
    {
        std::multimap<MyDataType, PFeatureInfo> igscoreprefeature;
        std::vector<PFeatureInfo> result;
        auto n = dataset->getFeatureSize(false);
        Filter igfe;
        for (int i = 0; i < n; ++i)
        {
            auto featureinfo = dataset->getFeature(i);
            if (featureinfo->getType() != OutType::Discrete)continue;
            featureinfo = dataset->getFeature(i, true);
            MyDataType score = 0.0;
            for (int dataid = 0; dataid < dataset->getNumID(); ++dataid)
            {
                dataset->setID(dataid);
                dataset->deserialize();
                std::vector<PFeatureInfo> templist;
                templist.push_back(featureinfo);
                igfe.init(templist);
                score += igfe.produceScore(dataset);
            }
            dataset->clearFeatureData(i);
            igscoreprefeature.insert({ score, featureinfo });
        }
        for (auto& it : igscoreprefeature)
        {
            if (result.size() >= numofreturn)
                return result;
            result.push_back(it.second);
        }
        return result;
    }

    std::vector<PFCOperators> getOperators(DataSet* dataset, int maxcombinations, int opertype, std::vector<PFeatureInfo> mustincludefeature, bool reducenumoffeature)
    {
        auto i = maxcombinations;
        std::vector<FCOperator*>operatorlist;
        if (opertype == 1)
            getOperatorList(Property::getProperty()->getUnaryOperators(), operatorlist);
        else
            getOperatorList(Property::getProperty()->getMutilOperators(), operatorlist);
        std::unordered_map<string, bool> addmultiopereliminate;
        std::vector<PFCOperators> operatorslist;
        if (reducenumoffeature && mustincludefeature.size() == 0)
        {
            if (dataset->getFeatureSize(false) > 60)
                mustincludefeature = getTopRankingDiscreteFeature(dataset, 10);
        }
        auto maxopernums = Property::getProperty()->getMaxFCOperator();
        while (i > 0)
        {
            if(i > 1 && dataset->getDiscreteFeatureSize() < 1)
            {
                i--;
                continue;
            }
            auto featurecombination = getFeatureCombination(dataset, i);
            LOG(INFO) << "featurecombination size: " << featurecombination.size() << " i: " << i;
            auto  datafeatures = dataset->getFeatures();
            int post0 = 0;
            for (auto& fc : featurecombination)
            {
                if (Property::getProperty()->getMaxFCOperator() >= 0 && operatorslist.size() >= Property::getProperty()->getMaxFCOperator())
                    break;
                if (fc.size() == 0)
                    continue;
                if (mustincludefeature.size() > 0)
                {
                    int numinclude = 0;
                    for (auto& fea : fc)
                        if (fea->in(mustincludefeature))numinclude++;
                    if (numinclude == 0)continue;
                }
                int post1 = 0;
                for (auto& op : operatorlist)
                {
                    if (Property::getProperty()->getMaxFCOperator() >= 0 && operatorslist.size() >= Property::getProperty()->getMaxFCOperator())
                        break;
                    if (op->isMatch(fc, std::vector<PFeatureInfo>()))
                    {
                        operatorslist.emplace_back(new FCOperators(fc, std::vector<PFeatureInfo>(), op->copy(), nullptr, false));
                        if (operatorslist.size() % 500 == 0)
                            LOG(DEBUG) << "operatorslist size: " << operatorslist.size() << " / " << Property::getProperty()->getMaxFCOperator();
                    }
                    if (opertype == 1) continue;
                    int post2 = 0;
                    for (auto& datafeature : datafeatures)
                    {
                        if (Property::getProperty()->getMaxFCOperator() >= 0 && operatorslist.size() >= Property::getProperty()->getMaxFCOperator())
                        	break;
                        std::vector<PFeatureInfo> datafeaturelist;
                        datafeaturelist.push_back(datafeature);
                        
                        if (op->isMatch(fc, datafeaturelist))
                        {
                            if (overlapexists(fc, datafeaturelist))
                                continue;
                            else
                            {
                                auto name = op->getName();
                                if (name == "AddOperator" || name == "MultiplyOperator" || name == "MaxOperator" || name == "MinOperator")
                                {
                                    string keyname = fc[0]->getName() + datafeaturelist[0]->getName() + name;
                                    string keyname1 = datafeaturelist[0]->getName() + fc[0]->getName() + name;
                                    if (addmultiopereliminate.find(keyname) != addmultiopereliminate.end() || addmultiopereliminate.find(keyname1) != addmultiopereliminate.end())continue;
                                    else addmultiopereliminate[keyname] = true;
                                }
                                operatorslist.emplace_back(new FCOperators(fc, datafeaturelist, op->copy(), nullptr, false));
                                if (operatorslist.size() % 500 == 0)
                                    LOG(DEBUG) << "operatorslist size: " << operatorslist.size() << " / " << Property::getProperty()->getMaxFCOperator();
                            }
                        }
                        
                    }
                }
            }
            i--;
        }
        for (int i = 0; i < operatorlist.size(); ++i)delete operatorlist[i], operatorlist[i] = nullptr;
        if (Property::getProperty()->getDatasetType() == DataType::LibSVMCF || Property::getProperty()->getDatasetType() == DataType::LibSVMRG)
            return operatorslist;
        operatorlist.clear();
        /*std::vector<PFCOperators>addoperatorslist;
        std::vector<FCOperator*>  operatorlist2;
        getOperatorList(Property::getProperty()->getUnaryOperators(), operatorlist2);
        for (int index = 0;index < operatorslist.size();++ index)
        {
            auto& opslt = operatorslist[index];
            if (opslt->getOperator()->getOperatorType() != OperatorType::Unary)
            {
                for (auto& oper : operatorlist2)
                {
                    if (oper->requireType() == opslt->getOperator()->getType())
                        addoperatorslist.emplace_back(new FCOperators(opslt->getSourceFeatures(), opslt->getTargetFeatures(), opslt->getOperator()->copy(), oper->copy(), true));
                }
            }
        }
        for (int i = 0; i < operatorlist2.size(); ++i)delete operatorlist2[i], operatorlist2[i] = nullptr;
        operatorlist2.clear();
        operatorslist.insert(operatorslist.end(), addoperatorslist.begin(), addoperatorslist.end());*/
        if (maxopernums > 0 && maxopernums < operatorslist.size())
        {
            //std::random_device rd;
            //std::mt19937 rng(rd());
            //std::shuffle(operatorslist.begin(), operatorslist.end(), 42);
            operatorslist.erase(operatorslist.begin() + maxopernums, operatorslist.end());
        }
        Property::getProperty()->setMaxFCOperator(maxopernums - operatorslist.size());
        return operatorslist;
    }

    PFeatureInfo generateFeature(DataSet* dataset, PFCOperators fcops, bool needPre, bool needgeneratesecondsources, bool needfindindataset, bool filtersinglevec)
    {
        auto sourcefeature = fcops->getSourceFeatures();
        auto targetfeature = fcops->getTargetFeatures();
        int numofinstances = dataset->getInstancesOfFeature(1);
        if (dataset->getNumID() > 1) numofinstances = dataset->getInstancesOfFeature(3);
        if(dataset->getNumID() > 1 && needfindindataset)
        {
            sourcefeature.clear();
            targetfeature.clear();
            for (auto& pfeainfo : fcops->getSourceFeatures())
            {
                PFeatureInfo newfeatureinfo = new FeatureInfo(*pfeainfo);
                Feature *feature = nullptr;
                switch (pfeainfo->getType())
                {
                    case OutType::Numeric: feature = new NumericFeature(numofinstances); break;
                    case OutType::Discrete: feature = new DiscreteFeature(numofinstances, pfeainfo->getNumsOfValues()); break;
                default:
                    break;
                }
                newfeatureinfo->setFeature(feature);
                sourcefeature.push_back(newfeatureinfo);
            }
            for (auto& pfeainfo : fcops->getTargetFeatures())
            {
				PFeatureInfo newfeatureinfo = new FeatureInfo(*pfeainfo);
				Feature *feature = nullptr;
				switch (pfeainfo->getType())
				{
				case OutType::Numeric: feature = new NumericFeature(numofinstances); break;
				case OutType::Discrete: feature = new DiscreteFeature(numofinstances, pfeainfo->getNumsOfValues()); break;
				default:
					break;
				}
				newfeatureinfo->setFeature(feature);
                targetfeature.push_back(newfeatureinfo);
			}
            for (int i = 0; i < dataset->getNumID(); ++i)
            {
				dataset->setID(i);
				dataset->deserialize();
                auto numinstancespredata = dataset->getInstancesOfFeature(1);
                auto startindex = dataset->getDatasetIndex()[i];
                for (auto& sf : sourcefeature)
                {
                    auto sizeofval = sizeof(MyDataType);
                    if (sf->getType() == OutType::Discrete)sizeofval = sizeof(int);
                    auto feainfo = dataset->getFeatureFromOld(sf);
                    memcpy(sf->getFeature()->getValue(startindex), feainfo->getFeature()->getValues(), numinstancespredata * sizeofval);
                    if((Property::getProperty()->getDatasetType() == DataType::LibSVMCF || Property::getProperty()->getDatasetType() == DataType::LibSVMRG) && needfindindataset)
                        dataset->clearFeatureData(feainfo);
                }

                for (const auto& sf : targetfeature)
                {
                    auto sizeofval = sizeof(MyDataType);
					if (sf->getType() == OutType::Discrete)sizeofval = sizeof(int);
					auto feainfo = dataset->getFeatureFromOld(sf);
					memcpy(sf->getFeature()->getValue(startindex), feainfo->getFeature()->getValues(), numinstancespredata * sizeofval);
                    if ((Property::getProperty()->getDatasetType() == DataType::LibSVMCF || Property::getProperty()->getDatasetType() == DataType::LibSVMRG) && needfindindataset)
                        dataset->clearFeatureData(feainfo);
                }

            }
		}
        else if (needfindindataset)
        {
            sourcefeature.clear();
            targetfeature.clear();
            for (const auto& sf : fcops->getSourceFeatures())
                sourcefeature.push_back(dataset->getFeatureFromOld(sf));

            for (const auto& sf : fcops->getTargetFeatures())
                targetfeature.push_back(dataset->getFeatureFromOld(sf));
        }
        PFeatureInfo newfeatureinfo = nullptr;
        if (Property::getProperty()->getDistributedNodes() > 1)
        {
#ifdef USE_MPICH
            numofinstances = dataset->getInstancesOfFeature(2);
            auto newsourcefeature = std::vector<PFeatureInfo>();
            auto newtargetfeature = std::vector<PFeatureInfo>();
            int num_process = 0, process_id = 0;
            MPI_Comm_size(MPI_COMM_WORLD, &num_process);
            MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
            int* counts = new int[num_process];
            memset(counts, 0, num_process * sizeof(int));
            int* sides = new int[num_process];
            memset(sides, 0, num_process * sizeof(int));
            int onevalsize = 4;
            int namelength = 0;
            int instanceoffeature = dataset->getInstancesOfFeature();
            
            globalVar::getglobalVar()->getMutex()->lock();
            auto fcoper_id = globalVar::getglobalVar()->getFCOperID();
            bool is_receive = globalVar::getglobalVar()->getIsReceive();
            if (!process_id) fcoper_id = fcops->getFCOperID();
            if (!process_id || (process_id && !is_receive))
            {
                MPI_Bcast(&fcoper_id, 1, MPI_INT, 0, MPI_COMM_WORLD);
                globalVar::getglobalVar()->setFCOperID(fcoper_id);
                is_receive = true;
                globalVar::getglobalVar()->setIsReceive(is_receive);
            }
            if (process_id)
            {
                while (fcops->getFCOperID() != fcoper_id)
                {
                    globalVar::getglobalVar()->getMutex()->unlock();
                    globalVar::getglobalVar()->getMutex()->lock();
                    fcoper_id = globalVar::getglobalVar()->getFCOperID();
                    is_receive = globalVar::getglobalVar()->getIsReceive();
                    if (!is_receive)
                    {
                        MPI_Bcast(&fcoper_id, 1, MPI_INT, 0, MPI_COMM_WORLD);
                        globalVar::getglobalVar()->setFCOperID(fcoper_id);
                        is_receive = true;
                        globalVar::getglobalVar()->setIsReceive(is_receive);
                    }
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
            for (int i = 0; i < sourcefeature.size() + targetfeature.size(); ++i)
            {
                PFeatureInfo oldfeainfo = nullptr;
                if (i < sourcefeature.size())oldfeainfo = sourcefeature[i];
                else oldfeainfo = targetfeature[i - sourcefeature.size()];
                if (needfindindataset) oldfeainfo = dataset->getFeatureFromOld(oldfeainfo);
                auto oldfea = oldfeainfo->getFeature();
                Feature* feature = nullptr;
                
                if (!process_id)
                {
                    switch (oldfea->getType())
                    {
                    case FeatureType::Numeric: feature = new NumericFeature(numofinstances); onevalsize = sizeof(MyDataType); break;
                    case FeatureType::Discrete: feature = new DiscreteFeature(numofinstances, oldfea->getNumsOfValues()); onevalsize = sizeof(int); break;
                    case FeatureType::Date: feature = new DateFeature(numofinstances); onevalsize = sizeof(Date); break;
                    case FeatureType::String: feature = new StringFeature(numofinstances); onevalsize = sizeof(std::string); break;
                    }
                }
                MPI_Bcast(&onevalsize, 1, MPI_INT, 0, MPI_COMM_WORLD);

                int senddata = instanceoffeature * onevalsize;
                MPI_Gather(&senddata, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
                int post = 0;
                for (int j = 0; j < num_process; ++j)
                    sides[j] = post, post += counts[j];
                
                char* newfeaturestr = nullptr;
                if(!process_id)newfeaturestr = reinterpret_cast<char*>(feature->getValues());
                char* startfeaturestr = reinterpret_cast<char*>(oldfea->getValues());

                MPI_Gatherv(startfeaturestr, senddata, MPI_CHAR, newfeaturestr, counts, sides, MPI_CHAR, 0, MPI_COMM_WORLD);
                if ((Property::getProperty()->getDatasetType() == DataType::LibSVMCF || Property::getProperty()->getDatasetType() == DataType::LibSVMRG) && needfindindataset)
                    dataset->clearFeatureData(oldfeainfo);
                PFeatureInfo feainfo = new FeatureInfo(feature, std::vector<PFeatureInfo>(), std::vector<PFeatureInfo>(), oldfeainfo->getType(), oldfeainfo->getName(), false);
                if(i < sourcefeature.size())newsourcefeature.push_back(feainfo);
                else newtargetfeature.push_back(feainfo);
            }
            if (newtargetfeature.size() != targetfeature.size() || newsourcefeature.size() != sourcefeature.size())
            {
                LOG(ERROR) << "generateFeature error: targetfeature.size() or sourcefeature.size() not match";
                return nullptr;
            }
            PFeatureInfo allnewfeatureinfo = nullptr;
            if (!process_id)
            {
                if (needPre)
                    fcops->getOperator()->preGenerateFeature(dataset, newsourcefeature, newtargetfeature, false, numofinstances);
                if (!needgeneratesecondsources)
                {
                    fcops->getOperator()->clear();
                    allnewfeatureinfo =  new FeatureInfo();
                }

                if (fcops->getSecondOperator() == nullptr)
                    allnewfeatureinfo = fcops->getOperator()->generateFeature(dataset, newsourcefeature, newtargetfeature);
                else
                {
                    std::vector<PFeatureInfo> secondsourcefeatures;
                    secondsourcefeatures.push_back(fcops->getOperator()->generateFeature(dataset, newsourcefeature, newtargetfeature));
                    fcops->getSecondOperator()->preGenerateFeature(dataset, secondsourcefeatures, std::vector<PFeatureInfo>(), true, numofinstances);
                    allnewfeatureinfo = fcops->getSecondOperator()->generateFeature(dataset, secondsourcefeatures, std::vector<PFeatureInfo>());

                }
                namelength = allnewfeatureinfo->getName().size();
            }
            bool isfilter = filtersinglevec;
            if (!process_id && isfilter)
            {
                int threadnum = Property::getProperty()->getThreadNum();
                if (fcops->getOutType() == OutType::Numeric)
                    isfilter &= MyMath<MyDataType>::parallelisSingleVec(reinterpret_cast<MyDataType*>(allnewfeatureinfo->getFeature()->getValues()), numofinstances, threadnum);
                else if (fcops->getOutType() == OutType::Discrete)
                    isfilter &= MyMath<int>::parallelisSingleVec(reinterpret_cast<int*>(allnewfeatureinfo->getFeature()->getValues()), numofinstances, threadnum);
            }
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Bcast(&isfilter, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
            if (needgeneratesecondsources && !isfilter)
            {
                int numofvalues = 0;
                if (!process_id) numofvalues = allnewfeatureinfo->getFeature()->getNumsOfValues();
                MPI_Bcast(&numofvalues, 1, MPI_INT, 0, MPI_COMM_WORLD);
                Feature* newfeature = nullptr;
                switch (fcops->getOutType())
                {
                case OutType::Numeric: newfeature = new NumericFeature(instanceoffeature); onevalsize = sizeof(MyDataType); break;
                case OutType::Discrete: newfeature = new DiscreteFeature(instanceoffeature, numofvalues); onevalsize = sizeof(int); break;
                case OutType::Date: newfeature = new DateFeature(instanceoffeature); onevalsize = sizeof(Date); break;
                case OutType::String: newfeature = new StringFeature(instanceoffeature); onevalsize = sizeof(std::string); break;
                }
                MPI_Bcast(&namelength, 1, MPI_INT, 0, MPI_COMM_WORLD);
                string name(namelength, ' ');
                int senddata = instanceoffeature * onevalsize;
                char* newfeaturestr = reinterpret_cast<char*>(newfeature->getValues());
                char* startfeaturestr = nullptr;
                if (!process_id) { startfeaturestr = reinterpret_cast<char*>(allnewfeatureinfo->getFeature()->getValues()); name = allnewfeatureinfo->getName(); }
                MPI_Bcast(const_cast<char*>(name.c_str()), namelength, MPI_CHAR, 0, MPI_COMM_WORLD);
                MPI_Gather(&senddata, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
                int post = 0;
                for (int i = 0; i < num_process; ++i)
                    sides[i] = post, post += counts[i];
                MPI_Scatterv(startfeaturestr, counts, sides, MPI_CHAR, newfeaturestr, senddata, MPI_CHAR, 0, MPI_COMM_WORLD);
                if (fcops->getSecondOperator())
                {
                    Feature* newsoufeature = nullptr;
                    switch (fcops->getOutType())
                    {
                    case OutType::Numeric: newsoufeature = new NumericFeature(numofinstances); onevalsize = sizeof(MyDataType); break;
                    case OutType::Discrete: newsoufeature = new DiscreteFeature(numofinstances, numofvalues); onevalsize = sizeof(int); break;
                    case OutType::Date: newsoufeature = new DateFeature(numofinstances); onevalsize = sizeof(Date); break;
                    case OutType::String: newsoufeature = new StringFeature(numofinstances); onevalsize = sizeof(std::string); break;
                    }
                    char* sendstartstr = nullptr, *recivestartstr = reinterpret_cast<char*>(newsoufeature->getValues());
                    if (!process_id)
                    {
                        auto soufeainfo = allnewfeatureinfo->getSourceFeatures()[0];
                        sendstartstr = reinterpret_cast<char*>(soufeainfo->getFeature()->getValues());
                        namelength = soufeainfo->getName().size();
                    }
                    senddata = instanceoffeature * onevalsize;
                    MPI_Bcast(&namelength, 1, MPI_INT, 0, MPI_COMM_WORLD);

                    string souname(namelength, ' ');
                    MPI_Bcast(const_cast<char*>(souname.c_str()), namelength, MPI_CHAR, 0, MPI_COMM_WORLD);
                    MPI_Gather(&senddata, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
                    post = 0;
                    for (int i = 0; i < num_process; ++i)
                        sides[i] = post, post += counts[i];
                    MPI_Scatterv(sendstartstr, counts, sides, MPI_CHAR, recivestartstr, senddata, MPI_CHAR, 0, MPI_COMM_WORLD);
                    PFeatureInfo soufeatureinfo = new FeatureInfo(newsoufeature, sourcefeature, targetfeature, fcops->getOperator()->getType(), souname, false);
                    std::vector<PFeatureInfo> soufeatureinfos;
                    soufeatureinfos.push_back(soufeatureinfo);
                    newfeatureinfo = new FeatureInfo(newfeature, soufeatureinfos, std::vector<PFeatureInfo>(), fcops->getOutType(), name, true);
                }
                else newfeatureinfo = new FeatureInfo(newfeature, sourcefeature, targetfeature, fcops->getOutType(), name, false);
                
            }
            else if(!isfilter)newfeatureinfo = new FeatureInfo();
            else newfeatureinfo = nullptr;

            is_receive = false;
            globalVar::getglobalVar()->setIsReceive(is_receive);
            globalVar::getglobalVar()->getMutex()->unlock();

            delete [] counts;
            delete[] sides;
            for (int i = 0; i < sourcefeature.size() + targetfeature.size(); ++i)
            {
                if (i < sourcefeature.size())
                {
                    newsourcefeature[i]->clear();
                    delete newsourcefeature[i];
                    newsourcefeature[i] = nullptr;
                }
                else
                {
                    newtargetfeature[i - sourcefeature.size()]->clear();
                    delete newtargetfeature[i - sourcefeature.size()];
                    newtargetfeature[i - sourcefeature.size()] = nullptr;
                }
            }
            if(allnewfeatureinfo)allnewfeatureinfo->clear();
            delete allnewfeatureinfo;

#else
    LOG(ERROR) << "generateFeature Please USE_MPICH = ON when CMake this project, or you cannot set Property::getProperty()->setDistributedNodes() > 1";
#endif
        }
        else
        {
            if (needPre)
                fcops->getOperator()->preGenerateFeature(dataset, sourcefeature, targetfeature, false, numofinstances);
            if (!needgeneratesecondsources)
            {
                fcops->getOperator()->clear();
                newfeatureinfo = new FeatureInfo();
            }
            else if (fcops->getSecondOperator() == nullptr)
            {
                newfeatureinfo = fcops->getOperator()->generateFeature(dataset, sourcefeature, targetfeature);
                newfeatureinfo->setNumsOfValues(newfeatureinfo->getFeature()->getNumsOfValues());
            }
            else
            {
                std::vector<PFeatureInfo> secondsourcefeatures;
                secondsourcefeatures.push_back(fcops->getOperator()->generateFeature(dataset, sourcefeature, targetfeature));
                fcops->getSecondOperator()->preGenerateFeature(dataset, secondsourcefeatures, std::vector<PFeatureInfo>(), true, numofinstances);
                newfeatureinfo = fcops->getSecondOperator()->generateFeature(dataset, secondsourcefeatures, std::vector<PFeatureInfo>());
                newfeatureinfo->setNumsOfValues(newfeatureinfo->getFeature()->getNumsOfValues());
            }
        }
        if (Property::getProperty()->getDatasetType() == DataType::LibSVMCF || Property::getProperty()->getDatasetType() == DataType::LibSVMRG)
        {
            if (needfindindataset && dataset->getNumID() == 1)
            {
                clearFeatureData(dataset, sourcefeature);
                clearFeatureData(dataset, targetfeature);
            }
        }
        if (dataset->getNumID() > 1 && needfindindataset)
        {
            for (auto& sf : sourcefeature)
			{
				sf->clear();
				delete sf;
				sf = nullptr;
			}
            for (auto& sf : targetfeature)
            {
                sf->clear();
                delete sf;
                sf = nullptr;
            }
        }
        return newfeatureinfo;
    }

    string getNowTime()
    {
        auto now = std::chrono::system_clock::now();
        
        uint64_t dis_millseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
            - std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * 1000;
        time_t tt = std::chrono::system_clock::to_time_t(now);
        auto time_tm = localtime(&tt);
        char strTime[125] = { 0 };
        sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d %03d", time_tm->tm_year + 1900,
            time_tm->tm_mon + 1, time_tm->tm_mday, time_tm->tm_hour,
            time_tm->tm_min, time_tm->tm_sec, (int)dis_millseconds);
        return  string(strTime) ;
    }

    bool addFeatureToDataset(DataSet* dataset, std::vector<PFCOperators> &operators, bool ismutilthread, bool isiter)
    {
        if (ismutilthread && dataset->getNumID() == 1)
        {
            #pragma omp parallel for shared(dataset)
            for (int i = 0; i < operators.size(); ++i)
            {
                if (i % 1000 == 0)
                    LOG(INFO) << "Add features to dataset: " << i << " / " << operators.size();
                PFeatureInfo newfeature = nullptr;
                if(isiter)
                    newfeature = generateFeature(dataset, operators[i]);
                else newfeature = generateFeature(dataset, operators[i]);
                #pragma omp critical
                {
                    dataset->addFeature(newfeature);
                }
            }
        }
        else
        {
            for (int i = 0; i < operators.size(); ++i)
            {
                if (i % 10000 == 0)
                    LOG(INFO) << "Add features to dataset: " << std::to_string(i) << " / " << std::to_string(operators.size());

                auto featureinfo = generateFeature(dataset, operators[i]);
                auto numid = dataset->getNumID();
                if (numid == 1)
                {
                    dataset->addFeature(featureinfo);
                    continue;
                }
                for (int dataid = 0; dataid < numid; ++dataid)
                {
                    int numofinstances = dataset->getNumOfDatasetInstances()[dataid];
                    int maxnumofinstances = dataset->getNumOfDatasetInstances()[0];
                    if(Property::getProperty()->getDatasetType() == DataType::LibSVMCF || Property::getProperty()->getDatasetType() == DataType::LibSVMRG)
						maxnumofinstances = numofinstances;
                    int startindex = dataset->getDatasetIndex()[dataid];
                    dataset->setID(dataid);
                    dataset->deserialize(0);
                    dataset->setInstancesOfFeature(numofinstances);
                    PFeatureInfo newfeature = new FeatureInfo(*featureinfo);
                    newfeature->setFeature(nullptr);
                    newfeature->setIsSecondFeature(false);
                    Feature* feature = nullptr;
                    auto sizeofval = sizeof(MyDataType);
                    switch (featureinfo->getType())
                    {
                        case OutType::Numeric: feature = new NumericFeature(std::max(maxnumofinstances, numofinstances)); break;
                        case OutType::Discrete: feature = new DiscreteFeature(std::max(maxnumofinstances, numofinstances), featureinfo->getNumsOfValues()); sizeofval = sizeof(int); break;
                    }
                    newfeature->setFeature(feature);
                    
					memcpy(newfeature->getFeature()->getValues(), featureinfo->getFeature()->getValue(startindex), sizeofval * numofinstances);
						
                    if (dataid == numid - 1)
                    {
                        dataset->addFeature(newfeature);
                        dataset->serialize(dataset->getFeatureSize() - 1);
                    }
                    else
                    {
                        dataset->addFeature(newfeature);
                        dataset->serialize(dataset->getFeatureSize() - 1);
                        auto feainfo = dataset->popFeature(true);
                        feainfo->clear();
                        delete feainfo;
                    }
                }
                featureinfo->clear();
                delete featureinfo;
                featureinfo = nullptr;
            }
        }
        dataset->relloc();
        return true;
    }

    void clearFeatureData(DataSet* dataset, std::vector<PFeatureInfo>& featureinfos)
	{
		for (auto& featureinfo : featureinfos)
			dataset->clearFeatureData(featureinfo);
	}

  
    char* initMyMemory(const int n)
    {
        char* ptr = nullptr;
        ptr = new char[n + ADDMEMORY];
        if (!ptr)
            LOG(ERROR) << "initMyMemory Error: new char error!";
        return ptr;
    }

    bool clearMyMemory(char* &ptr)
    {
        delete[] ptr;
        ptr = nullptr;
        return true;
    }

    bool addFeatureToDataset(DataSet* dataset, std::vector<std::pair<PFeatureInfo, PFCOperators>>& featureinfoopers, bool ismutilthread)
    {
        if (ismutilthread && dataset->getNumID() == 1)
        {
			#pragma omp parallel for shared(dataset)
            for (int i = 0; i < featureinfoopers.size(); ++i)
            {
				if (i % 1000 == 0)
                    LOG(INFO) << "Add features to dataset: " << std::to_string(i) << " / " << std::to_string(featureinfoopers.size());
				#pragma omp critical
                {
					dataset->addFeature(featureinfoopers[i].first);
				}
			}
		}
        else
        {
            for (int i = 0; i < featureinfoopers.size(); ++i)
            {
				if (i % 10000 == 0)
					LOG(INFO) << "Add features to dataset: " << std::to_string(i) << " / " << std::to_string(featureinfoopers.size());
                PFeatureInfo& featureinfo = featureinfoopers[i].first;
                if(featureinfo == nullptr)featureinfo = generateFeature(dataset, featureinfoopers[i].second);
                auto numid = dataset->getNumID();
                if (numid == 1)
                {
                    dataset->addFeature(featureinfo);
                    continue;
                }
                for (int dataid = 0; dataid < numid; ++dataid)
                {
                    int numofinstances = dataset->getNumOfDatasetInstances()[dataid];
                    int maxnumofinstances = dataset->getNumOfDatasetInstances()[0];
                    int startindex = dataset->getDatasetIndex()[dataid];
                    dataset->setID(dataid);
                    dataset->deserialize(0);
                    dataset->setInstancesOfFeature(numofinstances);
                    PFeatureInfo newfeature = new FeatureInfo(*featureinfo);
                    newfeature->setFeature(nullptr);
                    newfeature->setIsSecondFeature(false);
                    Feature* feature = nullptr;
                    auto sizeofval = sizeof(MyDataType);
                    switch (featureinfo->getType())
                    {
                    case OutType::Numeric: feature = new NumericFeature(std::max(maxnumofinstances, numofinstances)); break;
                    case OutType::Discrete: feature = new DiscreteFeature(std::max(maxnumofinstances, numofinstances), featureinfo->getNumsOfValues()); sizeofval = sizeof(int); break;
                    }
                    newfeature->setFeature(feature);

                    memcpy(newfeature->getFeature()->getValues(), featureinfo->getFeature()->getValue(startindex), sizeofval * numofinstances);

                    if (dataid == numid - 1)
                    {
                        dataset->addFeature(newfeature);
                        dataset->serialize(dataset->getFeatureSize() - 1);
                    }
                    else
                    {
                        dataset->addFeature(newfeature);
                        dataset->serialize(dataset->getFeatureSize() - 1);
                        auto feainfo = dataset->popFeature(true);
                        feainfo->clear();
                        delete feainfo;
                    }
                }
                featureinfo->clear();
                delete featureinfo;
                featureinfo = nullptr;
			}
		}
		dataset->relloc();
		return true;
	}

    FCOperators* copyFCOperators(DataSet* dataset, FCOperators* fcos)
    {
        FCOperator* oper = fcos->getOperator()->copy();
        FCOperator* secondoper = nullptr;
        if (fcos->getSecondOperator() != nullptr)
			secondoper = fcos->getSecondOperator()->copy();
        std::vector<PFeatureInfo> sourcefeatures;
        for (auto& fea : fcos->getSourceFeatures())
        {
			auto newfea = dataset->getFeatureFromOld(fea, false);
            if (newfea == nullptr)
            {
				LOG(ERROR) << "copyFCOperators Error: " << fea->getName() << " is not in dataset!";
				return nullptr;
			}
			else
				sourcefeatures.push_back(newfea);
		}
        std::vector<PFeatureInfo> targetfeatures;
        for (auto& fea : fcos->getTargetFeatures())
        {
            auto newfea = dataset->getFeatureFromOld(fea, false);
            if (newfea == nullptr)
            {
				LOG(ERROR) << "copyFCOperators Error: " << fea->getName() << " is not in dataset!";
				return nullptr;
			}
			else
				targetfeatures.push_back(newfea);
        }
        FCOperators* newfcos = new FCOperators(sourcefeatures, targetfeatures, oper, secondoper);
        newfcos->setFScore(fcos->getFScore());
        newfcos->setWScore(fcos->getWScore());
        newfcos->setHasCalc(fcos->getHasCalc());
        return newfcos;
    }

    std::shared_timed_mutex* globalVar::getMutex()
    {
        if (!m_rwmutex)
            m_rwmutex = new  std::shared_timed_mutex;
        return  m_rwmutex;
    }
    bool globalVar::getIsReceive()
    {
        return m_isreceive;
    }
    void globalVar::setIsReceive(bool isreceive)
    {
        m_isreceive = isreceive;
    }
    int globalVar::getFCOperID()
    {
        return m_fcoperid;
    }
    void globalVar::setFCOperID(int fcoperid)
    {
        m_fcoperid = fcoperid;
    }

    std::shared_timed_mutex* globalVar::getMutex2()
    {
        if (!m_rwmutex2)
            m_rwmutex2 = new  std::shared_timed_mutex;
        return  m_rwmutex2;
    }
    bool globalVar::getIsReceive2()
    {
        return m_isreceive2;
    }
    void globalVar::setIsReceive2(bool isreceive)
    {
        m_isreceive2 = isreceive;
    }
    int globalVar::getFCOperID2()
    {
        return m_fcoperid2;
    }
    void globalVar::setFCOperID2(int fcoperid)
    {
        m_fcoperid2 = fcoperid;
    }

    globalVar* globalVar::getglobalVar()
    {
        if (m_instance == nullptr)
        {
            return m_instance = new globalVar();
        }
        else
            return m_instance;
    }
    bool globalVar::freeglobalVar()
    {
        delete m_instance;
        m_instance = nullptr;
        return true;
    }
}//EaSFE