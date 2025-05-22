#!/bin/bash

# 定义测试用例数组
test_cases=(
  "TPCHTest.Q1FourThreadTest"
  "TPCHTest.Q2FourThreadTest"
  "TPCHTest.Q3FourThreadTest"
  "TPCHTest.Q4FourThreadTest"
  "TPCHTest.Q5FourThreadTest"
#  "TPCHTest.Q6FourThreadTest"
  "TPCHTest.Q7FourThreadTest"
#  "TPCHTest.Q8FourThreadTest"
  "TPCHTest.Q9FourThreadTest"
  "TPCHTest.Q10FourThreadTest"
#  "TPCHTest.Q11FourThreadTest"
#  "TPCHTest.Q12FourThreadTest"
#  "TPCHTest.Q13FourThreadTest"
#  "TPCHTest.Q14FourThreadTest"
#  "TPCHTest.Q15FourThreadTest"
#  "TPCHTest.Q16FourThreadTest"
  "TPCHTest.Q17FourThreadTest"
#  "TPCHTest.Q18FourThreadTest"
#  "TPCHTest.Q19FourThreadTest"
#  "TPCHTest.Q20FourThreadTest"
  "TPCHTest.Q21FourThreadTest"
#  "TPCHTest.Q22FourThreadTest"
)

test_sqls=(
  "Q1"
  "Q2"
  "Q3"
  "Q4"
  "Q5"
#  "Q6"
  "Q7"
#  "Q8"
  "Q9"
  "Q10"
#  "Q11"
#  "Q12"
#  "Q13"
#  "Q14"
#  "Q15"
#  "Q16"
  "Q17"
#  "Q18"
#  "Q19"
#  "Q20"
  "Q21"
#  "Q22"
)

DURATION_TIMES=()

# 定义文件路径
file="/home/cwl/workspace/dasex/executeTime.txt"

# 可执行文件路�?
test_binary="/home/cwl/workspace/dasex/build/DaseX_Test"

cpu_node="0"
memory_node="2"

# 遍历测试用例并依次执�?
for test_case in "${test_cases[@]}"; do
  echo "Running test: $test_case"

  # 使用 numactl 绑定 CPU 机器
  numactl --cpunodebind=$cpu_node --membind=$memory_node $test_binary --gtest_filter="$test_case"

  # 检查返回�?
  if [ $? -eq 0 ]; then
    echo "Test $test_case PASSED"
#    DURATION_TIME=$(cat "$file")
    DURATION_TIME=$(cat "$file" | grep -Eo '[0-9]+(\.[0-9]+)?')
    DURATION_TIMES+=("$DURATION_TIME")
  else
    echo "Test $test_case FAILED"
  fi
  echo "---------------------------------------"
done
count=0
for test_sql in "${test_sqls[@]}"; do
  echo "$test_sql 执行时间: ${DURATION_TIMES[$count]} ms."
  ((count++))
  echo "---------------------------------------"
done

sum=0
for time in "${DURATION_TIMES[@]}"; do
    sum=$(echo "$sum + $time" | bc)
done

# 计算平均值
average=$(echo "scale=2; $sum / $count" | bc)

# 打印平均值
echo "$count 条查询平均执行时间： $average ms."




## �����߼���ִ�� DB314_IMDB_STAND_ALONE
imdb_binary="/home/lsc/daseT_imdb/daseT_imdb/build/DB314_IMDB_STAND_ALONE"
param="30"
#
echo "Running DB314_IMDB_STAND_ALONE with parameter $param"
$imdb_binary $param
#
## ��鷵��ֵ
if [ $? -eq 0 ]; then
  echo "DB314_IMDB_STAND_ALONE execution PASSED"
else
  echo "DB314_IMDB_STAND_ALONE execution FAILED"
fi
