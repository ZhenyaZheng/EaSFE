#include "EaSFE/EaSFE.h"

namespace EaSFE {

    DataSet* clearProcessesfortestdata()
    {
        if (Property::getProperty()->getDistributedNodes() > 1)
        {
#ifdef USE_MPICH
            int process_id;
            MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
            auto numstestdatasets = Property::getProperty()->getNumTempDatasets();
            if (numstestdatasets == 1)
            {
                MPI_Finalize();
                Property::getProperty()->setDistributedNodes(1);
                if (process_id > 0)
                {
                    LOG(INFO) << "process_id: " << process_id << ", clearProcesses: MPI_Finalize!";
                    return nullptr;
                }
            }
#else
            LOG(ERROR) << "clearProcesses Error: USE_MPICH is not defined!";
            exit(-1);
#endif // 
        }
        DataSet* testdataset = nullptr;
        if (EaSFE::Property::getProperty()->getTestDataPath() != "")
        {
            auto targetname = EaSFE::Property::getProperty()->getClassName();
            auto discretename = EaSFE::Property::getProperty()->getDiscreteFeatureName();
            auto datename = EaSFE::Property::getProperty()->getDateFeatureName();
            auto classnum = EaSFE::Property::getProperty()->getTargetClasses();
            auto datasetname = EaSFE::Property::getProperty()->getDatasetName();
            auto testdatapath = EaSFE::Property::getProperty()->getRootPath() + EaSFE::Property::getProperty()->getTestDataPath();
            if(Property::getProperty()->getIsAllNumber())
                testdataset = EaSFE::Load::loadDataAllNumeber(testdatapath, targetname, discretename, datename, classnum, datasetname + "_test", true, false, true, true);
            else testdataset = EaSFE::Load::loadData(testdatapath, targetname, discretename, datename, classnum, datasetname + "_test", true, false, true, true);
        }
        return testdataset;
	}

    DataSet* getValData()
    {
        DataSet* valdataset = nullptr;
        if (EaSFE::Property::getProperty()->getValDataPath() != "")
        {
            auto targetname = EaSFE::Property::getProperty()->getClassName();
            auto discretename = EaSFE::Property::getProperty()->getDiscreteFeatureName();
            auto datename = EaSFE::Property::getProperty()->getDateFeatureName();
            auto classnum = EaSFE::Property::getProperty()->getTargetClasses();
            auto datasetname = EaSFE::Property::getProperty()->getDatasetName();
            auto valdatapath = EaSFE::Property::getProperty()->getRootPath() + EaSFE::Property::getProperty()->getValDataPath();
            if(Property::getProperty()->getIsAllNumber())
                valdataset = EaSFE::Load::loadDataAllNumeber(valdatapath, targetname, discretename, datename, classnum, datasetname + "_val", true, false, true, true);
            else valdataset = EaSFE::Load::loadData(valdatapath, targetname, discretename, datename, classnum, datasetname + "_val", true, false, true, true);
        }
        return valdataset;
    } 

	DataSet* LoadTrainData()
    {
        string datapath = EaSFE::Property::getProperty()->getRootPath() + EaSFE::Property::getProperty()->getDatasetPath();
        string targetname = EaSFE::Property::getProperty()->getClassName();
        string datasetname = EaSFE::Property::getProperty()->getDatasetName();
        std::vector<string> discretename = EaSFE::Property::getProperty()->getDiscreteFeatureName();
        std::vector<string> datename = EaSFE::Property::getProperty()->getDateFeatureName();
        int classnum = EaSFE::Property::getProperty()->getTargetClasses();
        DataSet* dataset = nullptr;
        if(Property::getProperty()->getIsAllNumber())
            dataset = EaSFE::Load::loadDataAllNumeber(datapath, targetname, discretename, datename, classnum, datasetname, true, true, true);
        else dataset = EaSFE::Load::loadData(datapath, targetname, discretename, datename, classnum, datasetname, true, true, true);
        auto valdataset = getValData();
		dataset->megerDataSet(valdataset);
		if(valdataset != nullptr) delete valdataset, valdataset = nullptr;
        return dataset;
    }

    void selectFeatureFromScore(double* scores, std::vector<std::pair<PFeatureInfo, PFCOperators>> & featurefcops, int orinumfeatures, int leftfeatures)
    {
        std::vector<std::pair<double, int>> scoreindex;
        for (int i = 0; i < featurefcops.size(); ++i)
        {
            scoreindex.push_back({ scores[i+orinumfeatures], i });
        }
        std::sort(scoreindex.begin(), scoreindex.end(), [](const std::pair<double, int>& a, const std::pair<double, int>& b) {return a.first > b.first; });
        if (leftfeatures > 0 && leftfeatures < scoreindex.size())
            scoreindex.resize(leftfeatures);
        std::vector<std::pair<PFeatureInfo, PFCOperators>> result;
        vector<bool> selected(featurefcops.size(), false);
        for (auto& index : scoreindex)
        {
            result.push_back(featurefcops[index.second]);
            selected[index.second] = true;
        }
        // release other don't selected features
        for (int i = 0; i < featurefcops.size(); ++i)
        {
            if (!selected[i])
            {
                auto& featureinfo = featurefcops[i].first;
                featureinfo->clear();
                delete featureinfo;
                featureinfo = nullptr;
            }
        }
        featurefcops = result;
    }

    DataSet* startFeatureConstruction(DataSet* &dataset, DataSet* &testdataset, int numsfeatures)
    {
        StopWatch stop;
        stop.Start();
        auto maxselectionnum = Property::getProperty()->getMaxSelectionNum();
        std::vector<PFCOperators> vec_operators1 = getOperators(dataset, 1, 1); // get all operators type = 1
        std::vector<PFCOperators> vec_operators2 = getOperators(dataset, maxselectionnum, 2); //get all operators type = 2
        std::vector<PFCOperators> vec_operators(std::move(vec_operators1));
        vec_operators.insert(vec_operators.end(), vec_operators2.begin(), vec_operators2.end());
        resetFCOperatorID(vec_operators);
        string savepath = EaSFE::Property::getProperty()->getRootPath() + EaSFE::Property::getProperty()->getSavePath();
        auto memorysize = dataset->getMemorySize();
        if (numsfeatures >= 0)
        {
            if (numsfeatures == 0) numsfeatures = dataset->getFeatureSize() / 2;
            LOG(INFO) << "Begin filterFeatureByIGAndGBM" << " numsfeatures: " << numsfeatures << " / all: " << vec_operators.size();
            std::vector<std::pair<PFeatureInfo, PFCOperators>> featureinfoopers;
            if (dataset->getNumID() > 1 || Property::getProperty()->getDistributedNodes() > 1)
            {
                filterFeatureByIG(dataset, vec_operators, featureinfoopers, numsfeatures);
            }
			else
			{
                int gbmboostfeatures = Property::getProperty()->getGBMBoostFeatures();
                if (gbmboostfeatures <= 0) gbmboostfeatures = MAX_GBM_FEATURES;
                gbmboostfeatures = std::min(gbmboostfeatures, MAX_GBM_FEATURES);
                if (vec_operators.size() > gbmboostfeatures)
                    filterFeatureByIG(dataset, vec_operators, featureinfoopers, gbmboostfeatures);
                else
                {
                    for (auto& fops : vec_operators)
                        featureinfoopers.push_back({ nullptr, fops });
                }
                if (featureinfoopers.size() > numsfeatures)
                {
                    GBMFilter gbmfilter(dataset, featureinfoopers);
                    gbmfilter.produceScore(numsfeatures);
                    gbmfilter.clear();
                }
			}
            
            
            LOG(INFO) << "End filterFeatureByIGAndGBM"<< " numsfeatures: " << numsfeatures << " / all: " << vec_operators.size();
            if (dataset->getNumID() > 1 || Property::getProperty()->getDistributedNodes() > 1)
            {
                if (!addFeatureToDataset(dataset, featureinfoopers))
                {
                    LOG(ERROR) << "startnoIterateFeatureConstruction" << " addFeatureToDataset Error!";
                    exit(1);
                }
                if ((memorysize >> 20) > Property::getProperty()->getLimitedMemory())
                {
                    stop.Stop();
                    dataset->write(savepath);
                    delete dataset;
                    dataset = nullptr;
                    stop.Start();
                }
                stop.Stop();
                testdataset = clearProcessesfortestdata(); // load test data

                stop.Start();
                if (testdataset != nullptr)
                {
                    std::vector<PFCOperators> testfcopers;
                    for (auto& featureinfooper : featureinfoopers)
                    {
                        testfcopers.push_back(copyFCOperators(testdataset, featureinfooper.second));
                    }

                    if (!addFeatureToDataset(testdataset, testfcopers))
                    {
                        LOG(ERROR) << "startnoIterateFeatureConstruction" << " addFeatureToDataset Error!";
                        exit(1);
                    }
                    autofcClear(testfcopers);
                }
            }
            else
            {
                stop.Stop();
                testdataset = clearProcessesfortestdata(); // load test data
                stop.Start();
                dataset->megerDataSet(testdataset, 1);
                if(testdataset != nullptr) delete testdataset, testdataset = nullptr;
                for(auto & featureinfooper : featureinfoopers)
				{
					auto& featureinfo = featureinfooper.first;
					if (featureinfo)
					{
						featureinfo->clear();
						delete featureinfo;
						featureinfo = nullptr;
					}
				}
                if(!addFeatureToDataset(dataset, featureinfoopers))
				{
					LOG(ERROR) << "startnoIterateFeatureConstruction" << " addFeatureToDataset Error!";
					exit(1);
				}
            }
        }
        else
        {
            if (!addFeatureToDataset(dataset, vec_operators, false, false))
            {
                LOG(ERROR) << "startnoIterateFeatureConstruction" << " addFeatureToDataset Error!";
                exit(1);
            }
            if ((memorysize >> 20) > Property::getProperty()->getLimitedMemory())
            {
                stop.Stop();
                dataset->write(savepath);
                delete dataset;
                dataset = nullptr;
                stop.Start();
            }
            stop.Stop();
            auto valdataset = getValData();
            if(valdataset != nullptr)
            {
                std::vector<PFCOperators> valfcopers;
                for (auto& fops : vec_operators)
                {
                    valfcopers.push_back(copyFCOperators(valdataset, fops));
                }
                if (!addFeatureToDataset(valdataset, valfcopers))
                {
                    LOG(ERROR) << "startnoIterateFeatureConstruction" << " addFeatureToDataset Error!";
                    exit(1);
                }
                autofcClear(valfcopers);
                valdataset->write(savepath);
                delete valdataset;
            }
            testdataset = clearProcessesfortestdata(); // load test data
            stop.Start();
            if (testdataset != nullptr)
            {
                std::vector<PFCOperators> testfcopers;
                for (auto& fops : vec_operators)
                {
                    testfcopers.push_back(copyFCOperators(testdataset, fops));
                }
                if (!addFeatureToDataset(testdataset, testfcopers))
                {
                    LOG(ERROR) << "startnoIterateFeatureConstruction" << " addFeatureToDataset Error!";
                    exit(1);
                }
                autofcClear(testfcopers);
            }
        }
        autofcClear(vec_operators);
        stop.Stop();
        LOG(INFO) << "FeatureConstruction cost: " << stop.Elapsed() << " us.";
        return dataset;
    }
    
    void filterFeatureByIG(DataSet* dataset, std::vector<PFCOperators>& fcopers, std::vector<std::pair<PFeatureInfo, PFCOperators>>& result, int leftfeatures)
    {
        using myqueuenode = std::pair<MyDataType, std::pair<PFeatureInfo, PFCOperators>>;
        struct cmp
        {
            bool operator()(myqueuenode a, myqueuenode b)
            {
                if (a.first == b.first)return a.second.second->getFCOperID() < b.second.second->getFCOperID();
                return a.first < b.first;
            };
        };
        auto igscoreprefeature = std::priority_queue<myqueuenode, std::vector<myqueuenode>, cmp>();
        if (Property::getProperty()->getDistributedNodes() > 1)
        {
#ifdef USE_MPICH
            for (int i = 0; i < fcopers.size(); ++i)
            {
                if (i % 10 == 0)
                    LOG(TRACE) << "filterFeatureByIG: " << i << " / " << fcopers.size();
                if (i % 500 == 0)
                    LOG(INFO) << "filterFeatureByIG: " << i << " / " << fcopers.size();
                auto featureinfo = generateFeature(dataset, fcopers[i]);
                if (!featureinfo) continue;
                std::vector<MyDataType> allscore = { 0.0 };
                for (int dataid = 0; dataid < dataset->getNumID(); ++dataid)
                {
                    dataset->setID(dataid);
                    dataset->deserialize();
                    if (dataid > 0)
                        featureinfo = generateFeature(dataset, fcopers[i]);
                    std::vector<PFeatureInfo> templist;
                    templist.push_back(featureinfo);
                    Filter igfe;
                    igfe.init(templist);
                    allscore[0] += igfe.produceScore(dataset);
                    if (dataid < dataset->getNumID() - 1)
                    {
                        featureinfo->clear();
                        delete featureinfo;
                        featureinfo = nullptr;
                    }
                }
                if (Property::getProperty()->getDistributedNodes() > 1)
                    getDistributeAvg(allscore, false);
                MyDataType score = allscore[0];
                if (igscoreprefeature.size() < leftfeatures)
                    igscoreprefeature.push({ score, {featureinfo, fcopers[i]} });
                else
                {
                    auto minnode = igscoreprefeature.top();
                    if (minnode.first > score || (minnode.first == score && minnode.second.second->getFCOperID() > fcopers[i]->getFCOperID()))
                    {
                        igscoreprefeature.pop();
                        igscoreprefeature.push({ score, {featureinfo, fcopers[i]} });
                        auto& featureinfo = minnode.second.first;
                        featureinfo->clear();
                        delete featureinfo;
                        featureinfo = nullptr;
                    }
                    else
                    {
                        featureinfo->clear();
                        delete featureinfo;
                        featureinfo = nullptr;
                    }
                }
            }
            while (!igscoreprefeature.empty())
            {
                auto& node = igscoreprefeature.top();
                result.push_back(node.second);
                igscoreprefeature.pop();
            }

#else
            LOG(FATAL) << "filterFeatureByIG Please USE_MPICH = ON when CMake this project, or you cannot set Property::getProperty()->setDistributedNodes() > 1";
#endif
        }
        else
        {
            TempDataSet* m_tempdataset = nullptr;
            if (dataset->getNumID() > 1)
            {
                auto numdatasets = Property::getProperty()->getNumTempDatasets();
                numdatasets = std::min(dataset->getNumID(), numdatasets);

                std::vector<DataSet*> tempdatasets;
                tempdatasets.push_back(dataset);
                for (int i = 0; i < numdatasets - 1; ++i)
                    tempdatasets.push_back(dataset->deepcopy());
                m_tempdataset = new TempDataSet(tempdatasets);
            }

            int threads = Property::getProperty()->getWeThreadNum();
            if (threads > fcopers.size()) threads = fcopers.size();
            std::vector<std::thread> threadsPool;
            int lengthPreThread = fcopers.size() / threads;
            std::atomic<int> count = 0;
            std::mutex mutex_;
            int threadnums = Property::getProperty()->getThreadNum();
            for (int i = 0; i < threads; ++i)
            {
                int start = i * lengthPreThread;
                int end = (i == threads - 1) ? fcopers.size() : (i + 1) * lengthPreThread;
                threadsPool.push_back(std::thread([start, end, &fcopers, &igscoreprefeature, dataset, leftfeatures, &count, &mutex_, threadnums, &m_tempdataset]()
                {
                    for (int j = start; j < end; ++j)
                    {
                        if (count++ % 1500 == 0)
                            LOG(INFO) << "filterFeatureByIG: " << count << " / " << fcopers.size();
                        if (count % 10 == 0)
                            LOG(TRACE) << "filterFeatureByIG: " << count << " / " << fcopers.size();

                        MyDataType score = 0.0;
                        PFeatureInfo featureinfo = nullptr;
                        auto numid = dataset->getNumID();
                        for (int dataid = 0; dataid < numid; ++dataid)
                        {
                            auto odataset = dataset;
                            bool wlock = false;
                            if (numid > 1 && m_tempdataset)
                            {
                                m_tempdataset->getMutex()->lock();
                                odataset = m_tempdataset->getDataSet();
                                m_tempdataset->getNext();
                                m_tempdataset->getMutex()->unlock();
                                odataset->getMutex()->lock();
                            }
                            featureinfo = generateFeature(odataset, fcopers[j]);
                            if (numid > 1 && m_tempdataset) odataset->getMutex()->unlock();
                            std::vector<PFeatureInfo> templist;
                            templist.push_back(featureinfo);
                            Filter igfe;
                            igfe.init(templist);
                            score += igfe.produceScore(odataset);
                            int instances = odataset->getInstancesOfFeature();
                            if(numid > 1) instances = odataset->getInstancesOfFeature(3);
                            if ((numid > 1 && instances >= (2 << 10)) || instances >= (16 << 20) || instances * leftfeatures >= (1 << 25))
                            {
                                featureinfo->clear();
                                delete featureinfo;
                                featureinfo = nullptr;
                            }
                        }
                        mutex_.lock();
                        if (igscoreprefeature.size() < leftfeatures)
                        {
                            igscoreprefeature.push({ score, {featureinfo, fcopers[j]} });
                            mutex_.unlock();
                        }
                        else
                        {
                            mutex_.unlock();
                            auto minnode = igscoreprefeature.top();
                            if (minnode.first >= score)
                            {
                                mutex_.lock();
                                minnode = igscoreprefeature.top();
                                if (minnode.first < score || (minnode.first == score && minnode.second.second->getFCOperID() <= fcopers[j]->getFCOperID()))
                                {
                                    mutex_.unlock();
                                    if (featureinfo)
                                    {
                                        featureinfo->clear();
                                        delete featureinfo;
                                        featureinfo = nullptr;
                                    }
                                    continue;
                                }

                                igscoreprefeature.pop();
                                igscoreprefeature.push({ score, {featureinfo, fcopers[j]} });
                                mutex_.unlock();
                                auto& featureinfo2 = minnode.second.first;
                                if (featureinfo2)
                                {
                                    featureinfo2->clear();
                                    delete featureinfo2;
                                    featureinfo2 = nullptr;
                                }
                            }
                            else
                            {
                                if (featureinfo)
                                {
                                    featureinfo->clear();
                                    delete featureinfo;
                                    featureinfo = nullptr;
                                }
                            }
                        }
                    }
                }));
            }
            for (auto& i : threadsPool)
                i.join();

            if (m_tempdataset)
            {
                auto tempdatasets = m_tempdataset->getDataSets();
                for (int i = 1; i < tempdatasets.size(); ++i)
                    delete tempdatasets[i];
                delete m_tempdataset; m_tempdataset = nullptr;
            }
            while (!igscoreprefeature.empty())
            {
                auto& node = igscoreprefeature.top();
                result.push_back(node.second);
                igscoreprefeature.pop();
            }
        }
        std::reverse(result.begin(), result.end());
    }

    void saveData(DataSet*& dataset)
    {
        string savepath = EaSFE::Property::getProperty()->getRootPath() + EaSFE::Property::getProperty()->getSavePath();
        if (dataset != nullptr) {dataset->write(savepath); delete dataset; dataset = nullptr;}
    }
    
    void InitLog()
    {
        string logpath = EaSFE::Property::getProperty()->getRootPath() + EaSFE::Property::getProperty()->getLoggerPath();
        el::Configurations defaultConf;
        // set path to log file
        defaultConf.set(el::Level::Global,
                    el::ConfigurationType::Filename, logpath);
        el::Level level = static_cast<el::Level>(EaSFE::Property::getProperty()->getLoggerLevel());
        el::Loggers::reconfigureLogger("default", defaultConf);
        el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);
        el::Loggers::setLoggingLevel(level);
        defaultConf.setToDefault();
    }

}//EaSFE



