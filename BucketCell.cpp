//
// Created by sq111 on 25-5-29.
//

#include "header/BucketCell.h"

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>

#include "header/MurmurHash3.h"

BucketCell::BucketCell(int num1,int num2) {
    countbit = 9;
    bnum = num1;
    cnum = num2;

    x_intra_group_num = cnum * 0.1;

    BC_b = 1.08;

    for (int i = 0; i < MaxBuckets; ++i) {
        BCell[i] = (cell*)malloc(sizeof(cell) * MaxCell);
        for (int j = 0; j < MaxCell; ++j) {
            BCell[i][j].count = 0;
            BCell[i][j].x_fingerprint = 0;
            BCell[i][j].y_fingerprint = 0;
        }
    }
}

void BucketCell::update(unsigned int x_value, const unsigned int x_index, const uint32_t y) {
    unsigned int Bucket_value = x_value % bnum;
    unsigned int Cell_start = x_value % cnum;

    bool flag = false;
    for (int i = 0; i < x_intra_group_num; i++) {
        unsigned int Cell_value = (Cell_start + i)%cnum;
        if (BCell[Bucket_value][Cell_value].x_fingerprint == x_index && BCell[Bucket_value][Cell_value].y_fingerprint == y) {
            uint16_t byte = BCell[Bucket_value][Cell_value].count;
            if (((byte >> 8) & 1) == 0) {
                BCell[Bucket_value][Cell_value].count ++;
            }else if (((byte >> 8) & 1) == 1){
                uint16_t alpha = (byte >> encode_bit) & 0b00111111;
                uint16_t beta = byte & 0b00000111;

                int beta_t = encode_table[beta];
                int gamma = encode_bit;
                if (!(rand() % (int)pow(2, (gamma + beta_t)))) {
                    alpha += 1;
                    if (alpha >= pow(2,countbit - encode_bit) - 1 && beta < pow(2,encode_bit) - 1 ) {
                        alpha = pow(2,countbit - encode_bit - 1);
                        beta += 1;
                    }
                }
                uint16_t high5 = (alpha & 0b00111111);
                high5 <<= 3;
                uint16_t low3 = (beta & 0b00000111);
                uint32_t combined = high5 | low3;
                BCell[Bucket_value][Cell_value].count = combined;
            }
            flag = true;
        }
    }

    if (!flag) {
        int min_cnt = INT_MAX;
        unsigned int min_index = 0;
        for (int i = 0; i < x_intra_group_num; ++i) {
            unsigned int Cell_value = (Cell_start + i)%cnum;
            if (BCell[Bucket_value][Cell_value].count < min_cnt) {
                min_cnt = BCell[Bucket_value][Cell_value].count;
                min_index = Cell_value;
                if (min_cnt == 0) {
                    break;
                }
            }
        }
        uint8_t byte = min_cnt;
        if (((byte >> 8) & 1) == 0) {
            if (!(rand() % static_cast<int>(pow(BC_b, BCell[Bucket_value][min_index].count)))) {
                BCell[Bucket_value][min_index].count--;
                if (BCell[Bucket_value][min_index].count <= 0) {
                    BCell[Bucket_value][min_index].x_fingerprint = x_index;
                    BCell[Bucket_value][min_index].y_fingerprint = y;
                    BCell[Bucket_value][min_index].count = 1;
                }
            }
        }
    }
}

std::map<std::pair<uint32_t,uint32_t>, uint32_t> BucketCell::report(int x_index, const uint32_t x, float ph2) {
    unsigned int hash_start;
    MurmurHash3_x86_32(&x,4,10,&hash_start);
    unsigned int Bucket_value = hash_start % bnum;
    unsigned int Cell_start = hash_start % cnum;

    std::map<std::pair<uint32_t,uint32_t>, uint32_t> sub_result;

    for (int i = 0; i< x_intra_group_num ;i++) {
        unsigned int Cell_value = (Cell_start + i) % cnum;
        cell cell_x = BCell[Bucket_value][Cell_value];
        uint32_t count = decode_counter(cell_x.count);
        if (cell_x.x_fingerprint == x_index && count >= ph2) {
            std::pair<uint32_t,uint32_t> qua_ele = {x, cell_x.y_fingerprint};
            sub_result[qua_ele] = count;
        }
    }
    return sub_result;
}

uint32_t BucketCell::decode_counter(unsigned int count) {
    uint32_t result;
    if (count >= pow(2, countbit - 1)) {
        uint8_t alpha = (count >> encode_bit) & 0b00111111;
        uint8_t beta = count & 0b00000111;
        int beta_t = encode_table[beta];
        int gamma = encode_bit;
        result = alpha * pow(2,gamma + beta_t);
    }else {
        result = count;
    }
    return result;
}

void BucketCell::clear_bucket(int x_index, const unsigned int x_value) {
    unsigned int Bucket_value = x_value % bnum;
    unsigned int Cell_start = x_value % cnum;

    for (int i = 0; i < x_intra_group_num; i++) {
        unsigned int Cell_value = (Cell_start + i) % cnum;
        if (BCell[Bucket_value][Cell_value].x_fingerprint == x_index) {
            BCell[Bucket_value][Cell_value].x_fingerprint = 0;
            BCell[Bucket_value][Cell_value].y_fingerprint = 0;
            BCell[Bucket_value][Cell_value].count = 0;
        }
    }
}