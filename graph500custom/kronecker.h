#ifndef __KRONECKER_H_INCLUDED__
#define __KRONECKER_H_INCLUDED__

#include <iostream>
#include <Eigen/Sparse>
#include <cstdlib>
#include <random>
#include <chrono>
#include <vector>

typedef Eigen::Triplet<int> EigenTriplet;
void kronecker(int scale, int edgefactor, std::vector<EigenTriplet> &edgelist);

#endif // __KRONECKER_H_INCLUDED__
