# CASA
## Introduction

Finding heavy quadratic elements has been an emerging topic in data engineering and data mining in recent years. 
We presents CASA, a framework for finding heavy quadratic elements to address the above challenges, which consists of three carefully designed data structures: the cluster swing filter, the index table, and the active bucket. 
To achieve both time and space efficiency, we make not only technical improvements but also theoretical contributions. 
Specifically, we introduce the bi-directional update scheme to the filter, which makes a large proportion of small flows cancel each other out, so that they do not cause much computational overhead. 
Then, we leverage the idea of the two-mode active counter to improve the counter architecture, thereby reducing memory consumption. 
In addition, to achieve higher accuracy and recall, we newly design a multi-hash mechanism in the filter to ameliorate potential hash collisions. 
Apart from these technical contributions, we also present a set of thorough theoretical analyses, including how to estimate accurately based on the improved filtering procedure and the new counter architecture,
how to prove the error bound for both primary elements and quadratic elements, and how to allocate memory space for different data structures. 
We conduct extensive experiments on four real-world datasets to evaluate CASA. The results show that CASA surpasses prior solutions by 155%-224% in F1-score and achieves 1.22-16 times greater throughput.

## Code Structure
This folder contains source codes for the paper CASA: A Time and Space Efficient Framework for Finding Heavy Quadratic Elements.

```text
CASA/
├── header/               # All header files of the code
├── BucketCell.cpp        # Implementation code of ABucket
├── Index_table.cpp       # Implementation code of the index table
├── OSFilter.cpp          # Implementation code of CSFilter
├── MurmurHash3.cpp       # Implementation code of Hash function
└── main.cpp              # Implementation code
```

## How to run

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Datesets
Due to the large-scale of the datasets, we do not upload the raw dataset and the groundtruth dataset. 
The datasets that we use in this project is downloaded from [CAIDA](https://www.caida.org/catalog/datasets/passive) and [MAWI](https://mawi.wide.ad.jp/mawi/). The format of the groundtruth dataset is (SrcIP DstIP).
