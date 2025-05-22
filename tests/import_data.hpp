#pragma once

#include "include.hpp"

extern std::condition_variable cv;

// NOTE: 不要再用这里的函数！！！

void ImportDataFromNation(std::shared_ptr<Table> &table,
                          int partition_idx,
                          std::string &fileName,
                          int thread_nums,
                          bool &importFinish);

void ImportDataFromRegion(std::shared_ptr<Table> &table,
                          int partition_idx,
                          std::string &fileName,
                          int thread_nums,
                          bool &importFinish);

void ImportDataFromPart(std::shared_ptr<Table> &table,
                        int partition_idx,
                        std::string &fileName,
                        int thread_nums,
                        bool &importFinish);

void ImportDataFromSupplier(std::shared_ptr<Table> &table,
                            int partition_idx,
                            std::string &fileName,
                            int thread_nums,
                            bool &importFinish);

void ImportDataFromPartsupp(std::shared_ptr<Table> &table,
                            int partition_idx,
                            std::string &fileName,
                            int thread_nums,
                            bool &importFinish);

void ImportDataFromCustomer(std::shared_ptr<Table> &table,
                            int partition_idx,
                            std::string &fileName,
                            int thread_nums,
                            bool &importFinish);

void ImportDataFromOrders(std::shared_ptr<Table> &table,
                          int partition_idx,
                          std::string &fileName,
                          int thread_nums,
                          bool &importFinish);

void ImportDataFromLineitem(std::shared_ptr<Table> &table,
                            int partition_idx,
                            std::string &fileName,
                            int thread_nums,
                            bool &importFinish);

void ImportDataFromRevenue(std::shared_ptr<Table> &table,
                           int partition_idx,
                           std::string &fileName,
                           int thread_nums,
                           bool &importFinish);

void InsertNation(std::shared_ptr<Table> &table,
                  std::shared_ptr<Scheduler> &scheduler,
                  std::string &fileDir,
                  bool &importFinish);

void InsertRegion(std::shared_ptr<Table> &table,
                  std::shared_ptr<Scheduler> &scheduler,
                  std::string &fileDir,
                  bool &importFinish);

void InsertPart(std::shared_ptr<Table> &table,
                std::shared_ptr<Scheduler> &scheduler,
                std::string &fileDir,
                bool &importFinish);

void InsertSupplier(std::shared_ptr<Table> &table,
                    std::shared_ptr<Scheduler> &scheduler,
                    std::string &fileDir,
                    bool &importFinish);

void InsertPartsupp(std::shared_ptr<Table> &table,
                    std::shared_ptr<Scheduler> &scheduler,
                    std::string &fileDir,
                    bool &importFinish);

void InsertCustomer(std::shared_ptr<Table> &table,
                    std::shared_ptr<Scheduler> &scheduler,
                    std::string &fileDir,
                    bool &importFinish);

void InsertOrders(std::shared_ptr<Table> &table,
                  std::shared_ptr<Scheduler> &scheduler,
                  std::string &fileDir,
                  bool &importFinish);

void InsertLineitem(std::shared_ptr<Table> &table,
                    std::shared_ptr<Scheduler> &scheduler,
                    std::string &fileDir,
                    bool &importFinish);

void InsertRevenue(std::shared_ptr<Table> &table,
                   std::shared_ptr<Scheduler> &scheduler,
                   std::string &fileDir,
                   bool &importFinish);

void InsertNationMul(std::shared_ptr<Table> &table,
                     std::shared_ptr<Scheduler> &scheduler,
                     std::vector<int> &work_ids,
                     std::vector<int> &partition_idxs,
                     std::vector<std::string> &file_names,
                     bool &importFinish);

void InsertRegionMul(std::shared_ptr<Table> &table,
                     std::shared_ptr<Scheduler> &scheduler,
                     std::vector<int> &work_ids,
                     std::vector<int> &partition_idxs,
                     std::vector<std::string> &file_names,
                     bool &importFinish) ;

void InsertPartMul(std::shared_ptr<Table> &table,
                   std::shared_ptr<Scheduler> &scheduler,
                   std::vector<int> &work_ids,
                   std::vector<int> &partition_idxs,
                   std::vector<std::string> &file_names,
                   bool &importFinish);

void InsertSupplierMul(std::shared_ptr<Table> &table,
                       std::shared_ptr<Scheduler> &scheduler,
                       std::vector<int> &work_ids,
                       std::vector<int> &partition_idxs,
                       std::vector<std::string> &file_names,
                       bool &importFinish);

void InsertPartsuppMul(std::shared_ptr<Table> &table,
                       std::shared_ptr<Scheduler> &scheduler,
                       std::vector<int> &work_ids,
                       std::vector<int> &partition_idxs,
                       std::vector<std::string> &file_names,
                       bool &importFinish);

void InsertCustomerMul(std::shared_ptr<Table> &table,
                       std::shared_ptr<Scheduler> &scheduler,
                       std::vector<int> &work_ids,
                       std::vector<int> &partition_idxs,
                       std::vector<std::string> &file_names,
                       bool &importFinish);

void InsertOrdersMul(std::shared_ptr<Table> &table,
                     std::shared_ptr<Scheduler> &scheduler,
                     std::vector<int> &work_ids,
                     std::vector<int> &partition_idxs,
                     std::vector<std::string> &file_names,
                     bool &importFinish);

void InsertLineitemMul(std::shared_ptr<Table> &table,
                       std::shared_ptr<Scheduler> &scheduler,
                       std::vector<int> &work_ids,
                       std::vector<int> &partition_idxs,
                       std::vector<std::string> &file_names,
                       bool &importFinish);

void InsertLineitemMul2(std::shared_ptr<Table> &table,
                       std::shared_ptr<SchedulerRead> &scheduler,
                       std::vector<int> &work_ids,
                       std::vector<int> &partition_idxs,
                       std::vector<std::string> &file_names,
                       bool &importFinish);

void InsertRevenueMul(std::shared_ptr<Table> &table,
                      std::shared_ptr<Scheduler> &scheduler,
                      std::vector<int> &work_ids,
                      std::vector<int> &partition_idxs,
                      std::vector<std::string> &file_names,
                      bool &importFinish);
