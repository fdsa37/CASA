#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "header/BucketCell.h"
#include "header/Index_table.h"
#include "header/OSFilter.h"
using namespace std;

vector<pair<uint32_t,pair<uint32_t,uint32_t>>> quadratic_data;

void loadDataSetMAWI() {
    std::vector<std::string> file_paths = {
            "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/MAWI 2022/parsed_mawi_20220101.csv",
            "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/MAWI 2022/parsed_mawi_20220102.csv",
            "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/MAWI 2022/parsed_mawi_20220103.csv",
            // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/MAWI 2022/parsed_mawi_20220104.csv",
            // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/MAWI 2024/parsed_mawi_20240101.csv"
    };

    std::vector<std::pair<uint32_t, uint32_t>> data;
    unordered_map<uint32_t, unordered_set<uint32_t>> true_cardinality;
    uint64_t total_data_num = 0;

    auto parse_ip = [](const std::string& ip_str) -> optional<uint32_t> {
        std::stringstream ss(ip_str);
        uint32_t result = 0;
        int part;

        for (int i = 0; i < 4; ++i) {
            if (!(ss >> part)) return std::nullopt;
            if (part < 0 || part > 255) return std::nullopt;
            result = (result << 8) | part;
            if (i < 3) {
                if (ss.peek() != '.') return std::nullopt;
                ss.ignore();
            }
        }

        if (ss.rdbuf()->in_avail() != 0) return std::nullopt;
        return result;
    };

    for (const auto& file_path : file_paths) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << file_path << std::endl;
            continue;
        }

        std::string line;
        if (!std::getline(file, line)) {
            std::cerr << "Empty file: " << file_path << std::endl;
            continue;
        }

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string src_ip_str, dst_ip_str;

            if (!std::getline(ss, src_ip_str, ',')) continue;
            if (!std::getline(ss, dst_ip_str, ',')) continue;

            auto src_ip_opt = parse_ip(src_ip_str);
            auto dst_ip_opt = parse_ip(dst_ip_str);
            if (!src_ip_opt || !dst_ip_opt) continue;

            uint32_t src_ip = src_ip_opt.value();
            uint32_t dst_ip = dst_ip_opt.value();

            quadratic_data.push_back(make_pair(total_data_num,make_pair(src_ip, dst_ip)));

            total_data_num++;
        }

        std::cout << "Loaded file: " << file_path << std::endl;
    }
}

void readDataSet() {
    std::vector<std::string> file_paths = {
        "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA 2016/caida_2016_ipv4_00.txt",
        "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA 2016/caida_2016_ipv4_01.txt",
        // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA2019/00.txt",
        // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA2019/01.txt",
        // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA2019/02.txt",
        // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA2019/03.txt",
        // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA2019/04.txt",
        // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA2019/05.txt",
        // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA2019/06.txt",
        // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA2019/07.txt",
        // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA2019/08.txt",
        // "D:/Paper/SFSS_Experiment/ATotal_Experiment/Data/CAIDA2019/09.txt",
    };

    uint64_t total_data_num = 0;

    for (const auto& file_path : file_paths) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << file_path << std::endl;
            continue;
        }

        std::string line;
        while (getline(file, line)) {
            std::istringstream iss(line);
            std::string source_ip, dest_ip;
            iss >> source_ip >> dest_ip;
            if (source_ip.empty() || dest_ip.empty()) continue;

            uint32_t src_ip_int = 0, dst_ip_int = 0;
            int part = 0;
            char dot;
            std::istringstream ss1(source_ip), ss2(dest_ip);
            bool valid = true;

            for (int i = 0; i < 4; i++) {
                if (!(ss1 >> part)) { valid = false; break; }
                if (part < 0 || part > 255) { valid = false; break; }
                src_ip_int = (src_ip_int << 8) | part;
                if (i < 3 && !(ss1 >> dot && dot == '.')) { valid = false; break; }
            }
            if (ss1 >> dot) valid = false;

            for (int i = 0; i < 4 && valid; i++) {
                if (!(ss2 >> part)) { valid = false; break; }
                if (part < 0 || part > 255) { valid = false; break; }
                dst_ip_int = (dst_ip_int << 8) | part;
                if (i < 3 && !(ss2 >> dot && dot == '.')) { valid = false; break; }
            }
            if (ss2 >> dot) valid = false;

            if (!valid) continue;

            quadratic_data.push_back(make_pair(total_data_num,make_pair(src_ip_int, dst_ip_int)));
            total_data_num++;
        }

        std::cout << file_path << " is loaded." << std::endl;
    }
}

void quadratic_element_update(
    Index_table& index_table,
    BucketCell& bucketcell,
    OSFilter& osfilter
    ) {

    for (const auto& data : quadratic_data) {
        if (index_table.ifexist(data.second.first) != 0) {
            index_table.update(data.second.first, data.second.second, bucketcell);
        }else {
            osfilter.update(data.first, data.second.first, data.second.second, index_table, bucketcell);
        }
    }
}

map<pair<uint32_t,uint32_t>, uint32_t> report_quadratic_elements(OSFilter& OSFilter, Index_table& index_table, BucketCell& bucketcell, int ph1, float ph2, int llen) {
    map<pair<uint32_t,uint32_t>, uint32_t> result;

    for (int i = 0; i < llen; i++) {
        counter cur_quadra_eleme_counter = index_table.get_counter(i);
        uint32_t count = cur_quadra_eleme_counter.count;
        if (count >= ph1) {
            float ph2_times_fx = ph2 * static_cast<float>(count);
            map<pair<uint32_t,uint32_t>, uint32_t> sub_result = bucketcell.report(i, cur_quadra_eleme_counter.x, ph2_times_fx);
            for (const auto& entry : sub_result) {
                result.insert(entry);
            }
        }
    }

    return result;
}

void calculateMetrics(const std::map<std::pair<uint32_t, uint32_t>, uint32_t>& real,
                      const std::map<std::pair<uint32_t, uint32_t>, uint32_t>& es) {
    int TP = 0;
    int FP = 0;
    int FN = 0;

    for (const auto& entry : es) {
        if (real.count(entry.first)) {
            TP++;
        } else {
            FP++;
        }
    }

    for (const auto& entry : real) {
        if (!es.count(entry.first)) {
            FN++;
        }
    }

    double precision = (TP + FP) ? static_cast<double>(TP) / (TP + FP) : 0.0;
    double recall = (TP + FN) ? static_cast<double>(TP) / (TP + FN) : 0.0;
    double f1 = (precision + recall) ? 2 * precision * recall / (precision + recall) : 0.0;

    std::cout << "Precision: " << precision << std::endl;
    std::cout << "Recall: " << recall << std::endl;
    std::cout << "F1 Score: " << f1 << std::endl;
}

void process(Index_table& index_table, BucketCell& bucketcell, OSFilter os_filter, int ph1, float ph2, map<pair<uint32_t,uint32_t>, uint32_t>& es_quadratic_eleme_sizes,int llen) {
    readDataSet();
    // loadDataSetMAWI();

    map<uint32_t, uint32_t> real_data;
    map<pair<uint32_t,uint32_t>, uint32_t> real_qua_data;
    map<pair<uint32_t,uint32_t>, uint32_t> real_qua_data_result;
    for (const auto& entry : quadratic_data) {
        real_qua_data[entry.second]++;
        real_data[entry.second.first]++;
    }
    for (const auto& entry : real_qua_data) {
        if (entry.second >= ph2 * real_data[entry.first.first] && real_data[entry.first.first] >= ph1) {
            real_qua_data_result[entry.first] = real_data[entry.second];
        }
    }

    clock_t start = clock();
    quadratic_element_update(index_table, bucketcell, os_filter);
    clock_t end = clock();

    double throughput_mpps = static_cast<double>(quadratic_data.size()) / ((double)(end - start)/ CLOCKS_PER_SEC) / 1000000;
    cout << "throughput: " << throughput_mpps << endl;

    es_quadratic_eleme_sizes = report_quadratic_elements(os_filter, index_table, bucketcell, ph1, ph2, llen);
    calculateMetrics(real_qua_data_result,es_quadratic_eleme_sizes);

}

int main() {
    system("chcp 65001");

    cout << "Hello! Program starts executing......" << endl;
    cout << "-------------------------------\n" << endl;

    float total_memory_kb = 500;
    cout << "Process: memory_kb="<< total_memory_kb << "KB | " << endl;
    float ratio = 0.05;
    float filter_memory_kb = ratio * total_memory_kb;
    int cnum = 120;
    int llen = 0.6 * total_memory_kb * 1024 /8;
    int bnum = static_cast<int>((1 - ratio) * total_memory_kb * 1024 - llen * 8)/cnum/7;
    int ph1 = 4000;
    float ph2 = 0.1;

    BucketCell bucketcell = BucketCell(bnum,cnum);
    Index_table index_table = Index_table(llen);
    OSFilter osfilter = OSFilter(filter_memory_kb);
    map<pair<uint32_t, uint32_t>, uint32_t> es_quadratic_data;
    process(index_table, bucketcell, osfilter, ph1, ph2,es_quadratic_data,llen);
    cout << "-------------------------------\n" << endl;

    return 0;

}