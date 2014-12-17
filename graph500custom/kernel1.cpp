#include "kernel1.h"
void sparseMatrixFromEdgelist(std::vector<EigenTriplet> &tripletList, Eigen::SparseMatrix<int> &outputmatrix, unsigned int &maximumvertexlabel)
{
    //find the maximum vertex label and delete self-edges
    std::cout << "Find maximum vertex label and delete self edges" << std::endl;

    for (unsigned int i = 0; i < tripletList.size(); i++)
    {
        if (maximumvertexlabel < tripletList.at(i).col())
        {
            maximumvertexlabel = tripletList.at(i).col();
        }
        if (maximumvertexlabel < tripletList.at(i).row())
        {
            maximumvertexlabel = tripletList.at(i).row();
        }

        if (tripletList.at(i).col() == tripletList.at(i).row())
        {
            tripletList.erase(tripletList.begin()+i);
            i--;
        }
    }

    //print tripletlist for debug
/*
        for (int i = 0; i < tripletList.size(); i++)
        {
            std::cout << tripletList.at(i).row() << " " << tripletList.at(i).col() << std::endl;
        }
*/

    //fill outputmatrix with tripletlist
    outputmatrix.resize((maximumvertexlabel+1),(maximumvertexlabel+1));
    outputmatrix.setFromTriplets(tripletList.begin(),tripletList.end());

    //squeeze out zeros
    outputmatrix.makeCompressed();


    //debug print
    //std::cout << outputmatrix << std::endl;



    return;
}
