#include <iostream>
#include <vector>

#include "Graph.h"

using namespace std;

int main() {
    // Create a graph with 4 vertices
    Graph g(4);

    // Add edges
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 2);
    g.addEdge(1, 3);

    cout << "Graph Coloring using Greedy Algorithm:\n";
    vector<pair<int, int>> result;
    result = g.colorThreads(3);
    for (int i = 0; i < 4; ++i) {
        cout << result[i].first << " "<<result[i].second << endl;
    }
    return 0;
}
