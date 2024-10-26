<img src="doc\EaSFE.jpg" alt="EaSFE" width="100%">

# EaSFE: Efficient and Scalable Automatic Feature Construction 

EaSFE is an efficient automatic feature construction method for large datasets (larger than a single machine's memory), written in C++17. EaSFE also supports the sparse  [datasets](https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/). In addition, thanks to [csv2](https://github.com/p-ranav/csv2), [easyloggingpp](https://github.com/abumq/easyloggingpp), [json](https://github.com/nlohmann/json) Open source tools.

 [Chinese](README_zh.md) 

## 1. Install Guide:

- ### **Windows：**

VisualStudio >= 2019

cmake >= 3.13

MPI >= 1.0.3 (optional)

run：

```shell
mkdir build
cd build
cmake ..  -DCMAKE_INSTALL_PREFIX="./"
```

If you don't want to run distributed, you don't need to set `-DUSE_MPICH=ON`. If your MPI is not in the system directory, you need to specify `-DMPICH_INCLUDE_DIR and -DMPI_LIBRARY`. Then select the `EaSFE.sln` project file, open it with VS, click INSTALL in the solution, and select Generate -> Generate INSTALL in the menu bar. You can install `EaSFE.dll` and `EaSFEMain.exe` to the folder specified in `CMAKE_INSTALL_PREFIX`.

The meaning of each parameter of cmake：

`USE_MPICH`：if use `MPI` or not

`MPICH_INCLUDE_DIR`：the include directory for `MPI`

`MPI_LIBRARY`：the lib directory for `MPI`

- ### **Linux & MacOS：**

cmake >= 3.13

MPI >= 4.1 (optional)


run:

```shell
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX="./" -DUSE_MPICH=ON
make install -j8
```
Similarly, if you don't want to run distributed, you don't need to set `-DUSE_MPICH=ON`. If your MPI is not in the system directory, you need to specify `-DMPICH_INCLUDE_DIR and -DMPI_LIBRARY`.

## 2. Use Guide:

- Start by configuring the file [property.json](config/property.json) in `config`, You can create different `json` file  for different datasets:

  **Parameter description**：

  1. **`rootpath`**: Root path of the project
  2. **`datasetpath`**: Path of the dataset, relative to **`rootpath`**
  3. **`savepath`**: Path where the dataset is saved, relative to **`rootpath`**
  4. **`classname`**: Name of the class feature; not required for sparse datasets
  5. **`datasetname`**: Name of the dataset
  6. `distributednodes`: Number of distributed nodes
  7. `maxfcoperators`: Maximum number of new features to generate, not the number of retained new features
  8. `datefeaturename`: Name of the time series feature
  9. **`discretefeaturename`**: Name of the discrete feature
  10. `loggerpath`: Path of the log files, relative to **`rootpath`**
  11. `numtempdatasets`: Number of buffer datasets, applicable for large dataset chunking
  12. **`testdatapath`**: Path of the test dataset, relative to **`rootpath`**; if none, leave as ""
  13. `threadnum`: Number of parallel threads during single feature generation process
  14. `wethreadnum`: Number of threads for parallel feature generation or filtering
  15. **`targetclasses`**: Number of class categories; 0 for regression datasets
  16. **`maxnumsfeatures`**: Number of retained new features; -1 for all retained (= `maxfcoperators`), 0 for half of original features (= `min`(**`featurenum`**/2, `maxfcoperators`)), >0 for `min`(**`maxnumsfeatures`**, `maxfcoperators`) features retained
  17. **`datasettype`**: Type of dataset, 0: csv data, 1: distributed data, 2: libsvm classification data, 3: libsvm regression data, 4: chunked dataset
  18. `loggerlevel`: Log level, `1: Global, 2: Trace, 4: Debug, 8: Fatal, 16: Error, 32: Warning, 64: Verbose, 128: Info, 1024: Unknown`
  19. `missval`: Missing values in the dataset
  20. **`featurenum`**: Number of original features in the dataset
  21. `discreteclass`: Number of buckets for Discretizer
  22. `maxselectionnum`: Maximum number of discrete features selected from the original dataset
  23. `mutiloperators`: Method for constructing multiple feature features
  24. `otherdatasethashead`: Whether the non-first block of chunked large datasets has a header row, is only required when chunking on a single machine
  25. `targetclassindex`: Index of the class feature; -1 for the last column, use positive numbers for other indices, counting from 0.
  26. `targetmutil`: Whether the class is multi-class
  27. `temppath`: Temp path, relative to **`rootpath`**
  28. `unaryoperators`: Method for constructing single feature features

  **Please Note**:

  - Parameters marked in bold may need to be modified.
  - EaSFE supports processing the following types of datasets:
    1. Single csv dataset: Set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset, and set **`datasettype`** to 0.
    2. Single-machine csv chunked dataset: Place all chunks of the dataset in the same directory, set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset directory, and set **`datasettype`** to 4. If you have only one test set, set **`rootpath`**+**`datasetpath`** as the absolute path of the test dataset; if you have multiple test machines, place all chunks of the test dataset in the same directory, and set **`rootpath`**+**`datasetpath`** as the absolute path of the test dataset directory.
    3. Distributed csv dataset: Set `distributednodes`, and set **`datasettype`** to 1. Distribute the dataset to nodes in the cluster, and append the identifier ID after the dataset according to the order of the node configuration file, starting from 1. For example, if there are four nodes in total, set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset for each node, and the name of the dataset on the first node's disk needs to be appended with '0' after **`rootpath`**+**`datasetpath`**, and similarly for subsequent nodes, adding '1', '2', '3', etc.
    4. Single libsvm classification dataset: Set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset, and set **`datasettype`** to 2.
    5. Single libsvm regression dataset: Set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset, and set **`datasettype`** to 3.
    6. Single-machine libsvm classification chunked dataset: Place all chunks of the dataset in the same directory, set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset directory, and set **`datasettype`** to 2. If you have only one test set, set **`rootpath`**+**`datasetpath`** as the absolute path of the test dataset; if you have multiple test machines, place all chunks of the test dataset in the same directory, and set **`rootpath`**+**`datasetpath`** as the absolute path of the test dataset directory.
    7. Single-machine libsvm regression chunked dataset: Place all chunks of the dataset in the same directory, set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset directory, and set **`datasettype`** to 3. If you have only one test set, set **`rootpath`**+**`datasetpath`** as the absolute path of the test dataset; if you have multiple test machines, place all chunks of the test dataset in the same directory, and set **`rootpath`**+**`datasetpath`** as the absolute path of the test dataset directory.
    8. Distributed libsvm classification dataset: Set `distributednodes`, set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset, and set **`datasettype`** to 2. Distribute the dataset to nodes in the cluster, and append the identifier ID after the dataset according to the order of the node configuration file, starting from 1. For example, if there are four nodes in total, set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset for each node, and the name of the dataset on the first node's disk needs to be appended with '0' after **`rootpath`**+**`datasetpath`**, and similarly for subsequent nodes, adding '1', '2', '3', etc.
    9. Distributed libsvm regression dataset: Set `distributednodes`, set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset, and set **`datasettype`** to 3. Distribute the dataset to nodes in the cluster, and append the identifier ID after the dataset according to the order of the node configuration file, starting from 1. For example, if there are four nodes in total, set **`rootpath`**+**`datasetpath`** as the absolute path of the dataset for each node, and the name of the dataset on the first node's disk needs to be appended with '0' after **`rootpath`**+**`datasetpath`**, and similarly for subsequent nodes, adding '1', '2', '3', etc.

  

- run ：

  1. distributed

     ```
     Windows:
     mpiexec -np 2 --hostfile config ./EaSFEMain.exe ../config/property.json
     Linux & MacOS:
     mpirun -n 2 --hostfile config ./EaSFEMain ../config/property.json
     ```

     the sample of `config` file：

     ```shell
     192.168.0.1 slots=1
     192.168.0.2 slots=1
     ```

  2. not distributed

     ```shell
     Windows:
     ./EaSFEMain.exe ../config/property.json
     Linux & MacOS:
     ./EaSFEMain ../config/property.json
     ```

