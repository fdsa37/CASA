//
// Created by sq111 on 25-5-29.
//

#ifndef INDEX_TABLE_H
#define INDEX_TABLE_H

#define CN 200000
#include <cstdint>
#include "BucketCell.h"

struct counter{
    uint32_t x;
    int count;
};

class Index_table {
private:
    int d;
    int group;
    int group_num;
    int* line_empty = new int[CN];
    counter* Index_counter = new counter[CN];
    int llen;
    float IT_b;
public:
    Index_table(int llen);

    bool ifexist(const uint32_t x_flow_label);

    void update(const uint32_t x_flow_label, const uint32_t y_flow_label, BucketCell& bucket_cell);

    counter get_counter(const uint32_t location);

    ~Index_table() {
        //delete[] Index_counter;
    }
};
#endif //INDEX_TABLE_H
