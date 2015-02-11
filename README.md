# graph500
C++ implementation of graph500 and comparison with GT reference code

The academic community has worked extensively on refining Graph500 benchmarking code for both OpenMP and MPI
in C, but quite frankly, a lot of it is extremely hard to read. I attempted to replicate the full performance of
one of the tuned OpenMP implementations using the C++ STL to improve maintainability and code legibility.
All of the files are contained in graph500custom/.
