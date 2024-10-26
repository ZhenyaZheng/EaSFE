<img src="doc\EaSFE.jpg" alt="EaSFE" width= "100%">

# EaSFE: 高效且可扩展的自动化特征工程

EaSFE是一个用`C++17`实现，支持大数据集（超过单机内存）的高效自动特征工程方法，得益于[ThunderSVM](https://github.com/Xtra-Computing/thundersvm)的工作，EaSFE也支持稀疏数据集（[libsvm](https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/)）。另外， 感谢[csv2](https://github.com/p-ranav/csv2)， [easyloggingpp](https://github.com/abumq/easyloggingpp)， [json](https://github.com/nlohmann/json)开源工具。

[English](README.md)



## 1. **安装方法：**

- ### **Windows：**

VisualStudio >= 2019

cmake >= 3.13

MPI >= 1.0.3 (可选的)


执行命令：

```shell
mkdir build
cd build
cmake ..  -DCMAKE_INSTALL_PREFIX="./" -DUSE_MPICH=ON
```

如果你不需要分布式运行，可以不用设置`-DUSE_MPICH=ON`, 如果你的MPI不在系统目录，则需要指定 `-DMPICH_INCLUDE_DIR 和 -DMPI_LIBRARY`。然后选择`EaSFE.sln`项目文件，用VS打开，在解决方案里点击INSTALL，然后选择菜单栏的生成->生成INSTALL，即可以安装`EaSFE.dll`以及`EaSFEMain.exe`到`CMAKE_INSTALL_PREFIX`指定的文件夹。

cmake的各参数所表示的意义：

`USE_MPICH`：是否使用`MPI`

`MPICH_INCLUDE_DIR`：`MPI`的include目录

`MPI_LIBRARY`：`MPI`的lib目录

- ### **Linux & MacOS：**

cmake >= 3.13
MPI >= 4.1 (可选的)

执行命令

```shell
mkdir build  
cd build
cmake .. -DCMAKE_INSTALL_PREFIX="./" -DUSE_MPICH=ON
make install -j8
```
同理，如果你不需要分布式运行，可以不用设置`-DUSE_MPICH=ON`, 如果你的MPI不在系统目录，则需要指定 `-DMPICH_INCLUDE_DIR 和 -DMPI_LIBRARY`。

## 2. **使用方法：**

- 首先对config里的文件[property.json](config/property.json)进行配置，你可以为不同的数据集创建不同的json：

  参数说明：

  1. **`rootpath`**：项目的根路径
  2. **`datasetpath`**：数据集的路径，相对于**`rootpath`**
  3. **`savepath`**：数据集保存的路径，相对于**`rootpath`**
  4. **`classname`**：class特征的名称，sparse数据集不需要
  5. **`datasetname`**：数据集的名字
  6. `distributednodes`：分布式节点个数
  7. `maxfcoperators`：生成新特征的最大数量，并不是保留新特征的数量
  8. `datefeaturename`：时间序列特征的名字
  9. **`discretefeaturename`**：离散型特征的名字
  10. `loggerpath`：日志的路径，相对于**`rootpath`**
  11. `numtempdatasets`：缓冲区个数，适用于大数据集分块
  12. **`testdatapath`**：测试集路径，相对于**`rootpath`**，没有的话为“”
  13. `threadnum`：单个特征生成过程中的并行线程个数
  14. `wethreadnum`：并行生成或过滤特征的线程个数
  15. **`targetclasses`**：class的类别数量，回归数据集为0
  16. **`maxnumsfeatures`**：保留的新特征数量，-1代表全部保留（=`maxfcoperators`），0代表保留原始特征的一半(= `min`(**`featurenum`**/2, `maxfcoperators`)), >0 代表保留`min`(**`maxnumsfeatures`**, `maxfcoperators`)个特征
  17. **`datasettype`**：数据集类型，0：csv数据，1：分布式数据，2：libsvm分类数据， 3：libsvm回归数据，4：分块数据集
  18. `loggerlevel`：日志等级，`1：Global， 2：Trace， 4：Debug， 8：Fatal， 16：Error， 32：Warning， 64： Verbose，128：Info，1024：Unknown`
  19. `missval`：数据集中的缺失值
  20. **`featurenum`**：数据集中原始特征的数量
  21. `discreteclass`：Discretizer分桶的数量
  22. `maxselectionnum`：从原始数据集中选择离散型特征的最大数量
  23. `mutiloperators`：多特征特征构造方法
  24. `otherdatasethashead`：分块大数据集的非首块是否含有标题行, 仅单机分块时需要设置
  25. `targetclassindex`：class特征的索引，-1代表最后一列，其他请用正数表示，从0开始计数。
  26. `targetmutil`：class是否为多类别
  27. `temppath`：temp路径，相对于**`rootpath`**
  28. `unaryoperators`：单特征特征构造方法

**请注意**：

- 以上标为粗体的参数都是可能需要改动的。
- EaSFE支持以下数据集的处理：
  1. 单个csv数据集：设置`rootpath`+`datasetpath`为数据集的绝对路径，`datasettype`设置为0。
  2. 单机csv分块数据集：把数据集的所有块放在同一个目录下，设置`rootpath`+`datasetpath`为数据集目录的绝对路径，`datasettype`设置为4。如果你只有一个测试集，设置`rootpath`+`datasetpath`为测试数据集的绝对路径，如果你有多个测试机，把测试数据集的所有块放在同一个目录下，设置`rootpath`+`datasetpath`为测试数据集目录的绝对路径。
  3. 分布式csv数据集：设置`distributednodes`，`datasettype`设置为1。把数据集分发到集群的节点中，根据节点配置文件的顺序在数据集后面添加标识ID，从1开始。例如一共有四个节点，每个节点设置`rootpath`+`datasetpath`为数据集的绝对路径，第一个节点上的数据集在磁盘上的名字需要在`rootpath`+`datasetpath`后加上‘0’，同理后面的节点依次加上‘1’, ‘2’，‘3’......
  4. 单个libsvm分类数据集：设置`rootpath`+`datasetpath`为数据集的绝对路径，`datasettype`设置为2。
  5. 单个libsvm回归数据集：设置`rootpath`+`datasetpath`为数据集的绝对路径，`datasettype`设置为3。
  6. 单机libsvm分类分块数据集：把数据集的所有块放在同一个目录下，设置`rootpath`+`datasetpath`为数据集目录的绝对路径，`datasettype`设置为2。如果你只有一个测试集，设置`rootpath`+`datasetpath`为测试数据集的绝对路径，如果你有多个测试机，把测试数据集的所有块放在同一个目录下，设置`rootpath`+`datasetpath`为测试数据集目录的绝对路径。
  7. 单机libsvm回归分块数据集：把数据集的所有块放在同一个目录下，设置`rootpath`+`datasetpath`为数据集目录的绝对路径，`datasettype`设置为3。如果你只有一个测试集，设置`rootpath`+`datasetpath`为测试数据集的绝对路径，如果你有多个测试机，把测试数据集的所有块放在同一个目录下，设置`rootpath`+`datasetpath`为测试数据集目录的绝对路径。
  8. 分布式libsvm分类数据集：设置`distributednodes`，设置`rootpath`+`datasetpath`为数据集的绝对路径，`datasettype`设置为2。把数据集分发到集群的节点中，根据节点配置文件的顺序在数据集后面添加标识ID，从1开始。例如一共有四个节点，每个节点设置`rootpath`+`datasetpath`为数据集的绝对路径，第一个节点上的数据集在磁盘上的名字需要在`rootpath`+`datasetpath`后加上‘0’，同理后面的节点依次加上‘1’ ,‘2’，‘3’......
  9. 分布式libsvm回归数据集：设置`distributednodes`，设置`rootpath`+`datasetpath`为数据集的绝对路径，`datasettype`设置为3。把数据集分发到集群的节点中，根据节点配置文件的顺序在数据集后面添加标识ID，从1开始。例如一共有四个节点，每个节点设置`rootpath`+`datasetpath`为数据集的绝对路径，第一个节点上的数据集在磁盘上的名字需要在`rootpath`+`datasetpath`后加上‘0’，同理后面的节点依次加上‘1’ ,‘2’，‘3’......

- 执行命令：

  1. 分布式

     ```
     Windows:
     mpiexec -np 2 --hostfile config ./EaSFEMain.exe ../config/property.json
     Linux & MacOS:
     mpirun -n 2 --hostfile config ./EaSFEMain ../config/property.json
     
     ```

     `config`文件例子：

     ```shell
     192.168.0.1 slots=1
     192.168.0.2 slots=1
     ```

  2. 非分布式

     ```shell
     Windows:
     ./EaSFEMain.exe ../config/property.json
     Linux & MacOS:
     ./EaSFEMain ../config/property.json
     ```

