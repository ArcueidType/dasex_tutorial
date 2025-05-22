#include "import_data.hpp"
#include <atomic>

std::mutex mtx;
std::condition_variable cv;
std::atomic<int> counter(0);

void ImportDataFromNation(std::shared_ptr<Table> &table, int partition_idx, std::string &fileName, int thread_nums, bool &importFinish) {
    arrow::Status ok;
    // 构建Schema
    std::shared_ptr<arrow::Field> n_nationkey, n_name, n_regionkey, n_comment;
    std::shared_ptr<arrow::Schema> schema;
    n_nationkey = arrow::field("n_nationkey", arrow::int32());
    n_name = arrow::field("n_name", arrow::utf8());
    n_regionkey = arrow::field("n_regionkey", arrow::int32());
    n_comment = arrow::field("n_comment", arrow::utf8());
    schema = arrow::schema({n_nationkey, n_name, n_regionkey, n_comment});
    // 读取文件数据
    std::vector<int> nationkey;
    std::vector<std::string> name;
    std::vector<int> regionkey;
    std::vector<std::string> comment;
    std::ifstream infile(fileName);
    std::string line;
    if (!infile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return;
    }
    int count = 0;
    int round = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(iss, part, '|')) {
            parts.emplace_back(part);
        }
        nationkey.push_back(std::stoi(parts[0]));
        name.push_back(parts[1]);
        regionkey.push_back(std::stoi(parts[2]));
        comment.push_back(parts[3]);
        count++;
        if(count == CHUNK_SZIE) {
            round++;
            std::shared_ptr<arrow::Array> nationkey_array;
            std::shared_ptr<arrow::Array> name_array;
            std::shared_ptr<arrow::Array> regionkey_array;
            std::shared_ptr<arrow::Array> comment_array;
            ok = DaseX::Util::AppendIntArray(nationkey_array, nationkey);
            ok = DaseX::Util::AppendStringArray(name_array, name);
            ok = DaseX::Util::AppendIntArray(regionkey_array, regionkey);
            ok = DaseX::Util::AppendStringArray(comment_array, comment);
            std::shared_ptr<arrow::RecordBatch> rbatch;
            rbatch = arrow::RecordBatch::Make(schema, CHUNK_SZIE, {nationkey_array, name_array, regionkey_array, comment_array});
            table->insert_data(rbatch, partition_idx);
            count = 0;
            // 清空向量
            nationkey.clear();
            name.clear();
            regionkey.clear();
            comment.clear();
        }
    }
    if(count != 0) {
        std::shared_ptr<arrow::Array> nationkey_array;
        std::shared_ptr<arrow::Array> name_array;
        std::shared_ptr<arrow::Array> regionkey_array;
        std::shared_ptr<arrow::Array> comment_array;
        ok = DaseX::Util::AppendIntArray(nationkey_array, nationkey);
        ok = DaseX::Util::AppendStringArray(name_array, name);
        ok = DaseX::Util::AppendIntArray(regionkey_array, regionkey);
        ok = DaseX::Util::AppendStringArray(comment_array, comment);
        std::shared_ptr<arrow::RecordBatch> rbatch;
        rbatch = arrow::RecordBatch::Make(schema, count, {nationkey_array, name_array, regionkey_array, comment_array});
        table->insert_data(rbatch, partition_idx);
    }
    // spdlog::info("[{} : {}] The round is: {}, the final count is: {} .", __FILE__, __LINE__, round, count);
    infile.close();
    counter.fetch_add(1, std::memory_order_relaxed);
    if(counter == thread_nums) {
        importFinish = true;
        counter.store(0, std::memory_order_relaxed);
        cv.notify_all();
    }
}

void ImportDataFromRegion(std::shared_ptr<Table> &table, int partition_idx, std::string &fileName, int thread_nums, bool &importFinish) {
    arrow::Status ok;
    // 构建Schema
    std::shared_ptr<arrow::Field> r_regionkey, r_name, r_comment;
    std::shared_ptr<arrow::Schema> schema;
    r_regionkey = arrow::field("r_regionkey", arrow::int32());
    r_name = arrow::field("r_name", arrow::utf8());
    r_comment = arrow::field("r_comment", arrow::utf8());
    schema = arrow::schema({r_regionkey, r_name, r_comment});
    // 读取文件数据
    std::vector<int> regionkey;
    std::vector<std::string> name;
    std::vector<std::string> comment;
    std::ifstream infile(fileName);
    std::string line;
    if (!infile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return;
    }
    int count = 0;
    int round = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(iss, part, '|')) {
            parts.emplace_back(part);
        }
        regionkey.push_back(std::stoi(parts[0]));
        name.push_back(parts[1]);
        comment.push_back(parts[2]);
        count++;
        if(count == CHUNK_SZIE) {
            round++;
            std::shared_ptr<arrow::Array> regionkey_array;
            std::shared_ptr<arrow::Array> name_array;
            std::shared_ptr<arrow::Array> comment_array;
            ok = DaseX::Util::AppendIntArray(regionkey_array, regionkey);
            ok = DaseX::Util::AppendStringArray(name_array, name);
            ok = DaseX::Util::AppendStringArray(comment_array, comment);
            std::shared_ptr<arrow::RecordBatch> rbatch;
            rbatch = arrow::RecordBatch::Make(schema, CHUNK_SZIE, {regionkey_array, name_array, comment_array});
            table->insert_data(rbatch, partition_idx);
            count = 0;
            // 清空向量
            regionkey.clear();
            name.clear();
            comment.clear();
        }
    }
    if(count != 0) {
        std::shared_ptr<arrow::Array> regionkey_array;
        std::shared_ptr<arrow::Array> name_array;
        std::shared_ptr<arrow::Array> comment_array;
        ok = DaseX::Util::AppendIntArray(regionkey_array, regionkey);
        ok = DaseX::Util::AppendStringArray(name_array, name);
        ok = DaseX::Util::AppendStringArray(comment_array, comment);
        std::shared_ptr<arrow::RecordBatch> rbatch;
        rbatch = arrow::RecordBatch::Make(schema, count, {regionkey_array, name_array, comment_array});
        table->insert_data(rbatch, partition_idx);
    }
    // spdlog::info("[{} : {}] The round is: {}, the final count is: {} .", __FILE__, __LINE__, round, count);
    infile.close();
    counter.fetch_add(1, std::memory_order_relaxed);
    if(counter == thread_nums) {
        importFinish = true;
        counter.store(0, std::memory_order_relaxed);
        cv.notify_all();
    }
}

void ImportDataFromPart(std::shared_ptr<Table> &table, int partition_idx, std::string &fileName, int thread_nums, bool &importFinish) {
    arrow::Status ok;
    // 构建Schema
    std::shared_ptr<arrow::Field> p_partkey, p_name, p_mfgr, p_brand, p_type, p_size, p_container, p_retailprice, p_comment;
    std::shared_ptr<arrow::Schema> schema;
    p_partkey = arrow::field("p_partkey", arrow::int32());
    p_name = arrow::field("p_name", arrow::utf8());
    p_mfgr = arrow::field("p_mfgr", arrow::utf8());
    p_brand = arrow::field("p_brand", arrow::utf8());
    p_type = arrow::field("p_type", arrow::utf8());
    p_size = arrow::field("p_size", arrow::int32());
    p_container = arrow::field("p_container", arrow::utf8());
    p_retailprice = arrow::field("p_retailprice", arrow::float32());
    p_comment = arrow::field("p_comment", arrow::utf8());
    schema = arrow::schema({p_partkey, p_name, p_mfgr, p_brand, p_type, p_size, p_container, p_retailprice, p_comment});
    // 读取文件数据
    std::vector<int> partkey;
    std::vector<std::string> name;
    std::vector<std::string> mfgr;
    std::vector<std::string> brand;
    std::vector<std::string> type;
    std::vector<int> size;
    std::vector<std::string> container;
    std::vector<float> retailprice;
    std::vector<std::string> comment;
    std::ifstream infile(fileName);
    std::string line;
    if (!infile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return;
    }
    int count = 0;
    int round = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(iss, part, '|')) {
            parts.emplace_back(part);
        }
        partkey.push_back(std::stoi(parts[0]));
        name.push_back(parts[1]);
        mfgr.push_back(parts[2]);
        brand.push_back(parts[3]);
        type.push_back(parts[4]);
        size.push_back(std::stoi(parts[5]));
        container.push_back(parts[6]);
        retailprice.push_back(std::stof(parts[7]));
        comment.push_back(parts[8]);
        count++;
        if(count == CHUNK_SZIE) {
            round++;
            std::shared_ptr<arrow::Array> partkey_array;
            std::shared_ptr<arrow::Array> name_array;
            std::shared_ptr<arrow::Array> mfgr_array;
            std::shared_ptr<arrow::Array> brand_array;
            std::shared_ptr<arrow::Array> type_array;
            std::shared_ptr<arrow::Array> size_array;
            std::shared_ptr<arrow::Array> container_array;
            std::shared_ptr<arrow::Array> retailprice_array;
            std::shared_ptr<arrow::Array> comment_array;
            ok = DaseX::Util::AppendIntArray(partkey_array, partkey);
            ok = DaseX::Util::AppendStringArray(name_array, name);
            ok = DaseX::Util::AppendStringArray(mfgr_array, mfgr);
            ok = DaseX::Util::AppendStringArray(brand_array, brand);
            ok = DaseX::Util::AppendStringArray(type_array, type);
            ok = DaseX::Util::AppendIntArray(size_array, size);
            ok = DaseX::Util::AppendStringArray(container_array, container);
            ok = DaseX::Util::AppendFloatArray(retailprice_array, retailprice);
            ok = DaseX::Util::AppendStringArray(comment_array, comment);
            std::shared_ptr<arrow::RecordBatch> rbatch;
            rbatch = arrow::RecordBatch::Make(schema, CHUNK_SZIE, {partkey_array, name_array, mfgr_array, brand_array, type_array, size_array, container_array, retailprice_array, comment_array});
            table->insert_data(rbatch, partition_idx);
            count = 0;
            // 清空向量
            partkey.clear();
            name.clear();
            mfgr.clear();
            brand.clear();
            type.clear();
            size.clear();
            container.clear();
            retailprice.clear();
            comment.clear();
        }
    }
    if(count != 0) {
        std::shared_ptr<arrow::Array> partkey_array;
        std::shared_ptr<arrow::Array> name_array;
        std::shared_ptr<arrow::Array> mfgr_array;
        std::shared_ptr<arrow::Array> brand_array;
        std::shared_ptr<arrow::Array> type_array;
        std::shared_ptr<arrow::Array> size_array;
        std::shared_ptr<arrow::Array> container_array;
        std::shared_ptr<arrow::Array> retailprice_array;
        std::shared_ptr<arrow::Array> comment_array;
        ok = DaseX::Util::AppendIntArray(partkey_array, partkey);
        ok = DaseX::Util::AppendStringArray(name_array, name);
        ok = DaseX::Util::AppendStringArray(mfgr_array, mfgr);
        ok = DaseX::Util::AppendStringArray(brand_array, brand);
        ok = DaseX::Util::AppendStringArray(type_array, type);
        ok = DaseX::Util::AppendIntArray(size_array, size);
        ok = DaseX::Util::AppendStringArray(container_array, container);
        ok = DaseX::Util::AppendFloatArray(retailprice_array, retailprice);
        ok = DaseX::Util::AppendStringArray(comment_array, comment);
        std::shared_ptr<arrow::RecordBatch> rbatch;
        rbatch = arrow::RecordBatch::Make(schema, count, {partkey_array, name_array, mfgr_array, brand_array, type_array, size_array, container_array, retailprice_array, comment_array});
        table->insert_data(rbatch, partition_idx);
    }
    // spdlog::info("[{} : {}] The round is: {}, the final count is: {} .", __FILE__, __LINE__, round, count);
    infile.close();
    counter.fetch_add(1, std::memory_order_relaxed);
    if(counter == thread_nums) {
        importFinish = true;
        counter.store(0, std::memory_order_relaxed);
        cv.notify_all();
    }
}

void ImportDataFromSupplier(std::shared_ptr<Table> &table, int partition_idx, std::string &fileName, int thread_nums, bool &importFinish) {
    arrow::Status ok;
    // 构建Schema
    std::shared_ptr<arrow::Field> s_suppkey, s_name, s_address, s_nationkey, s_phone, s_acctbal, s_comment;
    std::shared_ptr<arrow::Schema> schema;
    s_suppkey = arrow::field("s_suppkey", arrow::int32());
    s_name = arrow::field("s_name", arrow::utf8());
    s_address = arrow::field("s_address", arrow::utf8());
    s_nationkey = arrow::field("s_nationkey", arrow::int32());
    s_phone = arrow::field("s_phone", arrow::utf8());
    s_acctbal = arrow::field("s_acctbal", arrow::float32());
    s_comment = arrow::field("s_comment", arrow::utf8());
    schema = arrow::schema({s_suppkey, s_name, s_address, s_nationkey, s_phone, s_acctbal, s_comment});
    // 读取文件数据
    std::vector<int> suppkey;
    std::vector<std::string> name;
    std::vector<std::string> address;
    std::vector<int> nationkey;
    std::vector<std::string> phone;
    std::vector<float> acctbal;
    std::vector<std::string> comment;
    std::ifstream infile(fileName);
    std::string line;
    if (!infile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return;
    }
    int count = 0;
    int round = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(iss, part, '|')) {
            parts.emplace_back(part);
        }
        suppkey.push_back(std::stoi(parts[0]));
        name.push_back(parts[1]);
        address.push_back(parts[2]);
        nationkey.push_back(std::stoi(parts[3]));
        phone.push_back(parts[4]);
        acctbal.push_back(std::stof(parts[5]));
        comment.push_back(parts[6]);
        count++;
        if(count == CHUNK_SZIE) {
            round++;
            std::shared_ptr<arrow::Array> suppkey_array;
            std::shared_ptr<arrow::Array> name_array;
            std::shared_ptr<arrow::Array> address_array;
            std::shared_ptr<arrow::Array> nationkey_array;
            std::shared_ptr<arrow::Array> phone_array;
            std::shared_ptr<arrow::Array> acctbal_array;
            std::shared_ptr<arrow::Array> comment_array;
            ok = DaseX::Util::AppendIntArray(suppkey_array, suppkey);
            ok = DaseX::Util::AppendStringArray(name_array, name);
            ok = DaseX::Util::AppendStringArray(address_array, address);
            ok = DaseX::Util::AppendIntArray(nationkey_array, nationkey);
            ok = DaseX::Util::AppendStringArray(phone_array, phone);
            ok = DaseX::Util::AppendFloatArray(acctbal_array, acctbal);
            ok = DaseX::Util::AppendStringArray(comment_array, comment);
            std::shared_ptr<arrow::RecordBatch> rbatch;
            rbatch = arrow::RecordBatch::Make(schema, CHUNK_SZIE, {suppkey_array, name_array, address_array, nationkey_array, phone_array, acctbal_array, comment_array});
            table->insert_data(rbatch, partition_idx);
            count = 0;
            // 清空向量
            suppkey.clear();
            name.clear();
            address.clear();
            nationkey.clear();
            phone.clear();
            acctbal.clear();
            comment.clear();
        }
    }
    if(count != 0) {
        std::shared_ptr<arrow::Array> suppkey_array;
        std::shared_ptr<arrow::Array> name_array;
        std::shared_ptr<arrow::Array> address_array;
        std::shared_ptr<arrow::Array> nationkey_array;
        std::shared_ptr<arrow::Array> phone_array;
        std::shared_ptr<arrow::Array> acctbal_array;
        std::shared_ptr<arrow::Array> comment_array;
        ok = DaseX::Util::AppendIntArray(suppkey_array, suppkey);
        ok = DaseX::Util::AppendStringArray(name_array, name);
        ok = DaseX::Util::AppendStringArray(address_array, address);
        ok = DaseX::Util::AppendIntArray(nationkey_array, nationkey);
        ok = DaseX::Util::AppendStringArray(phone_array, phone);
        ok = DaseX::Util::AppendFloatArray(acctbal_array, acctbal);
        ok = DaseX::Util::AppendStringArray(comment_array, comment);
        std::shared_ptr<arrow::RecordBatch> rbatch;
        rbatch = arrow::RecordBatch::Make(schema, count, {suppkey_array, name_array, address_array, nationkey_array, phone_array, acctbal_array, comment_array});
        table->insert_data(rbatch, partition_idx);
    }
    // spdlog::info("[{} : {}] The round is: {}, the final count is: {} .", __FILE__, __LINE__, round, count);
    infile.close();
    counter.fetch_add(1, std::memory_order_relaxed);
    if(counter == thread_nums) {
        importFinish = true;
        counter.store(0, std::memory_order_relaxed);
        cv.notify_all();
    }
}

void ImportDataFromPartsupp(std::shared_ptr<Table> &table, int partition_idx, std::string &fileName, int thread_nums, bool &importFinish) {
    arrow::Status ok;
    // 构建Schema
    std::shared_ptr<arrow::Field> ps_partkey, ps_suppkey, ps_availqty, ps_supplycost, ps_comment;
    std::shared_ptr<arrow::Schema> schema;
    ps_partkey = arrow::field("ps_partkey", arrow::int32());
    ps_suppkey = arrow::field("ps_suppkey", arrow::int32());
    ps_availqty = arrow::field("ps_availqty", arrow::int32());
    ps_supplycost = arrow::field("ps_supplycost", arrow::float32());
    ps_comment = arrow::field("ps_comment", arrow::utf8());
    schema = arrow::schema({ps_partkey, ps_suppkey, ps_availqty, ps_supplycost, ps_comment});
    // 读取文件数据
    std::vector<int> partkey;
    std::vector<int> suppkey;
    std::vector<int> availqty;
    std::vector<float> supplycost;
    std::vector<std::string> comment;
    std::ifstream infile(fileName);
    std::string line;
    if (!infile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return;
    }
    int count = 0;
    int round = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(iss, part, '|')) {
            parts.emplace_back(part);
        }
        partkey.push_back(std::stoi(parts[0]));
        suppkey.push_back(std::stoi(parts[1]));
        availqty.push_back(std::stoi(parts[2]));
        supplycost.push_back(std::stof(parts[3]));
        comment.push_back(parts[4]);
        count++;
        if(count == CHUNK_SZIE) {
            round++;
            std::shared_ptr<arrow::Array> partkey_array;
            std::shared_ptr<arrow::Array> suppkey_array;
            std::shared_ptr<arrow::Array> availqty_array;
            std::shared_ptr<arrow::Array> supplycost_array;
            std::shared_ptr<arrow::Array> comment_array;
            ok = DaseX::Util::AppendIntArray(partkey_array, partkey);
            ok = DaseX::Util::AppendIntArray(suppkey_array, suppkey);
            ok = DaseX::Util::AppendIntArray(availqty_array, availqty);
            ok = DaseX::Util::AppendFloatArray(supplycost_array, supplycost);
            ok = DaseX::Util::AppendStringArray(comment_array, comment);
            std::shared_ptr<arrow::RecordBatch> rbatch;
            rbatch = arrow::RecordBatch::Make(schema, CHUNK_SZIE, {partkey_array, suppkey_array, availqty_array, supplycost_array, comment_array});
            table->insert_data(rbatch, partition_idx);
            count = 0;
            // 清空向量
            partkey.clear();
            suppkey.clear();
            availqty.clear();
            supplycost.clear();
            comment.clear();
        }
    }
    if(count != 0) {
        std::shared_ptr<arrow::Array> partkey_array;
        std::shared_ptr<arrow::Array> suppkey_array;
        std::shared_ptr<arrow::Array> availqty_array;
        std::shared_ptr<arrow::Array> supplycost_array;
        std::shared_ptr<arrow::Array> comment_array;
        ok = DaseX::Util::AppendIntArray(partkey_array, partkey);
        ok = DaseX::Util::AppendIntArray(suppkey_array, suppkey);
        ok = DaseX::Util::AppendIntArray(availqty_array, availqty);
        ok = DaseX::Util::AppendFloatArray(supplycost_array, supplycost);
        ok = DaseX::Util::AppendStringArray(comment_array, comment);
        std::shared_ptr<arrow::RecordBatch> rbatch;
        rbatch = arrow::RecordBatch::Make(schema, count, {partkey_array, suppkey_array, availqty_array, supplycost_array, comment_array});
        table->insert_data(rbatch, partition_idx);
    }
    // spdlog::info("[{} : {}] The round is: {}, the final count is: {} .", __FILE__, __LINE__, round, count);
    infile.close();
    counter.fetch_add(1, std::memory_order_relaxed);
    if(counter == thread_nums) {
        importFinish = true;
        counter.store(0, std::memory_order_relaxed);
        cv.notify_all();
    }
}

void ImportDataFromCustomer(std::shared_ptr<Table> &table, int partition_idx, std::string &fileName, int thread_nums, bool &importFinish) {
    arrow::Status ok;
    // 构建Schema
    std::shared_ptr<arrow::Field> c_custkey, c_name, c_address, c_nationkey, c_phone, c_acctbal, c_mktsegment, c_comment;
    std::shared_ptr<arrow::Schema> schema;
    c_custkey = arrow::field("c_custkey", arrow::int32());
    c_name = arrow::field("c_name", arrow::utf8());
    c_address = arrow::field("c_address", arrow::utf8());
    c_nationkey = arrow::field("c_nationkey", arrow::int32());
    c_phone = arrow::field("c_phone", arrow::utf8());
    c_acctbal = arrow::field("c_acctbal", arrow::float32());
    c_mktsegment = arrow::field("c_mktsegment", arrow::utf8());
    c_comment = arrow::field("c_comment", arrow::utf8());
    schema = arrow::schema({c_custkey, c_name, c_address, c_nationkey, c_phone, c_acctbal, c_mktsegment, c_comment});
    // 读取文件数据
    std::vector<int> custkey;
    std::vector<std::string> name;
    std::vector<std::string> address;
    std::vector<int> nationkey;
    std::vector<std::string> phone;
    std::vector<float> acctbal;
    std::vector<std::string> mktsegment;
    std::vector<std::string> comment;
    std::ifstream infile(fileName);
    std::string line;
    if (!infile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return;
    }
    int count = 0;
    int round = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(iss, part, '|')) {
            parts.emplace_back(part);
        }
        custkey.push_back(std::stoi(parts[0]));
        name.push_back(parts[1]);
        address.push_back(parts[2]);
        nationkey.push_back(std::stoi(parts[3]));
        phone.push_back(parts[4]);
        acctbal.push_back(std::stof(parts[5]));
        mktsegment.push_back(parts[6]);
        comment.emplace_back(parts[7]);
        count++;
        if(count == CHUNK_SZIE) {
            round++;
            std::shared_ptr<arrow::Array> custkey_array;
            std::shared_ptr<arrow::Array> name_array;
            std::shared_ptr<arrow::Array> address_array;
            std::shared_ptr<arrow::Array> nationkey_array;
            std::shared_ptr<arrow::Array> phone_array;
            std::shared_ptr<arrow::Array> acctbal_array;
            std::shared_ptr<arrow::Array> mktsegment_array;
            std::shared_ptr<arrow::Array> comment_array;
            ok = DaseX::Util::AppendIntArray(custkey_array, custkey);
            ok = DaseX::Util::AppendStringArray(name_array, name);
            ok = DaseX::Util::AppendStringArray(address_array, address);
            ok = DaseX::Util::AppendIntArray(nationkey_array, nationkey);
            ok = DaseX::Util::AppendStringArray(phone_array, phone);
            ok = DaseX::Util::AppendFloatArray(acctbal_array, acctbal);
            ok = DaseX::Util::AppendStringArray(mktsegment_array, mktsegment);
            ok = DaseX::Util::AppendStringArray(comment_array, comment);
            std::shared_ptr<arrow::RecordBatch> rbatch;
            rbatch = arrow::RecordBatch::Make(schema, CHUNK_SZIE, {custkey_array, name_array, address_array, nationkey_array, phone_array, acctbal_array, mktsegment_array, comment_array});
            table->insert_data(rbatch, partition_idx);
            count = 0;
            // 清空向量
            custkey.clear();
            name.clear();
            address.clear();
            nationkey.clear();
            phone.clear();
            acctbal.clear();
            mktsegment.clear();
            comment.clear();
        }
    }
    if(count != 0) {
        std::shared_ptr<arrow::Array> custkey_array;
        std::shared_ptr<arrow::Array> name_array;
        std::shared_ptr<arrow::Array> address_array;
        std::shared_ptr<arrow::Array> nationkey_array;
        std::shared_ptr<arrow::Array> phone_array;
        std::shared_ptr<arrow::Array> acctbal_array;
        std::shared_ptr<arrow::Array> mktsegment_array;
        std::shared_ptr<arrow::Array> comment_array;
        ok = DaseX::Util::AppendIntArray(custkey_array, custkey);
        ok = DaseX::Util::AppendStringArray(name_array, name);
        ok = DaseX::Util::AppendStringArray(address_array, address);
        ok = DaseX::Util::AppendIntArray(nationkey_array, nationkey);
        ok = DaseX::Util::AppendStringArray(phone_array, phone);
        ok = DaseX::Util::AppendFloatArray(acctbal_array, acctbal);
        ok = DaseX::Util::AppendStringArray(mktsegment_array, mktsegment);
        ok = DaseX::Util::AppendStringArray(comment_array, comment);
        std::shared_ptr<arrow::RecordBatch> rbatch;
        rbatch = arrow::RecordBatch::Make(schema, count, {custkey_array, name_array, address_array, nationkey_array, phone_array, acctbal_array, mktsegment_array, comment_array});
        table->insert_data(rbatch, partition_idx);
    }
    // spdlog::info("[{} : {}] The round is: {}, the final count is: {} .", __FILE__, __LINE__, round, count);
    infile.close();
    counter.fetch_add(1, std::memory_order_relaxed);
    if(counter == thread_nums) {
        importFinish = true;
        counter.store(0, std::memory_order_relaxed);
        cv.notify_all();
    }
}

void ImportDataFromOrders(std::shared_ptr<Table> &table, int partition_idx, std::string &fileName, int thread_nums, bool &importFinish) {
    arrow::Status ok;
    // 构建Schema
    std::shared_ptr<arrow::Field> o_orderkey, o_custkey, o_orderstatus, o_totalprice, o_orderdate, o_orderpriority, o_clerk, o_shippriority, o_comment;
    std::shared_ptr<arrow::Schema> schema;
    o_orderkey = arrow::field("o_orderkey", arrow::int32());
    o_custkey = arrow::field("o_custkey", arrow::int32());
    o_orderstatus = arrow::field("o_orderstatus", arrow::utf8());
    o_totalprice = arrow::field("o_totalprice", arrow::float32());
    o_orderdate = arrow::field("o_orderdate", arrow::int32());
    o_orderpriority = arrow::field("o_orderpriority", arrow::utf8());
    o_clerk = arrow::field("o_clerk", arrow::utf8());
    o_shippriority = arrow::field("o_shippriority", arrow::int32());
    o_comment = arrow::field("o_comment", arrow::utf8());
    schema = arrow::schema({o_orderkey, o_custkey, o_orderstatus, o_totalprice, o_orderdate, o_orderpriority, o_clerk, o_shippriority, o_comment});
    // 读取文件数据
    std::vector<int> orderkey;
    std::vector<int> custkey;
    std::vector<std::string> orderstatus;
    std::vector<float> totalprice;
    std::vector<int> orderdate;
    std::vector<std::string> orderpriority;
    std::vector<std::string> clerk;
    std::vector<int> shippriority;
    std::vector<std::string> comment;
    std::ifstream infile(fileName);
    std::string line;
    if (!infile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return;
    }
    int count = 0;
    int round = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(iss, part, '|')) {
            parts.emplace_back(part);
        }
        orderkey.push_back(std::stoi(parts[0]));
        custkey.push_back(std::stoi(parts[1]));
        orderstatus.push_back(parts[2]);
        totalprice.push_back(std::stof(parts[3]));
        orderdate.push_back(std::stoi(parts[4]));
        orderpriority.push_back(parts[5]);
        clerk.push_back(parts[6]);
        shippriority.push_back(std::stoi(parts[7]));
        comment.emplace_back(parts[8]);
        count++;
        if(count == CHUNK_SZIE) {
            round++;
            std::shared_ptr<arrow::Array> orderkey_array;
            std::shared_ptr<arrow::Array> custkey_array;
            std::shared_ptr<arrow::Array> orderstatus_array;
            std::shared_ptr<arrow::Array> totalprice_array;
            std::shared_ptr<arrow::Array> orderdate_array;
            std::shared_ptr<arrow::Array> orderpriority_array;
            std::shared_ptr<arrow::Array> clerk_array;
            std::shared_ptr<arrow::Array> shippriority_array;
            std::shared_ptr<arrow::Array> comment_array;
            ok = DaseX::Util::AppendIntArray(orderkey_array, orderkey);
            ok = DaseX::Util::AppendIntArray(custkey_array, custkey);
            ok = DaseX::Util::AppendStringArray(orderstatus_array, orderstatus);
            ok = DaseX::Util::AppendFloatArray(totalprice_array, totalprice);
            ok = DaseX::Util::AppendIntArray(orderdate_array, orderdate);
            ok = DaseX::Util::AppendStringArray(orderpriority_array, orderpriority);
            ok = DaseX::Util::AppendStringArray(clerk_array, clerk);
            ok = DaseX::Util::AppendIntArray(shippriority_array, shippriority);
            ok = DaseX::Util::AppendStringArray(comment_array, comment);
            std::shared_ptr<arrow::RecordBatch> rbatch;
            rbatch = arrow::RecordBatch::Make(schema, CHUNK_SZIE, {orderkey_array, custkey_array, orderstatus_array, totalprice_array, orderdate_array, orderpriority_array, clerk_array, shippriority_array, comment_array});
            table->insert_data(rbatch, partition_idx);
            count = 0;
            // 清空向量
            orderkey.clear();
            custkey.clear();
            orderstatus.clear();
            totalprice.clear();
            orderdate.clear();
            orderpriority.clear();
            clerk.clear();
            shippriority.clear();
            comment.clear();
        }
    }
    if(count != 0) {
        std::shared_ptr<arrow::Array> orderkey_array;
        std::shared_ptr<arrow::Array> custkey_array;
        std::shared_ptr<arrow::Array> orderstatus_array;
        std::shared_ptr<arrow::Array> totalprice_array;
        std::shared_ptr<arrow::Array> orderdate_array;
        std::shared_ptr<arrow::Array> orderpriority_array;
        std::shared_ptr<arrow::Array> clerk_array;
        std::shared_ptr<arrow::Array> shippriority_array;
        std::shared_ptr<arrow::Array> comment_array;
        ok = DaseX::Util::AppendIntArray(orderkey_array, orderkey);
        ok = DaseX::Util::AppendIntArray(custkey_array, custkey);
        ok = DaseX::Util::AppendStringArray(orderstatus_array, orderstatus);
        ok = DaseX::Util::AppendFloatArray(totalprice_array, totalprice);
        ok = DaseX::Util::AppendIntArray(orderdate_array, orderdate);
        ok = DaseX::Util::AppendStringArray(orderpriority_array, orderpriority);
        ok = DaseX::Util::AppendStringArray(clerk_array, clerk);
        ok = DaseX::Util::AppendIntArray(shippriority_array, shippriority);
        ok = DaseX::Util::AppendStringArray(comment_array, comment);
        std::shared_ptr<arrow::RecordBatch> rbatch;
        rbatch = arrow::RecordBatch::Make(schema, count, {orderkey_array, custkey_array, orderstatus_array, totalprice_array, orderdate_array, orderpriority_array, clerk_array, shippriority_array, comment_array});
        table->insert_data(rbatch, partition_idx);
    }
    // spdlog::info("[{} : {}] The round is: {}, the final count is: {} .", __FILE__, __LINE__, round, count);
    infile.close();
    counter.fetch_add(1, std::memory_order_relaxed);
    if(counter == thread_nums) {
        importFinish = true;
        counter.store(0, std::memory_order_relaxed);
        cv.notify_all();
    }
}

void ImportDataFromLineitem(std::shared_ptr<Table> &table, int partition_idx, std::string &fileName, int thread_nums, bool &importFinish) {
    arrow::Status ok;
    // 构建Schema
    std::shared_ptr<arrow::Field> l_orderkey, l_partkey, l_suppkey, l_linenumber, l_quantity, l_extendedprice, l_discount, l_tax, l_returnflag, l_linestatus, l_shipdate, l_commitdate, l_receiptdate, l_shipinstruct, l_shipmode, l_comment;
    std::shared_ptr<arrow::Schema> schema;
    l_orderkey = arrow::field("l_orderkey", arrow::int32());
    l_partkey = arrow::field("l_partkey", arrow::int32());
    l_suppkey = arrow::field("l_suppkey", arrow::int32());
    l_linenumber = arrow::field("l_linenumber", arrow::int32());
    l_quantity = arrow::field("l_quantity", arrow::float32());
    l_extendedprice = arrow::field("l_extendedprice", arrow::float32());
    l_discount = arrow::field("l_discount", arrow::float32());
    l_tax = arrow::field("l_tax", arrow::float32());
    l_returnflag = arrow::field("l_returnflag", arrow::utf8());
    l_linestatus = arrow::field("l_linestatus", arrow::utf8());
    l_shipdate = arrow::field("l_shipdate", arrow::int32());
    l_commitdate = arrow::field("l_commitdate", arrow::int32());
    l_receiptdate = arrow::field("l_receiptdate", arrow::int32());
    l_shipinstruct = arrow::field("l_shipinstruct", arrow::utf8());
    l_shipmode = arrow::field("l_shipmode", arrow::utf8());
    l_comment = arrow::field("l_comment", arrow::utf8());
    schema = arrow::schema({l_orderkey, l_partkey, l_suppkey, l_linenumber, l_quantity, l_extendedprice, l_discount, l_tax, l_returnflag, l_linestatus, l_shipdate, l_commitdate, l_receiptdate, l_shipinstruct, l_shipmode, l_comment});
    // 读取文件数据
    std::vector<int> orderkey;
    std::vector<int> partkey;
    std::vector<int> suppkey;
    std::vector<int> linenumber;
    std::vector<float> quantity;
    std::vector<float> extendedprice;
    std::vector<float> discount;
    std::vector<float> tax;
    std::vector<std::string> returnflag;
    std::vector<std::string> linestatus;
    std::vector<int> shipdate;
    std::vector<int> commitdate;
    std::vector<int> receiptdate;
    std::vector<std::string> shipinstruct;
    std::vector<std::string> shipmode;
    std::vector<std::string> comment;
    std::ifstream infile(fileName);
    std::string line;
    if (!infile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return;
    }
    int count = 0;
    int round = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(iss, part, '|')) {
            parts.emplace_back(part);
        }
        orderkey.push_back(std::stoi(parts[0]));
        partkey.push_back(std::stoi(parts[1]));
        suppkey.push_back(std::stoi(parts[2]));
        linenumber.push_back(std::stoi(parts[3]));
        quantity.push_back(std::stof(parts[4]));
        extendedprice.push_back(std::stof(parts[5]));
        discount.push_back(std::stof(parts[6]));
        tax.push_back(std::stof(parts[7]));
        returnflag.emplace_back(parts[8]);
        linestatus.emplace_back(parts[9]);
        shipdate.push_back(std::stoi(parts[10]));
        commitdate.push_back(std::stoi(parts[11]));
        receiptdate.push_back(std::stoi(parts[12]));
        shipinstruct.emplace_back(parts[13]);
        shipmode.emplace_back(parts[14]);
        comment.emplace_back(parts[15]);
        count++;
        if(count == CHUNK_SZIE) {
            round++;
            std::shared_ptr<arrow::Array> orderkey_array;
            std::shared_ptr<arrow::Array> partkey_array;
            std::shared_ptr<arrow::Array> suppkey_array;
            std::shared_ptr<arrow::Array> linenumber_array;
            std::shared_ptr<arrow::Array> quantity_array;
            std::shared_ptr<arrow::Array> extendedprice_array;
            std::shared_ptr<arrow::Array> discount_array;
            std::shared_ptr<arrow::Array> tax_array;
            std::shared_ptr<arrow::Array> returnflag_array;
            std::shared_ptr<arrow::Array> linestatus_array;
            std::shared_ptr<arrow::Array> shipdate_array;
            std::shared_ptr<arrow::Array> commitdate_array;
            std::shared_ptr<arrow::Array> receiptdate_array;
            std::shared_ptr<arrow::Array> shipinstruct_array;
            std::shared_ptr<arrow::Array> shipmode_array;
            std::shared_ptr<arrow::Array> comment_array;
            ok = DaseX::Util::AppendIntArray(orderkey_array, orderkey);
            ok = DaseX::Util::AppendIntArray(partkey_array, partkey);
            ok = DaseX::Util::AppendIntArray(suppkey_array, suppkey);
            ok = DaseX::Util::AppendIntArray(linenumber_array, linenumber);
            ok = DaseX::Util::AppendFloatArray(quantity_array, quantity);
            ok = DaseX::Util::AppendFloatArray(extendedprice_array, extendedprice);
            ok = DaseX::Util::AppendFloatArray(discount_array, discount);
            ok = DaseX::Util::AppendFloatArray(tax_array, tax);
            ok = DaseX::Util::AppendStringArray(returnflag_array, returnflag);
            ok = DaseX::Util::AppendStringArray(linestatus_array, linestatus);
            ok = DaseX::Util::AppendIntArray(shipdate_array, shipdate);
            ok = DaseX::Util::AppendIntArray(commitdate_array, commitdate);
            ok = DaseX::Util::AppendIntArray(receiptdate_array, receiptdate);
            ok = DaseX::Util::AppendStringArray(shipinstruct_array, shipinstruct);
            ok = DaseX::Util::AppendStringArray(shipmode_array, shipmode);
            ok = DaseX::Util::AppendStringArray(comment_array, comment);
            std::shared_ptr<arrow::RecordBatch> rbatch;
            rbatch = arrow::RecordBatch::Make(schema, CHUNK_SZIE, {orderkey_array, partkey_array, suppkey_array, linenumber_array, quantity_array, extendedprice_array, discount_array, tax_array, returnflag_array, linestatus_array, shipdate_array, commitdate_array, receiptdate_array, shipinstruct_array, shipmode_array, comment_array});
            table->insert_data(rbatch, partition_idx);
            count = 0;
            // 清空向量
            orderkey.clear();
            partkey.clear();
            suppkey.clear();
            linenumber.clear();
            quantity.clear();
            extendedprice.clear();
            discount.clear();
            tax.clear();
            returnflag.clear();
            linestatus.clear();
            shipdate.clear();
            commitdate.clear();
            receiptdate.clear();
            shipinstruct.clear();
            shipmode.clear();
            comment.clear();
        }
    }
    if(count != 0) {
        std::shared_ptr<arrow::Array> orderkey_array;
        std::shared_ptr<arrow::Array> partkey_array;
        std::shared_ptr<arrow::Array> suppkey_array;
        std::shared_ptr<arrow::Array> linenumber_array;
        std::shared_ptr<arrow::Array> quantity_array;
        std::shared_ptr<arrow::Array> extendedprice_array;
        std::shared_ptr<arrow::Array> discount_array;
        std::shared_ptr<arrow::Array> tax_array;
        std::shared_ptr<arrow::Array> returnflag_array;
        std::shared_ptr<arrow::Array> linestatus_array;
        std::shared_ptr<arrow::Array> shipdate_array;
        std::shared_ptr<arrow::Array> commitdate_array;
        std::shared_ptr<arrow::Array> receiptdate_array;
        std::shared_ptr<arrow::Array> shipinstruct_array;
        std::shared_ptr<arrow::Array> shipmode_array;
        std::shared_ptr<arrow::Array> comment_array;
        ok = DaseX::Util::AppendIntArray(orderkey_array, orderkey);
        ok = DaseX::Util::AppendIntArray(partkey_array, partkey);
        ok = DaseX::Util::AppendIntArray(suppkey_array, suppkey);
        ok = DaseX::Util::AppendIntArray(linenumber_array, linenumber);
        ok = DaseX::Util::AppendFloatArray(quantity_array, quantity);
        ok = DaseX::Util::AppendFloatArray(extendedprice_array, extendedprice);
        ok = DaseX::Util::AppendFloatArray(discount_array, discount);
        ok = DaseX::Util::AppendFloatArray(tax_array, tax);
        ok = DaseX::Util::AppendStringArray(returnflag_array, returnflag);
        ok = DaseX::Util::AppendStringArray(linestatus_array, linestatus);
        ok = DaseX::Util::AppendIntArray(shipdate_array, shipdate);
        ok = DaseX::Util::AppendIntArray(commitdate_array, commitdate);
        ok = DaseX::Util::AppendIntArray(receiptdate_array, receiptdate);
        ok = DaseX::Util::AppendStringArray(shipinstruct_array, shipinstruct);
        ok = DaseX::Util::AppendStringArray(shipmode_array, shipmode);
        ok = DaseX::Util::AppendStringArray(comment_array, comment);
        std::shared_ptr<arrow::RecordBatch> rbatch;
        rbatch = arrow::RecordBatch::Make(schema, count, {orderkey_array, partkey_array, suppkey_array, linenumber_array, quantity_array, extendedprice_array, discount_array, tax_array, returnflag_array, linestatus_array, shipdate_array, commitdate_array, receiptdate_array, shipinstruct_array, shipmode_array, comment_array});
        table->insert_data(rbatch, partition_idx);
    }
    // spdlog::info("[{} : {}] The round is: {}, the final count is: {} .", __FILE__, __LINE__, round, count);
    infile.close();
    counter.fetch_add(1, std::memory_order_relaxed);
    if(counter == thread_nums) {
        importFinish = true;
        counter.store(0, std::memory_order_relaxed);
        cv.notify_all();
    }
}

void ImportDataFromRevenue(std::shared_ptr<Table> &table, int partition_idx, std::string &fileName, int thread_nums, bool &importFinish) {
    arrow::Status ok;
    // 构建Schema
    std::shared_ptr<arrow::Field> supplier_no, total_revenue;
    std::shared_ptr<arrow::Schema> schema;
    supplier_no = arrow::field("supplier_no", arrow::int32());
    total_revenue = arrow::field("total_revenue", arrow::float64());
    schema = arrow::schema({supplier_no, total_revenue});
    // 读取文件数据
    std::vector<int> supplier;
    std::vector<double> revenue;
    std::ifstream infile(fileName);
    std::string line;
    if (!infile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return;
    }
    int count = 0;
    int round = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(iss, part, '|')) {
            parts.emplace_back(part);
        }
        supplier.push_back(std::stoi(parts[0]));
        revenue.push_back(std::stod(parts[1]));
        count++;
        if(count == CHUNK_SZIE) {
            round++;
            std::shared_ptr<arrow::Array> supplier_array;
            std::shared_ptr<arrow::Array> revenue_array;
            ok = DaseX::Util::AppendIntArray(supplier_array, supplier);
            ok = DaseX::Util::AppendDoubleArray(revenue_array, revenue);
            std::shared_ptr<arrow::RecordBatch> rbatch;
            rbatch = arrow::RecordBatch::Make(schema, CHUNK_SZIE, {supplier_array, revenue_array});
            table->insert_data(rbatch, partition_idx);
            count = 0;
            // 清空向量
            supplier.clear();
            revenue.clear();
        }
    }
    if(count != 0) {
        std::shared_ptr<arrow::Array> supplier_array;
        std::shared_ptr<arrow::Array> revenue_array;
        ok = DaseX::Util::AppendIntArray(supplier_array, supplier);
        ok = DaseX::Util::AppendDoubleArray(revenue_array, revenue);
        std::shared_ptr<arrow::RecordBatch> rbatch;
        rbatch = arrow::RecordBatch::Make(schema, CHUNK_SZIE, {supplier_array, revenue_array});
        table->insert_data(rbatch, partition_idx);
    }
    // spdlog::info("[{} : {}] The round is: {}, the final count is: {} .", __FILE__, __LINE__, round, count);
    infile.close();
    counter.fetch_add(1, std::memory_order_relaxed);
    if(counter == thread_nums) {
        importFinish = true;
        counter.store(0, std::memory_order_relaxed);
        cv.notify_all();
    }
}

void InsertNation(std::shared_ptr<Table> &table, std::shared_ptr<Scheduler> &scheduler, std::string &fileDir, bool &importFinish) {
    std::vector<std::string> filed_names = {"n_nationkey", "n_name", "n_regionkey", "n_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::INTEGER;
    LogicalType filed4  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    std::string table_name = "nation";
    TableInfo nation_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(nation_info);
    rc = global_catalog->get_table(table_name, table);
    std::string fileName = fileDir + "/" + "nation.tbl";
    scheduler->submit_task(ImportDataFromNation, 1, true, std::ref(table), 1, std::ref(fileName), 1, std::ref(importFinish));
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertRegion(std::shared_ptr<Table> &table, std::shared_ptr<Scheduler> &scheduler, std::string &fileDir, bool &importFinish) {
    std::vector<std::string> filed_names = {"r_regionkey", "r_name", "r_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    std::string table_name = "region";
    TableInfo region_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(region_info);
    rc = global_catalog->get_table(table_name, table);
    std::string fileName = fileDir + "/" + "region.tbl";
    scheduler->submit_task(ImportDataFromRegion, 1, true, std::ref(table), 1, std::ref(fileName), 1, std::ref(importFinish));
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertPart(std::shared_ptr<Table> &table, std::shared_ptr<Scheduler> &scheduler, std::string &fileDir, bool &importFinish) {
    std::vector<std::string> filed_names = {"p_partkey", "p_name", "p_mfgr", "p_brand", "p_type", "p_size", "p_container", "p_retailprice", "p_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::STRING;
    LogicalType filed5  = LogicalType::STRING;
    LogicalType filed6  = LogicalType::INTEGER;
    LogicalType filed7  = LogicalType::STRING;
    LogicalType filed8  = LogicalType::FLOAT;
    LogicalType filed9  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    filed_types.push_back(filed9);
    std::string table_name = "part";
    TableInfo part_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(part_info);
    rc = global_catalog->get_table(table_name, table);
    std::string fileName = fileDir + "/" + "part.tbl";
    scheduler->submit_task(ImportDataFromPart, 1, true, std::ref(table), 1, std::ref(fileName), 1, std::ref(importFinish));
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertSupplier(std::shared_ptr<Table> &table, std::shared_ptr<Scheduler> &scheduler, std::string &fileDir, bool &importFinish) {
    std::vector<std::string> filed_names = {"s_suppkey", "s_name", "s_address", "s_nationkey", "s_phone", "s_acctbal", "s_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::INTEGER;
    LogicalType filed5  = LogicalType::STRING;
    LogicalType filed6  = LogicalType::FLOAT;
    LogicalType filed7  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    std::string table_name = "supplier";
    TableInfo supplier_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(supplier_info);
    rc = global_catalog->get_table(table_name, table);
    std::string fileName = fileDir + "/" + "supplier.tbl";
    scheduler->submit_task(ImportDataFromSupplier, 1, true, std::ref(table), 1, std::ref(fileName), 1, std::ref(importFinish));
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertPartsupp(std::shared_ptr<Table> &table, std::shared_ptr<Scheduler> &scheduler, std::string &fileDir, bool &importFinish) {
    std::vector<std::string> filed_names = {"ps_partkey", "ps_suppkey", "ps_availqty", "ps_supplycost", "ps_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::INTEGER;
    LogicalType filed3  = LogicalType::INTEGER;
    LogicalType filed4  = LogicalType::FLOAT;
    LogicalType filed5  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    std::string table_name = "partsupp";
    TableInfo partsupp_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(partsupp_info);
    rc = global_catalog->get_table(table_name, table);
    std::string fileName = fileDir + "/" + "partsupp.tbl";
    scheduler->submit_task(ImportDataFromPartsupp, 1, true, std::ref(table), 1, std::ref(fileName), 1, std::ref(importFinish));
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertCustomer(std::shared_ptr<Table> &table, std::shared_ptr<Scheduler> &scheduler, std::string &fileDir, bool &importFinish) {
    std::vector<std::string> filed_names = {"c_custkey", "c_name", "c_address", "c_nationkey", "c_phone", "c_acctbal", "c_mktsegment", "c_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::INTEGER;
    LogicalType filed5  = LogicalType::STRING;
    LogicalType filed6  = LogicalType::FLOAT;
    LogicalType filed7  = LogicalType::STRING;
    LogicalType filed8  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    std::string table_name = "customer";
    TableInfo customer_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(customer_info);
    rc = global_catalog->get_table(table_name, table);
    std::string fileName = fileDir + "/" + "customer.tbl";
    scheduler->submit_task(ImportDataFromCustomer, 1, true, std::ref(table), 1, std::ref(fileName), 1, std::ref(importFinish));
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertOrders(std::shared_ptr<Table> &table, std::shared_ptr<Scheduler> &scheduler, std::string &fileDir, bool &importFinish) {
    std::vector<std::string> filed_names = {"o_orderkey", "o_custkey", "o_orderstatus", "o_totalprice", "o_orderdate", "o_orderpriority", "o_clerk", "o_shippriority", "o_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::INTEGER;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::FLOAT;
    LogicalType filed5  = LogicalType::INTEGER;
    LogicalType filed6  = LogicalType::STRING;
    LogicalType filed7  = LogicalType::STRING;
    LogicalType filed8  = LogicalType::INTEGER;
    LogicalType filed9  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    filed_types.push_back(filed9);
    std::string table_name = "orders";
    TableInfo orders_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(orders_info);
    rc = global_catalog->get_table(table_name, table);
    std::string fileName = fileDir + "/" + "orders.tbl";
    scheduler->submit_task(ImportDataFromOrders, 1, true, std::ref(table), 1, std::ref(fileName), 1, std::ref(importFinish));
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertLineitem(std::shared_ptr<Table> &table, std::shared_ptr<Scheduler> &scheduler, std::string &fileDir, bool &importFinish) {
    std::vector<std::string> filed_names = {"l_orderkey", "l_partkey", "l_suppkey", "l_linenumber", "l_quantity", "l_extendedprice", "l_discount", "l_tax", "l_returnflag", "l_linestatus", "l_shipdate", "l_commitdate", "l_receiptdate", "l_shipinstruct", "l_shipmode", "l_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::INTEGER;
    LogicalType filed3  = LogicalType::INTEGER;
    LogicalType filed4  = LogicalType::INTEGER;
    LogicalType filed5  = LogicalType::FLOAT;
    LogicalType filed6  = LogicalType::FLOAT;
    LogicalType filed7  = LogicalType::FLOAT;
    LogicalType filed8  = LogicalType::FLOAT;
    LogicalType filed9  = LogicalType::STRING;
    LogicalType filed10 = LogicalType::STRING;
    LogicalType filed11 = LogicalType::INTEGER;
    LogicalType filed12 = LogicalType::INTEGER;
    LogicalType filed13 = LogicalType::INTEGER;
    LogicalType filed14 = LogicalType::STRING;
    LogicalType filed15 = LogicalType::STRING;
    LogicalType filed16 = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    filed_types.push_back(filed9);
    filed_types.push_back(filed10);
    filed_types.push_back(filed11);
    filed_types.push_back(filed12);
    filed_types.push_back(filed13);
    filed_types.push_back(filed14);
    filed_types.push_back(filed15);
    filed_types.push_back(filed16);
    std::string table_name = "lineitem";
    TableInfo lineitem_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(lineitem_info);
    rc = global_catalog->get_table(table_name, table);
    std::string fileName = fileDir + "/" + "lineitem.tbl";
    scheduler->submit_task(ImportDataFromLineitem, 1, true, std::ref(table), 1, std::ref(fileName), 1, std::ref(importFinish));
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertRevenue(std::shared_ptr<Table> &table, std::shared_ptr<Scheduler> &scheduler, std::string &fileDir, bool &importFinish) {
    std::vector<std::string> filed_names = {"supplier_no", "total_revenue"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::DOUBLE;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    std::string table_name = "revenue";
    TableInfo revenue_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(revenue_info);
    rc = global_catalog->get_table(table_name, table);
    std::string fileName = fileDir + "/" + "revenue.tbl";
    scheduler->submit_task(ImportDataFromRevenue, 1, true, std::ref(table), 1, std::ref(fileName), 1, std::ref(importFinish));
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertNationMul(std::shared_ptr<Table> &table,
                     std::shared_ptr<Scheduler> &scheduler,
                     std::vector<int> &work_ids,
                     std::vector<int> &partition_idxs,
                     std::vector<std::string> &file_names,
                     bool &importFinish) {
    std::vector<std::string> filed_names = {"n_nationkey", "n_name", "n_regionkey", "n_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::INTEGER;
    LogicalType filed4  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    std::string table_name = "nation";
    TableInfo nation_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(nation_info);
    rc = global_catalog->get_table(table_name, table);
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(ImportDataFromNation, work_ids[i], true, std::ref(table), partition_idxs[i], std::ref(file_names[i]), size, std::ref(importFinish));
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertRegionMul(std::shared_ptr<Table> &table,
                     std::shared_ptr<Scheduler> &scheduler,
                     std::vector<int> &work_ids,
                     std::vector<int> &partition_idxs,
                     std::vector<std::string> &file_names,
                     bool &importFinish) {
    std::vector<std::string> filed_names = {"r_regionkey", "r_name", "r_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    std::string table_name = "region";
    TableInfo region_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(region_info);
    rc = global_catalog->get_table(table_name, table);
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(ImportDataFromRegion, work_ids[i], true, std::ref(table), partition_idxs[i], std::ref(file_names[i]), size, std::ref(importFinish));
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertPartMul(std::shared_ptr<Table> &table,
                   std::shared_ptr<Scheduler> &scheduler,
                   std::vector<int> &work_ids,
                   std::vector<int> &partition_idxs,
                   std::vector<std::string> &file_names,
                   bool &importFinish) {
    std::vector<std::string> filed_names = {"p_partkey", "p_name", "p_mfgr", "p_brand", "p_type", "p_size", "p_container", "p_retailprice", "p_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::STRING;
    LogicalType filed5  = LogicalType::STRING;
    LogicalType filed6  = LogicalType::INTEGER;
    LogicalType filed7  = LogicalType::STRING;
    LogicalType filed8  = LogicalType::FLOAT;
    LogicalType filed9  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    filed_types.push_back(filed9);
    std::string table_name = "part";
    TableInfo part_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(part_info);
    rc = global_catalog->get_table(table_name, table);
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(ImportDataFromPart, work_ids[i], true, std::ref(table), partition_idxs[i], std::ref(file_names[i]), size, std::ref(importFinish));
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertSupplierMul(std::shared_ptr<Table> &table,
                       std::shared_ptr<Scheduler> &scheduler,
                       std::vector<int> &work_ids,
                       std::vector<int> &partition_idxs,
                       std::vector<std::string> &file_names,
                       bool &importFinish) {
    std::vector<std::string> filed_names = {"s_suppkey", "s_name", "s_address", "s_nationkey", "s_phone", "s_acctbal", "s_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::INTEGER;
    LogicalType filed5  = LogicalType::STRING;
    LogicalType filed6  = LogicalType::FLOAT;
    LogicalType filed7  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    std::string table_name = "supplier";
    TableInfo supplier_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(supplier_info);
    rc = global_catalog->get_table(table_name, table);
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(ImportDataFromSupplier, work_ids[i], true, std::ref(table), partition_idxs[i], std::ref(file_names[i]), size, std::ref(importFinish));
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertPartsuppMul(std::shared_ptr<Table> &table,
                       std::shared_ptr<Scheduler> &scheduler,
                       std::vector<int> &work_ids,
                       std::vector<int> &partition_idxs,
                       std::vector<std::string> &file_names,
                       bool &importFinish) {
    std::vector<std::string> filed_names = {"ps_partkey", "ps_suppkey", "ps_availqty", "ps_supplycost", "ps_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::INTEGER;
    LogicalType filed3  = LogicalType::INTEGER;
    LogicalType filed4  = LogicalType::FLOAT;
    LogicalType filed5  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    std::string table_name = "partsupp";
    TableInfo partsupp_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(partsupp_info);
    rc = global_catalog->get_table(table_name, table);
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(ImportDataFromPartsupp, work_ids[i], true, std::ref(table), partition_idxs[i], std::ref(file_names[i]), size, std::ref(importFinish));
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertCustomerMul(std::shared_ptr<Table> &table,
                       std::shared_ptr<Scheduler> &scheduler,
                       std::vector<int> &work_ids,
                       std::vector<int> &partition_idxs,
                       std::vector<std::string> &file_names,
                       bool &importFinish) {
    std::vector<std::string> filed_names = {"c_custkey", "c_name", "c_address", "c_nationkey", "c_phone", "c_acctbal", "c_mktsegment", "c_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::STRING;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::INTEGER;
    LogicalType filed5  = LogicalType::STRING;
    LogicalType filed6  = LogicalType::FLOAT;
    LogicalType filed7  = LogicalType::STRING;
    LogicalType filed8  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    std::string table_name = "customer";
    TableInfo customer_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(customer_info);
    rc = global_catalog->get_table(table_name, table);
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(ImportDataFromCustomer, work_ids[i], true, std::ref(table), partition_idxs[i], std::ref(file_names[i]), size, std::ref(importFinish));
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertOrdersMul(std::shared_ptr<Table> &table,
                     std::shared_ptr<Scheduler> &scheduler,
                     std::vector<int> &work_ids,
                     std::vector<int> &partition_idxs,
                     std::vector<std::string> &file_names,
                     bool &importFinish) {
    std::vector<std::string> filed_names = {"o_orderkey", "o_custkey", "o_orderstatus", "o_totalprice", "o_orderdate", "o_orderpriority", "o_clerk", "o_shippriority", "o_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::INTEGER;
    LogicalType filed3  = LogicalType::STRING;
    LogicalType filed4  = LogicalType::FLOAT;
    LogicalType filed5  = LogicalType::INTEGER;
    LogicalType filed6  = LogicalType::STRING;
    LogicalType filed7  = LogicalType::STRING;
    LogicalType filed8  = LogicalType::INTEGER;
    LogicalType filed9  = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    filed_types.push_back(filed9);
    std::string table_name = "orders";
    TableInfo orders_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(orders_info);
    rc = global_catalog->get_table(table_name, table);
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(ImportDataFromOrders, work_ids[i], true, std::ref(table), partition_idxs[i], std::ref(file_names[i]), size, std::ref(importFinish));
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

// 多线程插入LineItem表，需要额外指定线程id, 要插入的表分区以及文件名。例如，如果要用4个线程插入lineitem.tbl数据，需要将其预处理分为4个文件，命名方式为lineitem.tbl_0、lineitem.tbl_1、lineitem.tbl_2、lineitem.tbl_3
void InsertLineitemMul(std::shared_ptr<Table> &table,
                       std::shared_ptr<Scheduler> &scheduler,
                       std::vector<int> &work_ids,
                       std::vector<int> &partition_idxs,
                       std::vector<std::string> &file_names,
                       bool &importFinish) {
    std::vector<std::string> filed_names = {"l_orderkey", "l_partkey", "l_suppkey", "l_linenumber", "l_quantity", "l_extendedprice", "l_discount", "l_tax", "l_returnflag", "l_linestatus", "l_shipdate", "l_commitdate", "l_receiptdate", "l_shipinstruct", "l_shipmode", "l_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::INTEGER;
    LogicalType filed3  = LogicalType::INTEGER;
    LogicalType filed4  = LogicalType::INTEGER;
    LogicalType filed5  = LogicalType::FLOAT;
    LogicalType filed6  = LogicalType::FLOAT;
    LogicalType filed7  = LogicalType::FLOAT;
    LogicalType filed8  = LogicalType::FLOAT;
    LogicalType filed9  = LogicalType::STRING;
    LogicalType filed10 = LogicalType::STRING;
    LogicalType filed11 = LogicalType::INTEGER;
    LogicalType filed12 = LogicalType::INTEGER;
    LogicalType filed13 = LogicalType::INTEGER;
    LogicalType filed14 = LogicalType::STRING;
    LogicalType filed15 = LogicalType::STRING;
    LogicalType filed16 = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    filed_types.push_back(filed9);
    filed_types.push_back(filed10);
    filed_types.push_back(filed11);
    filed_types.push_back(filed12);
    filed_types.push_back(filed13);
    filed_types.push_back(filed14);
    filed_types.push_back(filed15);
    filed_types.push_back(filed16);
    std::string table_name = "lineitem";
    TableInfo lineitem_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(lineitem_info);
    rc = global_catalog->get_table(table_name, table);
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(ImportDataFromLineitem, work_ids[i], true, std::ref(table), partition_idxs[i], std::ref(file_names[i]), size, std::ref(importFinish));
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertLineitemMul2(std::shared_ptr<Table> &table,
                       std::shared_ptr<SchedulerRead> &scheduler,
                       std::vector<int> &work_ids,
                       std::vector<int> &partition_idxs,
                       std::vector<std::string> &file_names,
                       bool &importFinish) {
    std::vector<std::string> filed_names = {"l_orderkey", "l_partkey", "l_suppkey", "l_linenumber", "l_quantity", "l_extendedprice", "l_discount", "l_tax", "l_returnflag", "l_linestatus", "l_shipdate", "l_commitdate", "l_receiptdate", "l_shipinstruct", "l_shipmode", "l_comment"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::INTEGER;
    LogicalType filed3  = LogicalType::INTEGER;
    LogicalType filed4  = LogicalType::INTEGER;
    LogicalType filed5  = LogicalType::FLOAT;
    LogicalType filed6  = LogicalType::FLOAT;
    LogicalType filed7  = LogicalType::FLOAT;
    LogicalType filed8  = LogicalType::FLOAT;
    LogicalType filed9  = LogicalType::STRING;
    LogicalType filed10 = LogicalType::STRING;
    LogicalType filed11 = LogicalType::INTEGER;
    LogicalType filed12 = LogicalType::INTEGER;
    LogicalType filed13 = LogicalType::INTEGER;
    LogicalType filed14 = LogicalType::STRING;
    LogicalType filed15 = LogicalType::STRING;
    LogicalType filed16 = LogicalType::STRING;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    filed_types.push_back(filed3);
    filed_types.push_back(filed4);
    filed_types.push_back(filed5);
    filed_types.push_back(filed6);
    filed_types.push_back(filed7);
    filed_types.push_back(filed8);
    filed_types.push_back(filed9);
    filed_types.push_back(filed10);
    filed_types.push_back(filed11);
    filed_types.push_back(filed12);
    filed_types.push_back(filed13);
    filed_types.push_back(filed14);
    filed_types.push_back(filed15);
    filed_types.push_back(filed16);
    std::string table_name = "lineitem";
    TableInfo lineitem_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(lineitem_info);
    rc = global_catalog->get_table(table_name, table);
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(ImportDataFromLineitem, work_ids[i], true, std::ref(table), partition_idxs[i], std::ref(file_names[i]), size, std::ref(importFinish));
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}

void InsertRevenueMul(std::shared_ptr<Table> &table,
                      std::shared_ptr<Scheduler> &scheduler,
                      std::vector<int> &work_ids,
                      std::vector<int> &partition_idxs,
                      std::vector<std::string> &file_names,
                      bool &importFinish) {
    std::vector<std::string> filed_names = {"supplier_no", "total_revenue"};
    std::vector<LogicalType> filed_types;
    LogicalType filed1  = LogicalType::INTEGER;
    LogicalType filed2  = LogicalType::DOUBLE;
    filed_types.push_back(filed1);
    filed_types.push_back(filed2);
    std::string table_name = "revenue";
    TableInfo revenue_info(table_name, filed_names, filed_types);
    RC rc = global_catalog->register_table(revenue_info);
    rc = global_catalog->get_table(table_name, table);
    int size = work_ids.size();
    for(int i = 0; i < size; i++) {
        scheduler->submit_task(ImportDataFromRevenue, work_ids[i], true, std::ref(table), partition_idxs[i], std::ref(file_names[i]), size, std::ref(importFinish));
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&importFinish] { return importFinish; });
}