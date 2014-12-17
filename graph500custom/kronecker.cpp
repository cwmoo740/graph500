////////////////////////////////////////////////////
/*
Generate an edgelist according to Graph500 parameters
Edge list returned in Nx2 matrix of [StartVertex, EndVertex]
*/
#include "kronecker.h"

void kronecker(int scale, int edgefactor, std::vector<EigenTriplet> &edgelist)
{
    int N = 1 << scale; //number of nodes
    int M = edgefactor * N; //roughly number of edges

    //probability given by graph500 specification
    double initiator_probability[3] = {0.57, 0.19, 0.19};


    double ab = initiator_probability[0]+initiator_probability[1];
    double c_norm = initiator_probability[2]/(1.0-ab);
    double a_norm = initiator_probability[0]/(ab);

    edgelist.clear();
    edgelist.resize(M,EigenTriplet(1,1,1));

    //seed random number generator with a time value
    std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch()/std::chrono::seconds(1));
    std::uniform_real_distribution<double> distribution(0.0,1.0);

    int ii_bit;
    int jj_bit;

    std::cout << "filling edgelist" << std::endl;
    for (int ib = 1; ib <= scale; ib++)
    {
        for (int i = 0; i < M; i++)
        {
            //build up edgelist values
            ii_bit = distribution(generator) > ab;
            jj_bit = distribution(generator) > ((double)ii_bit * c_norm + (double)(!ii_bit) * a_norm);


            edgelist.at(i) = EigenTriplet(edgelist.at(i).row()+(1 << (ib-1))*ii_bit, edgelist.at(i).col()+(1 << (ib-1))*jj_bit, 1);
        }
    }

    std::cout << "creating permutation vector" << std::endl;
    //create a permutation vector to do a 1->1 mapping of edgelist vals to new val
    std::vector<int> permutationvector;

    for (int i = 0; i < N; i++)
    {
        permutationvector.push_back(i);
    }
    std::shuffle(permutationvector.begin(),permutationvector.end(),generator);
    //permutation vector holds shuffled 0->N

    std::cout << "permuting edgelist vals" << std::endl;
    for (unsigned int i = 0; i < edgelist.size(); i++)
    {
        edgelist.at(i) = EigenTriplet(permutationvector.at(edgelist.at(i).row()),permutationvector.at(edgelist.at(i).col()),1);
    }

    permutationvector.clear();

    //fill and shuffle permutation vector to permute triplet order in edgelist
    for(unsigned int i = 0; i < edgelist.size(); i++)
    {
        permutationvector.push_back(i);
    }

    std::shuffle(permutationvector.begin(),permutationvector.end(),generator);

    std::cout << "permuting edgelist order" << std::endl;
    std::vector<EigenTriplet> permutededgelist(M);
    for (unsigned int i = 0; i < edgelist.size(); i++)
    {
        permutededgelist.at(i) = EigenTriplet(edgelist.at(permutationvector.at(i)).row(),edgelist.at(permutationvector.at(i)).col(),1);
    }

    edgelist = permutededgelist;

    permutededgelist.resize(0);



/*
    //Debug code to check minimum and maximum
    unsigned int minimum = 1024;
    unsigned int maximum = 0;
    for (int i = 0; i < edgelist.size(); i++)
    {
        if (maximum < edgelist.at(i).row())
        {
            maximum = edgelist.at(i).row();
        }
        if (maximum < edgelist.at(i).col())
        {
            maximum = edgelist.at(i).col();
        }
        if (minimum > edgelist.at(i).row())
        {
            minimum = edgelist.at(i).row();
        }
        if (minimum > edgelist.at(i).col())
        {
            minimum = edgelist.at(i).col();
        }
    }



    std::cout << "edgelist maximum: " << maximum << std::endl;
    std::cout << "edgelist minimum: " << minimum << std::endl;
*/
}
