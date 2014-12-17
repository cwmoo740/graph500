#include <Eigen/Sparse>

#include <cstdlib>
#include <random>
#include <chrono>
#include <ctime>
#include <vector>
#include <iostream>
#include <algorithm>

void createSearchKey(unsigned int numberRows, unsigned int NBFS, std::vector<int> &search_key, const Eigen::SparseMatrix<int> &EdgeMatrix);
