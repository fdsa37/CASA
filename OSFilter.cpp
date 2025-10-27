//
// Created by sq111 on 25-5-29.
//

#include <numeric>
#include "header/OSFilter.h"

#include <cmath>
#include <cstring>
#include <iomanip>

#include "header/MurmurHash3.h"

uint32_t total_pos = 0;
uint32_t total_neg = 0;
uint32_t total_flows = 0;

OSFilter::OSFilter(float memory_kb){
    d = 3;
    bits = 6;
    hash_group = 1;

    max_positive = pow(2, bits - 1) - 1;
    min_negative = - pow(2, bits - 1);

    uint32_t memory_bits = static_cast<uint32_t>(std::round(memory_kb * 1024 * 8));
    uint32_t memory_bits_layer_0 = static_cast<uint32_t>(std::round(memory_bits));

    num_counters = static_cast<uint32_t>(std::round(memory_bits_layer_0 / bits));
    counters = new int8_t[num_counters]{0};

    read_counters = 64/bits;
    counters_inter_group_num = hash_group * read_counters;
    counters_group_num = static_cast<int>(num_counters)/counters_inter_group_num;
    num_counters = counters_group_num * counters_inter_group_num;

}

uint32_t OSFilter::hash_start_group(const uint32_t flow_label) {
    uint32_t hash_start = 0;
    MurmurHash3_x86_32(&flow_label,4,1,&hash_start);
    return (hash_start % counters_group_num) * counters_inter_group_num;
}

int OSFilter::hash_s(const uint32_t flow_label) {
    uint32_t hash_value = 0;
    MurmurHash3_x86_32(&flow_label, 4, 2000, &hash_value);
    return hash_value % 2 == 0;
}

void OSFilter::readcounters(const int8_t* counters, uint32_t start_index, int8_t* out) {
    std::memcpy(out, counters + start_index, read_counters);
}

bool OSFilter::check(const uint32_t packet_id, const uint32_t flow_label) {
    bool flag = false;
    uint32_t group_index = packet_id % hash_group;
    uint32_t start_index = hash_start_group(flow_label);

    int8_t select_counter[read_counters];
    readcounters(counters, start_index + group_index * read_counters, select_counter);

    int satisfied_counters = 0;

    for (int i = 0; i < d; i++) {
        uint32_t hash_value = 0;
        MurmurHash3_x86_32(&flow_label, 4, i + 1999 + group_index, &hash_value);
        uint32_t j = hash_value % read_counters;
        if (hash_s(flow_label) == 1 && select_counter[j] >= max_positive) {
            satisfied_counters++;
        }else if (hash_s(flow_label) != 1 && select_counter[j] <= min_negative) {
            satisfied_counters++;
        }
    }
    if (satisfied_counters >= (d+1)/2) {
        flag = true;
    }
    return flag;
}


void OSFilter::update(
    const uint32_t packet_id,
    const uint32_t x_flow_label,
    const uint32_t y_flow_label,
    Index_table& index_table,
    BucketCell& bucket_cell,
    uint32_t weight){

    total_flows ++;
    uint32_t index_hash; // used to calculate the counter index

    uint32_t rand_value = packet_id % d;
    uint32_t rand_group = packet_id % hash_group;
    MurmurHash3_x86_32(&x_flow_label, 4, rand_value + 1999 + rand_group, &index_hash);

    uint32_t j = hash_start_group(x_flow_label) + rand_group * read_counters + index_hash % read_counters;

    int op_ = hash_s(x_flow_label);
    int8_t current_value = get_counter(j);
    int8_t next_value;

    if (op_ == 1){
        next_value = current_value + 1;
        if (next_value <= max_positive){
            total_pos ++;
            counters[j] = next_value;
            return;
        }
    } else{
        next_value = current_value - 1;
        if (next_value >= min_negative){
            total_neg ++;
            counters[j] = next_value;
            return;
        }
    }
    if (check(packet_id, x_flow_label) == false) {
        return;
    }else {
        index_table.update(x_flow_label, y_flow_label, bucket_cell);
    }
}

double skellam_pmf(int k, double lambda, double lambda_prime) {
    double coeff = std::exp(-(lambda + lambda_prime));

    double ratio = std::pow(lambda / lambda_prime, k / 2.0);

    double bessel = std::cyl_bessel_i(std::abs(k), 2.0 * std::sqrt(lambda * lambda_prime));

    return coeff * ratio * bessel;
}

double skellam_tail_prob(double lambda, double lambda_prime, int threshold) {
    double p = 0.0;
    for (int k = threshold; k < 10000; ++k) {
        double pk = skellam_pmf(k, lambda, lambda_prime);
        p += pk;
        if (pk < 1e-15) break;
    }
    return p;
}

int OSFilter::report(const uint32_t flow_label) {
    double estimation_n = 0;
    double lambda = total_pos / num_counters;
    double lambda_prime = total_neg / num_counters;
    int op_ = hash_s(flow_label);
    double f_d = 0;
    if (op_ == 1) {
        double p_pos = skellam_tail_prob(lambda,lambda_prime, max_positive);
        for (int i = 0; i < hash_group; i++) {
            int k_pos = 0;
            for (int c = 0; c < d; c++) {
                uint32_t index_hash = 0;
                MurmurHash3_x86_32(&flow_label, 4, c + 1999 + i, &index_hash);
                uint32_t j = hash_start_group(flow_label) + i * read_counters + index_hash % read_counters;
                if (get_counter(j) >= max_positive) {
                    k_pos++;
                }
            }
            if (k_pos >= (d+1)/2) {
                f_d = k_pos * max_positive + (d-k_pos) * (lambda - lambda_prime);
            }else {
                f_d = d*((1-p_pos)*(lambda - lambda_prime) + p_pos * max_positive);
            }
            estimation_n = (num_counters/(num_counters-d))*f_d - (d/(num_counters-d))*(total_pos - total_neg);
        }
    }else {
        double p_neg = skellam_tail_prob(lambda_prime,lambda, -min_negative);
        for (int i = 0; i < hash_group; i++) {
            int k_neg = 0;
            for (int c = 0; c < d; c++) {
                uint32_t index_hash = 0;
                MurmurHash3_x86_32(&flow_label, 4, c + 1999 + i, &index_hash);
                uint32_t j = hash_start_group(flow_label) + i * read_counters + index_hash % read_counters;
                if (get_counter(j) <= min_negative) {
                    k_neg++;
                }
            }
            if (k_neg >= (d+1)/2) {
                f_d = k_neg * (-min_negative) + (d-k_neg) * (lambda_prime - lambda);
            }else {
                f_d = d*((1-p_neg)*lambda + p_neg * (-min_negative));
            }
            estimation_n = (num_counters/(num_counters-d))*f_d - (d/(num_counters-d))*(total_neg - total_pos);
        }
    }
    return estimation_n;
}