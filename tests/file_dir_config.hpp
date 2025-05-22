//
// Created by cwl on 11/11/24.
//

#ifndef DASEX_FILE_DIR_CONFIG_HPP
#define DASEX_FILE_DIR_CONFIG_HPP

#include <string>

/// 配置表文件的目录
//const std::string fileDir1 = "/home/cwl/data/experiment_0.1";
//const std::string fileDir4 = "/home/cwl/data/experiment_0.1/thread4";
//const std::string fileDir8 = "/home/cwl/data/experiment_0.1/thread8";
//const std::string fileDir16 = "/home/cwl/data/experiment_0.1/thread16";
//const std::string fileDir32 = "/home/cwl/data/experiment_0.1/thread32";

const std::string fileDir1 = "../test_data";
//const std::string fileDir4 = "../test_data/4thread";
//const std::string fileDir1 = "/home/cwl/data/experiment_1/thread1";
//const std::string fileDir4 = "/home/cwl/data/experiment_1/thread4";
//const std::string fileDir8 = "/home/cwl/data/experiment_1/thread8";
//const std::string fileDir16 = "/home/cwl/data/experiment_1/thread16";
//const std::string fileDir32 = "/home/cwl/data/experiment_1/thread32";

//const std::string fileDir1 = "/home/cwl/data/experiment_10/thread1";
//const std::string fileDir4 = "/home/cwl/data/experiment_10/thread4";
//const std::string fileDir8 = "/home/cwl/data/experiment_10/thread8";
// const std::string fileDir4 = "/home/cwl/data/experiment_10/thread16";
// const std::string fileDir4 = "/home/cwl/data/experiment_10/thread34";

// 4线程
// const int arr[] = {1, 2, 3, 4};

// 8线程
//const int arr[] = {1, 2, 3, 4, 5, 6, 7, 8};

// 16线程
// const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

// 32线程
// const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

// 34线程
// const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,32,33};

#if THREAD_NUMS == 4
const std::string fileDir4 = "../test_data/4thread";
const int arr[] = {0, 1, 2, 3};
#elif THREAD_NUMS == 8
const std::string fileDir4 = "/home/cwl/data/experiment_10/thread8";
const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7};
#elif THREAD_NUMS == 16
const std::string fileDir4 = "/home/cwl/data/experiment_10/thread16";
const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15};
#elif THREAD_NUMS == 32
// const std::string fileDir4 = "/home/cwl/data/experiment_10/thread32";
const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
#elif THREAD_NUMS == 34
const std::string fileDir4 = "/home/cwl/data/experiment_10/thread34";
const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33};
#elif THREAD_NUMS == 44
const std::string fileDir4 = "/home/cwl/data/experiment_10/thread44";
const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,
                   44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65};
#elif THREAD_NUMS == 64
const std::string fileDir4 = "/home/cwl/data/experiment_10/thread64";
const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
                   32,33,34,35,36,37,38,39,40,41,42,43,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107};
#elif THREAD_NUMS == 88
const std::string fileDir4 = "/home/cwl/data/experiment_10/thread88";
const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
                   32,33,34,35,36,37,38,39,40,41,42,43,44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
                   59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83,
                   84, 85, 86, 87};

//const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
//                   32,33,34,35,36,37,38,39,40,41,42,43,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,
//                   108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131};
#else
    #error "Unsupported THREAD_NUMS value"
#endif

// 44线程
// const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,32,33,34,35,36,37,38,39,40,41,42,43};



// 1G: 149999, 0.1G: 14999
const int customer_value = 1499999;

#endif //DASEX_FILE_DIR_CONFIG_HPP