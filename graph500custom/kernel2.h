#include <Eigen/Sparse>
#include <Eigen/Core>
#include <vector>
#include <queue>
#include <deque>
#include <iostream>
#include <omp.h>

#include "bitmap.h"

void bfsTopDownSerial(Eigen::SparseMatrix<int> &EdgeMatrix, int startvertex, std::vector<int> &parent);

void bfsTopDownOMP(Eigen::SparseMatrix<int> &EdgeMatrix, const int startvertex, std::vector<int> &parent);

void bfsHybridOMP (Eigen::SparseMatrix<int> &EdgeMatrix, const int startvertex,
               std::vector<int> &parent);

static int
bottom_up_step(Eigen::SparseMatrix<int> &EdgeMatrix, std::vector<int> &parent,
bitmap_t &frontierBitmap, bitmap_t &nextFrontierBitmap, bitmap_t &visitedBitmap);

static void
top_down_step(Eigen::SparseMatrix<int> &EdgeMatrix, std::vector<int> &parent,
std::deque<int> &vertexQueue, bitmap_t &visitedBitmap);

static void
fillBitmapFromQueue(std::deque<int> &inputQueue, bitmap_t &outputBitmap);

//needs to be improved - very slow right now
static void
fillQueueFromBitmap(std::deque<int> &outputQueue, bitmap_t &inputBitmap, const int numberVertices);
