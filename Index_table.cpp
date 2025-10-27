//
// Created by sq111 on 25-5-29.
//
#include "header/Index_table.h"

#include <chrono>
#include <cmath>
#include <fstream>

#include "header/MurmurHash3.h"

int total_packet = 0;
int ge_packet = 0;
int le_packet = 0;

Index_table::Index_table(int len) {

    d = 1;
    group = 4;
    llen = len;
    group_num = len / group;
    for (int i = 0; i < llen; i++) {
        Index_counter[i].count = 0;
        Index_counter[i].x = 0;
    }
    for (int i = 0; i < group_num + 5; i++) {
        line_empty[i] = 0;
    }

    IT_b = 1.08;
}


bool Index_table::ifexist(const uint32_t x_flow_label) {
    unsigned int hash_start;
    MurmurHash3_x86_32(&x_flow_label,4,10,&hash_start);
    unsigned int index = hash_start % group_num;
    index = index * group;

    for (int i = 0; i < group; i++) {
        if (Index_counter[index + i].x == x_flow_label) {
            return true;
        }
    }
    return false;
}

void Index_table::update(const uint32_t x_flow_label, const uint32_t y_flow_label, BucketCell& bucket_cell) {
    unsigned int x_value;
    MurmurHash3_x86_32(&x_flow_label,4,10,&x_value);
    unsigned int group_index = x_value % group_num;
    unsigned int index = group_index * group;

    bool flag = false;
    for (int i = 0; i < group; i++) {
        if (Index_counter[index + i].x == x_flow_label) {
            Index_counter[index + i].count++;
            bucket_cell.update(x_value, index + i, y_flow_label);
            flag = true;
        }
    }

    if (!flag) {
        if (line_empty[group_index] < group) {
            int empty_temp = index + line_empty[group_index];
            Index_counter[empty_temp].x = x_flow_label;
            Index_counter[empty_temp].count = 1;
            bucket_cell.update(x_value,empty_temp,y_flow_label);
            line_empty[group_index]++;
        }else {
            int min_cnt = INT_MAX;
            int min_index = 0;
            for (int i = 0; i < group; i++) {
                if (Index_counter[index + i].count < min_cnt) {
                    min_cnt = Index_counter[index + i].count;
                    min_index = index + i;
                }
            }
            if (!(rand()%int(pow(IT_b,Index_counter[min_index].count)))) {
                Index_counter[min_index].count--;
                if (Index_counter[min_index].count <= 0) {
                    bucket_cell.clear_bucket(min_index, x_value);
                    bucket_cell.update(x_value, min_index, y_flow_label);
                    Index_counter[min_index].x = x_flow_label;
                    Index_counter[min_index].count = 1;
                }
            }
        }
    }
}

counter Index_table::get_counter(const uint32_t location) {
    return Index_counter[location];
}
