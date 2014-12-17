#ifndef KERNEL1_H_INCLUDED
#define KERNEL1_H_INCLUDED

#include <Eigen/Sparse>
#include <iostream>

typedef Eigen::Triplet<int> EigenTriplet;
void sparseMatrixFromEdgelist(std::vector<EigenTriplet> &tripletList, Eigen::SparseMatrix<int> &outputmatrix, unsigned int &maximumvertexlabel);






#endif // KERNEL1_H_INCLUDED
