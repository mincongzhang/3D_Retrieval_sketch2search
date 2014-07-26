3D_Retrieval_sketch2search
==========================

Sketch to search 3D retrieval system:  [Video Demo](https://www.youtube.com/watch?v=pWMIwprKJuw/)

![Demo](https://github.com/mincongzhang/3D_Retrieval_sketch2search/raw/master/demo.jpg)

1. Use Light field to extract 2D silhouettes from 3D models in several views

2. Map 2D silhouettes to grids and compute shape histogram to describe the shape for each 3D model

3. Map sketch to grids and compute similarities in the database

4. Sort and return 6 candidates models' indexes with high similarities


2014.5.5  
New idea: use affine invariant detectors?  
难点：  
(1)kdtree数据结构转换  
(2)算法优化减少时间复杂度  
