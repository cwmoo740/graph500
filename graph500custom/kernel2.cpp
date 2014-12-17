#include "kernel2.h"

void bfsTopDownSerial(Eigen::SparseMatrix<int> &EdgeMatrix, const int startvertex, std::vector<int> &parent)
{


    int parentvertex = startvertex;
    std::queue<int> vertexqueue;

    parent.at(parentvertex) = parentvertex;

    //put first vertex on queue
    vertexqueue.push(parentvertex);

    while (!vertexqueue.empty())
    {
        parentvertex = vertexqueue.front();
        vertexqueue.pop();

        for (Eigen::SparseMatrix<int>::InnerIterator iter(EdgeMatrix,parentvertex); iter; ++iter)
        {
            //parent starts filled with -1
            //overwrites with parent vertex of each node
            //stays -1 if node is not connected to search origin
            if (parent.at(iter.row()) == -1)
            {
                parent.at(iter.row()) = parentvertex;
                //add children to queue
                vertexqueue.push(iter.row());
            }
        }
    }
}

void bfsTopDownOMP(Eigen::SparseMatrix<int> &EdgeMatrix, const int startvertex, std::vector<int> &parent)
{
    //bitmap has 0 if vertex not visited yet, 1 if visited
    bitmap_t bitmap;
    bm_init(&bitmap, parent.size());
    bm_reset(&bitmap);

    //two queues - take from currentQueue and add to nextQueue - swap when currentQueue.empty()
    std::deque<int> currentQueue;
    std::deque<int> nextQueue;

    currentQueue.push_back(startvertex);


    bm_set_bit(&bitmap, startvertex);
    parent.at(startvertex) = startvertex;


    while (true)
    {
        int i;
        int parentVertex;
        parentVertex = currentQueue.front();
        currentQueue.pop_front();


        //std::cout << "parentVertex = " << parentVertex << "\n";


        #pragma omp parallel for
        for (i = EdgeMatrix.outerIndexPtr()[parentVertex]; i < EdgeMatrix.outerIndexPtr()[parentVertex+1]; ++i)
        {
            if (!bm_get_bit(&bitmap, EdgeMatrix.innerIndexPtr()[i]))
            {
                bm_set_bit(&bitmap, EdgeMatrix.innerIndexPtr()[i]);
                parent.at(EdgeMatrix.innerIndexPtr()[i]) = parentVertex;

                #pragma omp critical
                {
                    nextQueue.push_back(EdgeMatrix.innerIndexPtr()[i]);
                }
            }
        }

        if (nextQueue.empty() && currentQueue.empty())
        {
            //std::cout << "BREAK" << std::endl;
            break;
        }
        if (currentQueue.empty())
        {
            currentQueue.swap(nextQueue);
        }
    }
}

void
bfsHybridOMP(Eigen::SparseMatrix<int> &EdgeMatrix, const int startvertex,
             std::vector<int> &parent)
{
    //number of vertices in frontier
    int frontierSize = 1;

    //number of edges from vertices in frontier
    int frontierEdges = EdgeMatrix.outerIndexPtr()[startvertex+1] - EdgeMatrix.outerIndexPtr()[startvertex];

    //edges not yet searched
    int edgesRemaining = EdgeMatrix.outerIndexPtr()[parent.size()];

    //holds the queue for top-down
    std::deque<int> vertexQueue;
    vertexQueue.push_back(startvertex);
    parent.at(startvertex) = startvertex;

    //a bitmap that says if a vertex has been visited yet
    bitmap_t visitedBitmap;
    bm_init(&visitedBitmap, parent.size());
    bm_set_bit(&visitedBitmap, startvertex);

    //bitmap that holds the frontier
    bitmap_t frontierBitmap;
    bm_init(&frontierBitmap, parent.size());

    //bitmap that gets filled when finding next frontier
    bitmap_t nextFrontierBitmap;
    bm_init(&nextFrontierBitmap, parent.size());

    //while vertices still left to search
    while (frontierSize > 0)
    {
        //while frontierEdges is relatively small, do top-down
        if (frontierEdges < ((edgesRemaining - frontierEdges)/14))
        {
            //top down -
            top_down_step(EdgeMatrix, parent, vertexQueue, visitedBitmap);

            //get next frontier size
            frontierSize = vertexQueue.size();

            //subtract the number of searched edges
            edgesRemaining -= frontierEdges;
        }
        else
        {
            //make sure to run bottom-up at least once with this flag
            bool downFlag = true;
            //transform top-down queue to bitmap
            fillBitmapFromQueue(vertexQueue, frontierBitmap);
            //cut off bottom up when frontier is less than this
            int down_cutoff = parent.size()/24;

            while (frontierSize > down_cutoff || downFlag)
            {
                //bottom up - checks if unvisited vertices have a parent in the frontier
                frontierSize = bottom_up_step(EdgeMatrix, parent, frontierBitmap,
                                              nextFrontierBitmap, visitedBitmap);
                downFlag = false;
            }
            fillQueueFromBitmap(vertexQueue, frontierBitmap, parent.size());
            frontierSize = vertexQueue.size();
        }

        //sum up number of edges in the frontier
        frontierEdges = 0;
        #pragma omp parallel for reduction(+ : frontierEdges)
        for (unsigned int i = 0; i < vertexQueue.size(); i++)
        {
            int vertex = vertexQueue.at(i);
            frontierEdges += EdgeMatrix.outerIndexPtr()[vertex+1] - EdgeMatrix.outerIndexPtr()[vertex];
        }


    }





}





static int
bottom_up_step(Eigen::SparseMatrix<int> &EdgeMatrix, std::vector<int> &parent,
               bitmap_t &frontierBitmap, bitmap_t &nextFrontierBitmap, bitmap_t &visitedBitmap)
{
    int frontierSize = 0;
    #pragma omp parallel for reduction(+ : frontierSize)
    for (unsigned int i = 0; i < parent.size(); i++)
    {
        if (!bm_get_bit(&visitedBitmap,i))
        {
            for (int k = EdgeMatrix.outerIndexPtr()[i]; k < EdgeMatrix.outerIndexPtr()[i+1]; k++)
            {
                int possibleParent = EdgeMatrix.innerIndexPtr()[k];
                if (bm_get_bit(&frontierBitmap, possibleParent))
                {
                    parent.at(i) = possibleParent;
                    bm_set_bit_atomic(&nextFrontierBitmap, i);
                    bm_set_bit_atomic(&visitedBitmap, i);
                    frontierSize++;
                    break;
                }

            }
        }
    }

    bm_swap(&frontierBitmap, &nextFrontierBitmap);
    bm_reset(&nextFrontierBitmap);

    return frontierSize;
}

static void
top_down_step(Eigen::SparseMatrix<int> &EdgeMatrix, std::vector<int> &parent,
              std::deque<int> &vertexQueue, bitmap_t &visitedBitmap)
{
    std::deque<int> nextQueue;

    while (!vertexQueue.empty())
    {
        int parentVertex = vertexQueue.front();
        vertexQueue.pop_front();

        #pragma omp parallel for
        for (int i = EdgeMatrix.outerIndexPtr()[parentVertex]; i < EdgeMatrix.outerIndexPtr()[parentVertex+1]; ++i)
        {
            if (!bm_get_bit(&visitedBitmap, EdgeMatrix.innerIndexPtr()[i]))
            {
                bm_set_bit_atomic(&visitedBitmap, EdgeMatrix.innerIndexPtr()[i]);
                parent.at(EdgeMatrix.innerIndexPtr()[i]) = parentVertex;



                #pragma omp critical
                {
                    nextQueue.push_back(EdgeMatrix.innerIndexPtr()[i]);
                }

            }
        }
    }
    vertexQueue.swap(nextQueue);
}


static void
fillBitmapFromQueue(std::deque<int> &inputQueue, bitmap_t &outputBitmap)
{
    #pragma omp parallel for
    for (unsigned int i = 0; i < inputQueue.size(); i++)
    {
        bm_set_bit_atomic(&outputBitmap, inputQueue.at(i));
    }
}

static void
fillQueueFromBitmap(std::deque<int> &outputQueue, bitmap_t &inputBitmap, const int numberVertices)
{
    outputQueue.clear();
    #pragma omp parallel for
    for (int i = 0; i < numberVertices; i++)
    {
        if (bm_get_bit(&inputBitmap, i))
        {
            #pragma omp critical
            {
                outputQueue.push_back(i);
            }
        }
    }
}




