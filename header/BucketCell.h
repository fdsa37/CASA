//
// Created by sq111 on 25-5-29.
//

#ifndef BUCKETCELL_H
#define BUCKETCELL_H

#define MaxCell 550
#define MaxBuckets 10000
#include <cstdint>
#include <map>
#include <stdlib.h>
#include <vector>


struct cell{
    uint32_t x_fingerprint;
    uint32_t y_fingerprint;
    int count;
};

class BucketCell {
private:
    int bnum,cnum;
    int x_intra_group_num;

    cell** BCell = (cell**)malloc(sizeof(cell*) * MaxBuckets);

    uint8_t countbit;
    int encode_bit = 3;
    int encode_table[8] = {0,1,3,5,7,11,18,29};

    float BC_b;


public:
    BucketCell(int bnum,int cnum);

    void update(unsigned int x_value, const unsigned int x_index, const uint32_t y);

    std::map<std::pair<uint32_t,uint32_t>, uint32_t> report(int x_index, const uint32_t x, float ph2);

    uint32_t decode_counter(unsigned int count);
    void clear_bucket(int x_index, const unsigned int x_value);
    cell getCell(uint32_t bucket_num, uint32_t cell_num) {
        return BCell[bucket_num][cell_num];
    };
    ~BucketCell() {
    };

};
#endif //BUCKETCELL_H
