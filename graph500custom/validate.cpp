#include "validate.h"

int validateParentArray(std::vector<int> parent, std::vector<EigenTriplet> &edgelist, const int searchkey, const unsigned int maximumvertexlabel)
{
    int out = 1;


    if (parent.at(searchkey) != searchkey)
    {
        std::cout << "Search Key is not own parent" << std::endl;
        out = 0;
        return out;
    }

/* annoyingly slow so I took it out; I have no idea why they run this for every search key
    std::cout << "Finding maximum vertex label" << std::endl;
    unsigned int maximumvertexlabel = 0;
    for (unsigned int i = 0; i < edgelist.size(); i++)
    {
        if (maximumvertexlabel < edgelist.at(i).col())
        {
            maximumvertexlabel = edgelist.at(i).col();
        }
        if (maximumvertexlabel < edgelist.at(i).row())
        {
            maximumvertexlabel = edgelist.at(i).row();
        }
    }
    */

    std::vector<int> positiveIndices;
    std::vector<int> condensedParent;
    std::vector<bool> mask;

    //std::cout << "Creating PositiveIndices, condensedParent, mask" << std::endl;
    for (unsigned int i = 0; i < parent.size(); i++)
    {
        if (parent.at(i) != -1)
        {
            positiveIndices.push_back(i);
            condensedParent.push_back(parent.at(i));
            mask.push_back(parent.at(i) != searchkey);
        }
    }

    std::vector<int> level(parent.size(), 0);

    for (unsigned int i = 0; i < positiveIndices.size(); i++)
    {
        level.at(positiveIndices.at(i)) = 1;
    }

/*
    std::cout << "parent vector" << std::endl;
    for (unsigned int i = 0; i < parent.size(); i++)
    {
        std::cout << parent.at(i) << std::endl;
    }


    for (unsigned int i = 0; i < positiveIndices.size(); i++)
    {
        std::cout << condensedParent.at(i) << " " << positiveIndices.at(i) << " " << mask.at(i) << " " << level.at(i) << std::endl;
    }
    */

    unsigned int k = 0;
    while (std::accumulate(mask.begin(),mask.end(),0) != 0)
    {
        //std::cout << "Accumulate mask vector " << std::accumulate(mask.begin(),mask.end(),0) << std::endl;
        for (unsigned int i = 0; i < mask.size(); i++)
        {
            if (mask.at(i) == 1)
            {
                level.at(positiveIndices.at(i)) += 1;
            }
            condensedParent.at(i) = parent.at(condensedParent.at(i));
            mask.at(i) = (condensedParent.at(i) != searchkey);
        }
        k += 1;
        if (k > maximumvertexlabel)
        {
            //cycle in tree
            std::cout << "cycle in graph!" << std::endl;
            out = -3;
            break;
        }
    }
    /*
    for (unsigned int i = 0; i < positiveIndices.size(); i++)
    {
        std::cout << condensedParent.at(i) << " " << positiveIndices.at(i) << " " << mask.at(i) <<  " " << level.at(i) << std::endl;
    }
*/
    std::cout << "validate returned " << out << std::endl;
    return out;
}
