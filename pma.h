#ifndef DISTRIBUTEDSQUARECOUNTING_PMA_H
#define DISTRIBUTEDSQUARECOUNTING_PMA_H


#include <climits>
#include <cmath>
#include <iostream>
#include <vector>
#include <functional>
#include <cstring>
#include <cassert>
#include <chrono>
#include "types.h"


class PMA{

    public:
        typedef std::function<void(uint64_t)> range_func;
        static constexpr uint64_t INT_NULL = UINT64_MAX;
        static constexpr VertexID INVALID_IDX = UINT32_MAX;

        ui vertices_count;

        std::vector<uint64_t> data;
        VertexID length;
        double leaf_max = 0.75;
        VertexID num_elements;
        bool resize_allowed;
        VertexID max_index;
        std::vector<uint64_t> temp;

        PMA(VertexID size);
        PMA(VertexID size, double leaf_max);
        PMA(VertexID size, double leaf_max, bool resize_allowed);
        
        PMA() = default;

        std::vector<uint64_t> get_min_range(VertexID left, VertexID right);
        void swap_data(std::vector<uint64_t>& tmp);
        void range(uint64_t left, uint64_t right, range_func& op);
        
        VertexID logN();
        VertexID loglogN();
        VertexID leaf_index(VertexID index);
        VertexID next_leaf(VertexID index);
        VertexID leaf_number(VertexID index);
        VertexID leaf_position(VertexID leaf_num);
        VertexID num_leaves();
        VertexID depth();
        VertexID count_nonempty(VertexID index, VertexID len);

        VertexID size();
        void insert(uint64_t i);
        void delete_edge(uint64_t i);
        bool query(uint64_t i); 
        VertexID search(uint64_t i);
        void redistribute(VertexID index, VertexID len, VertexID density_count);
        void resize();
        void slide_left(VertexID index);
        void slide_right(VertexID index);
        void print_pma(std::ostream& stream = std::cout);
        
};

PMA::PMA(VertexID size_param) {
    data.resize(size_param, INT_NULL);
    length = size_param;
    num_elements = 0;
    resize_allowed = true;
    max_index = INVALID_IDX;
    vertices_count = 0;
}

PMA::PMA(VertexID size, double leaf_max_param) {
    data.resize(size, INT_NULL);
    length = size;
    num_elements = 0;
    leaf_max = leaf_max_param;
    resize_allowed = true;
    max_index = INVALID_IDX;
    vertices_count = 0;
}

PMA::PMA(VertexID size, double leaf_max_param, bool resize_allowed_param) {
    data.resize(size, INT_NULL);
    length = size;
    num_elements = 0;
    leaf_max = leaf_max_param;
    resize_allowed = resize_allowed_param;
    max_index = INVALID_IDX;
    vertices_count = 0;
}


VertexID MSSB(VertexID x) {
    VertexID i = 0;
    while (x != 0) {
        x = x >> 1;
        ++i;
    }
    return i - 1;
}

VertexID next_power_of_2(VertexID x) {
    return 1 << (MSSB(x) + 1);
}

VertexID PMA::size() { return length; } // Returning the size of the pma
VertexID PMA::logN() { return next_power_of_2((VertexID) log2(length)); }
VertexID PMA::loglogN() { return (VertexID) log2(logN()); }
VertexID PMA::leaf_index(VertexID index) { return (index & ~(logN() - 1)); }
VertexID PMA::next_leaf(VertexID index) { return leaf_index(index + logN()); }
VertexID PMA::leaf_number(VertexID index) { return leaf_index(index) >> loglogN(); }
VertexID PMA::leaf_position(VertexID leaf_num) { return leaf_num << loglogN(); }
VertexID PMA::num_leaves() { return length / logN(); }
VertexID PMA::depth() { return MSSB(num_leaves()); }


VertexID PMA::count_nonempty(VertexID index, VertexID len)  { 
    VertexID full = 0;
    for (VertexID i = index; i < index + len;) {
        if (data[i] != INT_NULL) {
            ++full;
            i += 1;
        }
        else {
            i = next_leaf(i);
        }
    }
    return full;
}

bool PMA::query(uint64_t key) {
    VertexID idx = search(key);
    if (idx == INVALID_IDX) {
        return false;
    }
    return data[idx] == key;
}


VertexID PMA::search(uint64_t key) {
    VertexID low = 0;
    VertexID high = leaf_index(length - 1);
    uint64_t min_key = data[low];
    if (key == min_key) {
        return low;
    }
    else if (key < min_key) {
        return INVALID_IDX;
    }
    
    VertexID argmax = max_index;
    uint64_t max_key = data[argmax];
    high = leaf_index(argmax);
    
    if (key >= max_key) {
        return argmax;
    }

    while (low < high) {
        VertexID mid = (low + high) / 2;
        VertexID mid_leaf = leaf_index(mid);
        if (data[mid_leaf] == key) {
            return mid_leaf;
        }
        else if (data[mid_leaf] > key) {
            high = mid_leaf - logN();
        }
        else {
            if (mid_leaf == low) {
                break; // avoid infinite loop, can only occur if high = low + 1 leaf
            }
            low = mid_leaf;
        }
    }

    // if low != high, then key could be either in low or high. Check first element of high to find out which
    if (data[high] == key) {
        return high;
    }
    if (data[high] < key) {
        low = high;
    }
    // search leaf
    VertexID leaf = leaf_index(low);
    for (VertexID i = 0; i < logN(); i += 1) {
        if (data[leaf + i] == INT_NULL) {
            return leaf + i - 1;
        }
        if (data[leaf + i] == key) {
            return leaf + i;
        }
        if (i != (logN() - 1) && data[leaf + i] < key && data[leaf + i + 1] > key) {
            return leaf + i;
        }
    }

    std::cerr << "Leaf of PMA too dense (is the PMA properly left packed?)" << std::endl;
    assert(false); // in theory this should never happen, since that would mean leaf is completely full
    return -1;
}


//TODO: needs to be updated
void PMA::slide_left(VertexID index) {
    uint64_t right;
    uint64_t left = data[index];
    for (VertexID i = index; i < leaf_position(leaf_number(index) + 1); i++) {
        right = data[i + 1];
        data[i] = right;
        if (i == max_index) {
            max_index = i - 1;
        }
        if (right == INT_NULL) {
            break;
        }
    }
}


void PMA::slide_right(VertexID index) {
    uint64_t right;
    uint64_t left = data[index];
    for (VertexID i = index; i < leaf_position(leaf_number(index) + 1); i++) {
        right = data[i + 1];
        data[i + 1] = left;
        left = right;
        if (i == max_index) {
            max_index = i + 1;
        }
        if (left == INT_NULL) {
            break;
        }
    }
}

void PMA::insert(uint64_t key) {
    
    auto start = std::chrono::high_resolution_clock::now();
    VertexID index = search(key);
    auto search_end = std::chrono::high_resolution_clock::now();
    if (index != INVALID_IDX && data[index] == key) {
        return;
    }
    
    num_elements += 1;
    if (index == max_index) {
        max_index = index + 1;
    }
    index = index + 1;

    if (data[index] != INT_NULL) {
        slide_right(index);
    }

    data[index] = key;


    VertexID len = logN();
    VertexID node_index = leaf_index(index);
    VertexID density_count = count_nonempty(node_index, len);

    int level = 0;
    while (density_count > (VertexID) ((leaf_max - 0.01 * level) * len) && (len < length)) {
        len *= 2;
        VertexID new_node_index = (node_index / len) * len;

        if (new_node_index < node_index) {
            density_count += count_nonempty(new_node_index, len / 2);
        } else {
            density_count += count_nonempty(new_node_index + len / 2, len / 2);
        }
        node_index = new_node_index;
        level += 1;
    }

    if (len == length && density_count > (VertexID) (leaf_max * len)) {
        // need to double PMA, will disallow this
        if (resize_allowed) {
            resize();
        }
    }
    else if (len > logN()) {
        redistribute(node_index, len, density_count);
    }

    auto other_end = std::chrono::high_resolution_clock::now();

}

//TODO: needs to be updated
void PMA::delete_edge(uint64_t key) {
    
    VertexID index = search(key);
    
    if (index != INVALID_IDX && data[index] == key) {
        slide_left(index);
    } else {
        return;
    }
    
    num_elements -= 1;

    if (index == max_index) {
        max_index = index - 1;
    }
    
    auto other_end = std::chrono::high_resolution_clock::now();

}


void PMA::redistribute(VertexID index, VertexID len, VertexID density_count) {
    temp.reserve(len);
    temp.clear();
    for (VertexID i = index; i < len + index;) {
        if (data[i] != INT_NULL) {
            temp.push_back(data[i]);
            data[i] = INT_NULL;
            i += 1;
        }
        else {
            i = next_leaf(i);
        }
    }
    
    VertexID nl = len / logN();
    VertexID elems_per_leaf = density_count / nl;
    VertexID x = 0;
    for (VertexID leaf = 0; leaf < nl; ++leaf) {
        VertexID num_elems_to_copy = elems_per_leaf + (leaf < density_count % nl);
        memcpy(&data[index + leaf * logN()], &temp[x], num_elems_to_copy * sizeof(uint64_t));
        x += num_elems_to_copy;
        
        if (leaf == nl - 1 && (max_index >= index && max_index < index + len || max_index == INVALID_IDX)) {
            if (num_elems_to_copy == 0) {
                std::cout << "???" << std::endl;
                exit(-1);
            }
            max_index = index + leaf * logN() + (num_elems_to_copy - 1);
        }
    }
}



void PMA::resize() {
    length *= 2;
    data.resize(length, INT_NULL);
    redistribute(0, length, num_elements);
}

void PMA::print_pma(std::ostream& stream) {
    stream << "[";
    for (auto element : data) {
        if (element == INT_NULL) {
            stream << "null" << " ";
        }
        else {
            stream << element << " ";
        }
    }
    stream << "]" << std::endl;
}

void PMA::range(uint64_t left, uint64_t right, range_func& op) {
    
    VertexID left_index = search(left);
    VertexID right_index = search(right);
    
    if (right_index == INVALID_IDX) {
        return;
    }

    if (left_index == INVALID_IDX) {
        left_index = 0;
    }

    if (data[left_index] < left || data[left_index] == INT_NULL) {
        if (left_index == right_index) {
            return;
        } else {
            left_index += 1;
        }
    }

    right_index += 1;
    for (VertexID i = left_index; i < right_index; ) {
        if (data[i] != INT_NULL) {
            op(data[i]);
            i += 1;
        }
        else {
            i = next_leaf(i);
        }
    }
}


#endif