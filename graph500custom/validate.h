#include <vector>
#include <iostream>
#include <numeric>

#include <Eigen/Sparse>

typedef Eigen::Triplet<int> EigenTriplet;

int validateParentArray(std::vector<int> parent, std::vector<EigenTriplet> &edgelist, const int searchkey, const unsigned int maximum);
