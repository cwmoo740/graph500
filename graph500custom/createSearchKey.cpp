#include "createSearchKey.h"

void createSearchKey(unsigned int numberRows, unsigned int NBFS, std::vector<int> &search_key, const Eigen::SparseMatrix<int> &EdgeMatrix)
{

//columndegree contains number of nonzeros per column
//for removing searchkey values that are not connected to main graph
    std::vector<int> columndegree;
    columndegree.reserve(numberRows);

    for (unsigned int i = 0; i < numberRows; i++)
    {
        columndegree.push_back(EdgeMatrix.outerIndexPtr()[i+1]-EdgeMatrix.outerIndexPtr()[i]);
    }

//generate search key values based on time seed
    std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch()/std::chrono::seconds(1));

    std::cout << "creating search key vector" << std::endl;
    for (unsigned int i = 0; i < numberRows; i++)
    {
        search_key.push_back(i);
    }
//shuffle search key values
    std::shuffle(search_key.begin(),search_key.end(),generator);

//take first 64 or entire search key, whichever is smaller
    if (search_key.size() > NBFS)
    {
        for (unsigned int i = 0; i < NBFS+20; i++)
        {
            //remove search key values that aren't connected to main graph
            if (columndegree.at(search_key.at(i)) == 0)
            {
                search_key.erase(search_key.begin()+i);
                i--;
            }
        }
        search_key.erase(search_key.begin()+NBFS, search_key.end());
    }

    std::cout << "Removing search keys with no edges" << std::endl;
    for (unsigned int i = 0; i < search_key.size(); i++)
    {
        //remove search key values that aren't connected to main graph
        if (columndegree.at(search_key.at(i)) == 0)
        {
            search_key.erase(search_key.begin()+i);
            i--;
        }
    }

    search_key.shrink_to_fit();
}
