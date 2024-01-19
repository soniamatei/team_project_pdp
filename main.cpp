#include <iostream>
#include <vector>
#include <mpi.h>
#include <set>

#include "Graph.h"

using namespace std;

void runThreads() {
    // Create a graph with 4 vertices
    Graph g{10};

    // Add edges
//    g.addEdge(0, 1);
//    g.addEdge(0, 2);
//    g.addEdge(1, 2);
//    g.addEdge(1, 3);
    g.addEdge(0, 2);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(2, 5);
    g.addEdge(2, 6);
    g.addEdge(3, 4);
    g.addEdge(4, 5);
    g.addEdge(5, 6);
    g.addEdge(6, 7);
    g.addEdge(6, 8);
    g.addEdge(7, 8);
    g.addEdge(7, 9);

    cout << "Graph Coloring using Greedy Algorithm (Threads):\n";
    colorThreads(g, 3);
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
        Graph g{10};

        // Add edges
        g.addEdge(0, 2);
        g.addEdge(1, 2);
        g.addEdge(2, 3);
        g.addEdge(2, 5);
        g.addEdge(2, 6);
        g.addEdge(3, 4);
        g.addEdge(4, 5);
        g.addEdge(5, 6);
        g.addEdge(6, 7);
        g.addEdge(6, 8);
        g.addEdge(7, 8);
        g.addEdge(7, 9);

//        cout << worldRank << endl;
//        for (int i = 0; i < g.no_vertices; ++i) {
//            for (int j = 0; j < g.no_vertices; ++j) {
//                cout << g.adj[i][j] << " ";
//            }
//            cout << endl;
//        }
//        cout << endl;

        // send graph to slaves
        for (int process_index = 1; process_index < worldSize; ++process_index) {
            MPI_Send(&g.no_vertices, 1, MPI_INT, process_index, 0, MPI_COMM_WORLD);
            for (int v = 0; v < g.no_vertices; ++v) {
                sendVector(g.adj[v], process_index);
            }
        }

        // partition graph & assign tasks
        for (int process_index = 1; process_index < worldSize; process_index++) {
            int start = (process_index * g.no_vertices) / worldSize;
            int finish = ((process_index + 1) * g.no_vertices) / worldSize;
            MPI_Send(&start, 1, MPI_INT, process_index, 0, MPI_COMM_WORLD);
            MPI_Send(&finish, 1, MPI_INT, process_index, 0, MPI_COMM_WORLD);
        }

        // find maximum degree
        int max_degree = g.vertices[0].degree;
        for (int i = 1; i < g.no_vertices; ++i) {
            max_degree = max(max_degree, g.vertices[i].degree);
        }

        // color internals
        cout << "Graph Coloring using Greedy Algorithm (Threads):\n";
        int start = 0;
        int finish = g.no_vertices / worldSize;
        colorInternalsMPI(g, start, finish, max_degree);

        // get colors from slaves
        for (int process_index = 1; process_index < worldSize; process_index++) {
            vector<int> colors = receiveVector(process_index);
            for (int v = 0; v < g.no_vertices; ++v) {
                if (colors[v] != -1) {
                    g.vertices[v].color = colors[v];
                }
            }
        }

        // color boundary vertices
        for (int v = 0; v < g.no_vertices; ++v) {
            if (g.vertices[v].color != -1) {
                continue;
            }

            set<int> adj_colors;

            // get colors of its neighbors
            for (int vertex = 0; vertex < g.no_vertices; ++vertex) {
                if (g.adj[v][vertex] == 1) {
                    if (v != vertex) {
                        adj_colors.insert(g.vertices[vertex].color);
                    }
                }
            }

            // assign color
            for (int color = 0; color <= max_degree; ++color) {
                if (adj_colors.find(color) == adj_colors.end()) {
                    g.vertices[v].color = color;
                    break;
                }
            }
        }

        // print colors
        for (int i = 0; i < g.no_vertices; ++i) {
            cout << i << " "<< g.vertices[i].color << endl;
        }
    } else {
        int no_vertices;
        MPI_Recv(&no_vertices, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        Graph g{no_vertices};

        for (int v = 0; v < g.no_vertices; ++v) {
            vector<int> adj = receiveVector(0);
            for (int n = 0; n < g.no_vertices; ++n) {
                if (adj[n] == 1) {
                    g.addEdge(v, n);
                }
            }
        }

        int start, finish;
        MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&finish, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


        // find maximum degree
        int max_degree = g.vertices[0].degree;
        for (int i = 1; i < g.no_vertices; ++i) {
            max_degree = max(max_degree, g.vertices[i].degree);
        }

//        cout << worldRank << endl;
//        for (int i = 0; i < g.no_vertices; ++i) {
//            for (int j = 0; j < g.no_vertices; ++j) {
//                cout << g.adj[i][j] << " ";
//            }
//            cout << endl;
//        }
//        cout << endl;

        // color internal vertices
        colorInternalsMPI(g, start, finish, max_degree);

        // send colors
        vector<int> colors;
        for (int i = 0; i < g.no_vertices; ++i) {
            colors.push_back(g.vertices[i].color);
        }
        sendVector(colors, 0);
    }

    MPI_Finalize();
}

int main() {
    runMPI();
    return 0;
}
