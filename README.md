# CASA
## Introduction

Finding heavy quadratic elements has been an emerging topic in data engineering and data mining in recent years. 
We presents CASA, a framework for finding heavy quadratic elements to address the above challenges, consisting of three carefully designed data structures. 
To achieve both time and space efficiency, we make not only technical improvements but also theoretical contributions. 
Specifically, we show how to leverage advanced techniques to improve data structures, including the bi-directional update scheme for the filter, which makes a large proportion of small quadratic elements cancel each other out, so that they do not cause much computational overhead.
Then, we adopt the idea of the two-mode active counter to improve the counter architecture, thereby reducing memory consumption.
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
