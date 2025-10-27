//
// Created by sq111 on 25-5-29.
//

#ifndef OSFILTER_H
#define OSFILTER_H

#include <cstdint>

#include "Index_table.h"

class OSFilter{
private:
    int d; // num of hash per layer
    int bits; // counter bits per layer
    int max_positive;
    int min_negative;
    uint32_t num_counters; // num of counters per layer
    int8_t* counters;

    //Optimization
    int counters_inter_group_num;
    int counters_group_num;
    int hash_group;
    int read_counters;

public:
    OSFilter(float memory_kb);

    int hash_s(const uint32_t flow_label);

    void update(const uint32_t packet_id, const uint32_t x_flow_label, const uint32_t y_flow_label, Index_table& index_table,BucketCell& bucket_cell, uint32_t weight= 1);

    int report(const uint32_t flow_label);

    //optimization
    uint32_t hash_start_group(const uint32_t flow_label);

    bool check(const uint32_t packet_id, const uint32_t flow_label);

    void readcounters(const int8_t* counters, uint32_t start_index, int8_t* out);

    int8_t get_counter(uint32_t i) {
        return counters[i];
    }

    void set_counter(uint32_t i, int8_t val) {
        counters[i] = val;
    }

    int get_counter_num() {
        return num_counters;
    }

    void writeCountersToTextFile();

    // Destructor to free memory
    ~OSFilter() {
        //delete[] counters;
    }

};

#endif //OSFILTER_H

