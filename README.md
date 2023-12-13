
# Globally Constrained Path Planning Problems*

## **2022 RISS Project:  Optimaility vs Efficiency in Globally Constrained Path Planning Problems** 

### Short Bio: https://riss.ri.cmu.edu/student/will-scott/

This repository contains open-source implementations of three A* search-based planning algorithms; A*, Constrained A*, and Weighted A* search.

## Background:

This work extends the repository developed by Ph. D student Dhruv Saxena [Forked Respository](https://github.com/dhruvms/amra) to test the performance of various A* algorithms to determine the fastest and most efficient algorithm for a rapid response scenario. As a member of the 2022 RISS Cohort, this research focuses on autonomous informed path-planning to serve global marginalized communities.


## Abstract*

Within the field of robotics, various algorithms have emerged to compute the optimal solution for the shortest- path problem (SPP). We are motivated to use these algorithms to address the growing issue of navigation in congested set- tlements within South Africa. These communities are densely populated with minimal critical infrastructure for municipal services such as roads or electricity. Consequently, emergencies like fires or floods cause extensive damage to these communities. To better assess these algorithms’ efficiency and optimality to solve globally constrained path-planning problems, we test three existing algorithms to simulate their behavior in urban environments. All algorithms extend the basic A* search to compute the optimal path by minimizing an objective function subject to a single global constraint.

The algorithm’s performance was measured separately, with time as a dependent or independent dimension, compared against the Constrained A* (CA*) search developed for re- planning with a global constraint [1]. We achieved optimality in the search when planning in a 2D grid world with time as an independent variable. Our findings communicate that a simplistic A* search planning in (x,y) would serve as the most efficient algorithm to realize urban planning with a constraint, setting up a discussion for efficiency over optimality in path- planning problems with a global constraint.

[Full Paper (pg. 252)](https://riss.ri.cmu.edu/wp-content/uploads/2023/08/CMU-RISS_Working_Papers_Journal-2022-PAPERS.pdf)

## Poster:

![2022-RISS-poster-Optimality_vs_Efficiency_in_Globally_Constrained_Path_Planning_Problems-SCOTT-William-LIKHACHEV](https://github.com/PenguinMesssiah/amra/assets/61028739/ab0e182d-8bec-48c6-8eb1-bbd2f1701926)

### Acknowledgements:

This work was supported by the Carnegie Mellon University Robotics Institute Summer Scholars (RISS) Program. A special thanks to Rachel Burcin and Dr. John Dolan for providing tremendous academic and mental health support to the cohort this summer, expanding the robotics community to individuals of all backgrounds. I want to thank my mentors Dr. Maxim Likhachev and Ph.D. student Dhruv Saxena from the Search-Based Planning Lab for supporting this project. Thank you everyone for making this summer a worthwhile academic and social experience.


### Dependencies
The repository has the following dependencies:
1. [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
2. [Boost C++](https://www.boost.org/)
3. [OMPL](https://ompl.kavrakilab.org/) (for the `RRT*` baseline experiment)
  + The OMPL dependency can be ignored if you do not plan to run the [`ompl_run`](test/ompl_run.cpp) example.

A lot of utility functions have been borrowed from the [SMPL](https://github.com/aurone/smpl) repository by @aurone.

### Building
1. Clone the repository.
    ```
    git clone https://github.com/PenguinMesssiah/amra
    ```
2. Create build directory and compile.
    ```
    cd amra && mkdir build && cd build && cmake ../ && make -j`nproc`
    ```
    
**Project Depricated** : [Will Scott](mailto:wscott2@andrew.cmu.edu)
