编译环境：
c++17
cmake-3.22
gcc/g++-10.3.1


项目依赖：
jemalloc
pthread
libnuma
arrow
googletest

jemalloc库编译安装:
https://github.com/jemalloc/jemalloc/releases
cd jemalloc-5.3.0/
yum install autoconf
./autogen.sh
make -j8
make install

libnuma库安装：
dnf install numactl-devel
编译需要加上  -lnuma  标识

Arrow库编译安装:
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir build
cd build
cmake -DARROW_COMPUTE=ON -DARROW_PARQUET=ON -DARROW_WITH_ZLIB=ON -DARROW_WITH_LZ4=ON -DARROW_WITH_SNAPPY=ON -DARROW_CSV=ON -DCMAKE_BUILD_TYPE=Release ..
make -j16
make install

GTest编译安装：
在https://gitee.com/caiwanli/googletest.git中下载对应tag
tar -zxvf googletest-release-1.8.1.tar.gz
cd googletest-release-1.8.1/
mkdir build
cd build
cmake ..
make -j16
make install

levelDB编译安装
git clone --recurse-submodules https://github.com/google/leveldb.git
cd leveldb
mkdir -p build && cd build
cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
sudo make install


项目使用：
cd dasex
mkdir build
cd build
cmake .. 
make -j8
编译产物（main）目前放置在./build
####运行  前面的参数用于打印jemalloc统计信息
MALLOC_CONF=stats_print:true ./DaseX_test


项目运行：
所有测试用例均在../tests目录下，tpch22条查询分别对应tpch_qx.cpp命名的文件，导入数据代码在import_data.hpp文件中。相关数据目录配置
统一放在file_dir_config.hpp文件中。目前已经将0.1G的小批量数据放在../test_data中，4线程要用到的数据在../test_data/4thread中。如果
要测试大规模数据，可以自己用TPCH生成数据并配置绝对路径（注意，要预先将数据中的时间数据处理为时间戳）。
上面提到编译后的产物为DaseX_test可执行文件，进入该文件所在目录，执行对应测试用例即可。例如，执行tpch_q1.cpp中的测试用例可以使用下面的命令：
./DaseX_Test --gtest_filter="TPCHTest.Q1SingleThreadTest"
或者
./DaseX_Test --gtest_filter="TPCHTest.Q1FourThreadTest"
每个测试文件中有两个测试用例，一个单线程运行，一个4线程运行，在调试阶段建议使用单线程测试用例。
代码执行没有结果输出，若需要输出结果，可以在对应算子中使用spdlog::info()打印结果（如果要查看最终结果，应在执行计划的最后一个算子中设置打印信息）。
spdlog打印模板：
spdlog::info("[{} : {}] build执行时间: {}", __FILE__, __LINE__, Util::time_difference(start, end));
每条查询的物理计划可在对应ppt(../tpch_plan目录中)中查看。
有关执行详细说明参看../tpch_plan目录中“项目执行说明.docx”。


项目说明：

1.项目源码在src目录中，
common：用于放置公共类和函数，例如，config和context；
  config：用于放置全局变量或者初始化配置信息；
  context：上下文信息，目前只有一个执行上下文，算子执行的信息都可以放在context中；
function：用于放置算子处理用到的函数，例如，Filter中的函数表达式；
operator：所有算子实现均放在该目录中，子目录对应具体算子实现，PhysicalOperator是所有算子的基类（physical_operator.hpp）；
  agg: 聚合函数/Groupby
  filter：过滤算子/where
  join：连接算子
  project：投影算子
  scan：表扫描算子
pipeline：管道实现，包含所有pipeline相关功能类；
  execute：pipeline执行器
sche：任务调度系统；
storage：底层存储，数据在内存中的抽象，采用Apache Arrow格式
util: 辅助类函数或者宏定义

2.测试代码在tests目录中
main.cpp固定写法，与GTest相关。测试用例实现在test.cpp中，可根据情况编写测试用例。
parser: sql解析,得到pg语法树，与pg格式一样，使用DuckDB parser


FQA：
1.编译完成后，运行时出现找不到xx文件？
使用   ldd ./main   查看缺少的动态库，然后将对应的路径添加的 LD_LIBRARY_PATH
命令： export LD_LIBRARY_PATH=LD_LIBRARY_PATH:/xxx.so ,永久生效需要添加到 ~/.bashrc 中。

2.内存泄漏检查？
valgrind --tool=memcheck --leak-check=full ./DB314_test

3.调试代码如何打印日志？
代码中统一使用spdlog::info来打印日志，格式如下所示：
spdlog::info("[{} : {}] build执行时间: {}", __FILE__, __LINE__, Util::time_difference(start, end));
前半部分固定格式，输出文件、日志所在位置，后半部分为需要打印的内容。

4. 执行指定测试用例
./DaseX_Test --gtest_filter="Test.MyTest1"
./DaseX_Test --gtest_filter="TPCHTest.Q7SingleThreadTest"

5. 查看某个函数的执行时间
auto start = std::chrono::steady_clock::now();
build(lstate);
auto end = std::chrono::steady_clock::now();
spdlog::info("[{} : {}] build执行时间: {}", __FILE__, __LINE__, Util::time_difference(start, end));