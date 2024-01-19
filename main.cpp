#include <iostream>
#include <vector>
#include <mpi.h>

#include "Graph.h"

using namespace std;

void runThreads() {
    // Create a graph with 4 vertices
    Graph g{4};

    // Add edges
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 2);
    g.addEdge(1, 3);

    cout << "Graph Coloring using Greedy Algorithm (Threads):\n";
    colorThreadsThreads(g, 3);
    for (int i = 0; i < g.no_vertices; ++i) {
        cout << i << " "<< g.vertices[i].color << endl;
    }
}

void runMPI() {
    int worldSize;
    int worldRank;

    MPI_Init(nullptr, nullptr);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

    if (worldRank == 0) {
        // Create a graph with 4 vertices
        Graph g{4};

        // Add edges
        g.addEdge(0, 1);
        g.addEdge(0, 2);
        g.addEdge(1, 2);
        g.addEdge(1, 3);

        MPI_Win_allocate_shared(sizeof(Graph), sizeof(Graph), MPI_INFO_NULL, MPI_COMM_WORLD, &shared_array, &win);

        cout << "Graph Coloring using Greedy Algorithm (Threads):\n";
        colorMPIMaster(g);
        for (int i = 0; i < g.no_vertices; ++i) {
            cout << i << " "<< g.vertices[i].color << endl;
        }
    } else {

        colorMPISlave(g);
    }
}

int main() {
    runThreads();
    return 0;
}
