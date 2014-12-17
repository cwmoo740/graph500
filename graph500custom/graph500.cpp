#include "utils.h"
#include "kronecker.h"
#include "kernel1.h"
#include "kernel2.h"
#include "validate.h"
#include "createSearchKey.h"

#include <iostream>
#include <fstream>

#include <Eigen/Sparse>
#include <Eigen/Core>

#include <cstdlib>
#include <random>
#include <chrono>
#include <ctime>
#include <vector>

#include <omp.h>

//valgrind = profiling tool, linux only
#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32))
#include <valgrind/callgrind.h>
#define VALGRIND_INCLUDED
#endif

//leave in when testing performance, take out for cache data
#undef VALGRIND_INCLUDED


typedef Eigen::Triplet<int> EigenTriplet;

int main(int argc, char* argv[])
{
    //set scale, edges per node, and maximum number of test search keys
    int scale = 0;
    int edgefactor = 0;
    unsigned int NBFS = 64;
    unsigned int maximumvertex = 0;
    int validateFlag = INT_MIN;

    if (argc == 4)
    {
        scale = std::atoi(argv[1]);
        if (scale < 4 || scale > 26)
        {
            std::cout << "Scale must be between 4 and 26" << std::endl;
            return -1;
        }
        edgefactor = std::atoi(argv[2]);
        if (edgefactor != 2 && edgefactor != 16)
        {
            std::cout << "Edgefactor must be 2 or 16" << std::endl;
            return -1;
        }
        if (std::atoi(argv[3]) != 0)
        {
            validateFlag = 1;
        }
    }
    else if (argc != 1)
    {
        std::cout<< "please enter scale, edgefactor, validate" << std::endl;
    }
    else
    {

        std::cout << "Enter problem scale" << std::endl;
        std::cout << "Scale: ";

//get user input for scale and edgefactor
        while(true)
        {
            getint(scale);
            if (scale > 2 && scale < 27)
            {
                break;
            }
            std::cout << "Number out of bounds" << std::endl;
            std::cout << std::endl;
        }

        std::cout << "Enter edge factor" << std::endl;
        std::cout << "Recommended: 2 (debug), 16" << std::endl;
        std::cout << "Edge Factor: ";

        while(true)
        {
            getint(edgefactor);
            if (edgefactor == 16 || edgefactor == 2)
            {
                break;
            }
            std::cout << "Just put in 16" << std::endl;
        }

        std::cout << "Run validate function? 1=yes, 0=no" << std::endl;

        while(true)
        {
            getint(validateFlag);
            if (validateFlag == 0)
            {
                std::cout << "Not running validate function!" << std::endl;
                break;
            }
            else if (validateFlag == 1)
            {
                break;
            }
            std::cout << "Enter 1 or 0" << std::endl;
        }
    }



//vector of triplets (row, col, val) to pass to kronecker generator
    std::vector<EigenTriplet> edgelist(0);

    std::cout << "calling kronecker generator" << std::endl;

    kronecker(scale, edgefactor, edgelist);
    std::cout << "done with kronecker" << std::endl;


//add transposed edgelist values to end of edgelist vector
//plays nicer with Eigen's setFromTriplets function
//otherwise Eigen can't make symmetric matrices well
    edgelist.reserve(2*edgelist.size());
    unsigned int edgeListOriginalSize = edgelist.size();
    for (unsigned int i = 0; i < edgeListOriginalSize; i++)
    {
        edgelist.push_back(EigenTriplet(edgelist.at(i).col(),edgelist.at(i).row(),1));
    }



//create container for adjacency matrix
    Eigen::SparseMatrix<int> EdgeMatrix(0,0);

    std::cout << "transform edgelist into sparse adjacency matrix" << std::endl;


//call kernel 1 to fill EdgeMatrix with data from edgelist
    sparseMatrixFromEdgelist(edgelist, EdgeMatrix, maximumvertex);



    std::vector<int> search_key;
    createSearchKey(EdgeMatrix.rows(), NBFS, search_key, EdgeMatrix);

    if (search_key.size() < NBFS)
    {
        NBFS = search_key.size();
    }



//create parent array and fill with negative 1
    std::vector<int> parent(EdgeMatrix.cols(),-1);

//calculate number of edges
//edgelist = original + transposed, so just sum along one column
    std::vector<int> edgehistogram(EdgeMatrix.cols(),0);
    for (unsigned int i = 0; i < edgelist.size(); i++)
    {
        edgehistogram.at(edgelist.at(i).col()) += 1;
    }


    std::cout << "calling BFS algorithm" << std::endl;

//fill parent array by doing BFS on edgematrix starting at search_key(k)
//calculate number of edges in graph connected to search_key
//occasionally have a small graph subsection with few edges that could falsely increase performance

    std::vector<double> numberedges(NBFS,0);
    std::vector<double> searchtime(NBFS,0);
    std::vector<double> TEPS(NBFS,0);

    //std::cout << EdgeMatrix << std::endl;

#ifdef VALGRIND_INCLUDED
    CALLGRIND_START_INSTRUMENTATION;
#endif
    for (unsigned int k = 0; k < NBFS; k++)
    {

        std::chrono::time_point<std::chrono::high_resolution_clock> timerstart, timerend;

        //search and validate results
        std::cout << "starting BFS on search key " << k << ": " << search_key.at(k) << std::endl;
        timerstart = std::chrono::high_resolution_clock::now();




        //bfsTopDownSerial(EdgeMatrix, search_key.at(k), parent, maximumvertex);
        //bfsTopDownOMP(EdgeMatrix, search_key.at(k), parent);
        bfsHybridOMP(EdgeMatrix, search_key.at(k), parent);

        timerend = std::chrono::high_resolution_clock::now();

        searchtime.at(k) = std::chrono::duration_cast<std::chrono::nanoseconds>(timerend-timerstart).count();


        //find number of edges from parent array

        for (unsigned int i = 0; i < parent.size(); i++)
        {
            if (parent.at(i) >= 0)
            {
                numberedges.at(k) += edgehistogram.at(i);
            }
        }
        numberedges.at(k) /= 2.0;
        TEPS.at(k) = numberedges.at(k)/(searchtime.at(k));

        if (validateFlag == 1)
        {
            std::cout << "Checking Search Key " << k << ": " << search_key.at(k) << std::endl;
            int validatecode = validateParentArray(parent,edgelist,search_key.at(k), maximumvertex);
            if (validatecode != 1)
            {
                std::cout << "validation failed on search key " << search_key.at(k) << std::endl;
                return validatecode;
            }

        }

        //clear parent array for next search
        //if you want to save it write it to file first
        std::fill(parent.begin(),parent.end(),-1);

    }

#ifdef VALGRIND_INCLUDED
    CALLGRIND_STOP_INSTRUMENTATION;
    CALLGRIND_DUMP_STATS;
#endif




//std::cout << EdgeMatrix << std::endl;

    std::cout << "time, number edges, TEPS" << std::endl;
    for (unsigned int i = 0; i < numberedges.size(); i++)
    {
        std::cout << searchtime.at(i) << " " << numberedges.at(i) << " " << TEPS.at(i) << "\n";
    }
    std::cout << std::endl;

    std::ofstream datafile;
    datafile.open("GTEPS.txt", std::ofstream::app);
#define average(vector) (std::accumulate(vector.begin(),vector.end(),0.0)/(double)vector.size())
    datafile << scale << " " << average(searchtime) << " " << average(numberedges) << " " << average(TEPS) << "\n";
    datafile.close();



    return 0;
}

