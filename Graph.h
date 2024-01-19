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
vector<pair<int, int>> colorThreads(Graph& graph, int no_threads);
void colorInRange(Graph& graph, int start, int finish);
void colorThreadsThreads(Graph& graph, int no_threads);
void colorInRangeThreads(Graph& graph, int start, int finish, int max_degree);

// MPI
void colorMPIMaster(Graph& graph);
void colorMPISlave(Graph& graph);


#endif //PDP_PROJECT_TEAM_GRAPH_H
