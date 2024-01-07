#ifndef PDP_PROJECT_TEAM_GRAPH_H
#define PDP_PROJECT_TEAM_GRAPH_H

#include <vector>
#include <mutex>

using namespace std;

class Graph {
private:
    struct Color {
        int color;
        std::mutex mtx;
    };

    int no_vertices;
    vector<vector<int>> adj;
    std::vector<Color> colors;

    void colorInRange(int start, int finish);

public:
    explicit Graph(int no_vertices);
    void addEdge(int vertex_1, int vertex_2);
    vector<pair<int, int>> colorThreads(int no_threads);
};


#endif //PDP_PROJECT_TEAM_GRAPH_H
