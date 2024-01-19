#ifndef PDP_PROJECT_TEAM_GRAPH_H
#define PDP_PROJECT_TEAM_GRAPH_H

#include <vector>
#include <mutex>

using namespace std;

class Graph {
public:
    struct Vertex {
        int color;
        int degree;
        mutex mtx;
    };

    int no_vertices;
    std::vector<Vertex> vertices;
    vector<vector<int>> adj;

public:
    explicit Graph(int no_vertices);
    void addEdge(int vertex_1, int vertex_2);
};

// Threads
void colorThreads(Graph& graph, int no_threads);
void colorPartitionThreads(Graph& graph, int start, int finish, int max_degree);

// MPI
void sendVector(const vector<int>& vector, int dest);
vector<int> receiveVector(int src);
void colorInternalsMPI(Graph& graph, int start, int finish, int max_degree);


#endif //PDP_PROJECT_TEAM_GRAPH_H
