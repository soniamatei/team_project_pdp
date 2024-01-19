#include "ThreadPool.h"
#include "Graph.h"
#include <iostream>
#include <mpi.h>
#include <set>


Graph::Graph(int no_vertices) : no_vertices(no_vertices),
                                vertices(no_vertices),
                                adj(no_vertices, std::vector<int>(no_vertices, 0))
{
    for (Vertex& vertex : vertices) {
        vertex.color = -1;
        vertex.degree = 0;
    }
}

void Graph::addEdge(int vertex_1, int vertex_2) {
    adj[vertex_1][vertex_2] = 1;
    adj[vertex_2][vertex_1] = 1;
    vertices[vertex_1].degree++;
    vertices[vertex_2].degree++;
}


/* ALGORITHMS */
vector<pair<int, int>> colorThreads(Graph& graph, int no_threads) {
    ThreadPool thread_pool{no_threads};

    for (int thread_index = 0; thread_index < no_threads; thread_index++) {
        int start = thread_index * graph.no_vertices / no_threads;
        int finish = (thread_index + 1) * graph.no_vertices / no_threads;
        cout << start << " " << finish << endl;
        thread_pool.enqueue([&graph, start, finish]() {
            colorInRange(graph, start, finish);
        });
    }

    thread_pool.close();

    vector<pair<int, int>> node_color;

    // create a vector with the nodes and corresponding colors
    for (int i = 0; i < graph.no_vertices; ++i) {
        cout << graph.vertices[i].color << endl;
        node_color.emplace_back(i, graph.vertices[i].color);
    }

    return node_color;
}

void colorInRange(Graph& graph, int start, int finish) {

    // iterate through vertices in the given range
    for (int vertex = start; vertex < finish; vertex++) {
        if (graph.vertices[vertex].color == -1) {

            // find first available color
            // fictional color represented as a number in range [0, n)
            for (int color_index = 0; color_index < graph.no_vertices; color_index++) {

                bool available = true;
                for (int i = 0; i < graph.no_vertices; i++) {

                    // if there is an adjacent node with the given color, go to next color
                    if (graph.adj[vertex][i] && graph.vertices[i].color == color_index) {
                        available = false;
                        break;
                    }
                }

                if (available) {

                    // verify one last time if the color hadn't been written since the check
                    unique_lock<mutex> lock{graph.vertices[vertex].mtx};
                    cout << "a";
                    if (graph.vertices[vertex].color == -1) {
                        graph.vertices[vertex].color = color_index;
                        cout << "b" << graph.vertices[vertex].color;
                        lock.unlock();
                        break;
                    }
                    lock.unlock();
                }
            }
        }
    }
}

void colorThreadsThreads(Graph& graph, int no_threads) {
    if (graph.no_vertices < no_threads) {
        cout << "Number of threads exceeds the number of vertices!";
        return;
    }

    ThreadPool thread_pool{no_threads};

    // find maximum degree
    int max_degree = graph.vertices[0].degree;
    for (int i = 1; i < graph.no_vertices; ++i) {
        max_degree = max(max_degree, graph.vertices[i].degree);
    }

    // partition graph & assign tasks
    for (int thread_index = 0; thread_index < no_threads; thread_index++) {
        int start = (thread_index * graph.no_vertices) / no_threads;
        int finish = ((thread_index + 1) * graph.no_vertices) / no_threads;
        cout << thread_index << " thread is coloring vertices " << start << " - " << finish - 1 << endl;
        thread_pool.enqueue([&graph, start, finish, max_degree]() {
            colorInRangeThreads(graph, start, finish, max_degree);
        });
    }

    // wait until all threads finish coloring
    thread_pool.close();
}

void colorInRangeThreads(Graph& graph, int start, int finish, int max_degree) {
    // identify boundary & internal vertices
    set<int> boundary_vertices;
    set<int> internal_vertices;
    for (int partition_vertex = start; partition_vertex < finish; ++partition_vertex) {
        bool is_boundary = false;
        for (int vertex = 0; vertex < graph.no_vertices; ++vertex) {
            // skip self
            if (partition_vertex == vertex) {
                continue;
            }

            // check if there is an edge between partition's vertex and other vertex
            // and the other vertex is not in the partition
            if (graph.adj[partition_vertex][vertex] == 1 && (vertex < start || vertex >= finish)) {
                is_boundary = true;
                break;
            }
        }

        if (is_boundary) {
            boundary_vertices.insert(partition_vertex);
        } else {
            internal_vertices.insert(partition_vertex);
        }
    }

    // color internal vertices
    for (auto internal_vertex : internal_vertices) {
        // find colors of adjacent vertices
        set<int> adj_colors;
        for (int vertex = 0; vertex < graph.no_vertices; ++vertex) {
            // skip self
            if (internal_vertex == vertex) {
                continue;
            }

            // add color to the used one
            if (graph.adj[internal_vertex][vertex] == 1 && graph.vertices[vertex].color != -1) {
                adj_colors.insert(graph.vertices[vertex].color);
            }
        }

        // assign color
        for (int color = 0; color <= max_degree; ++color) {
            if (adj_colors.find(color) == adj_colors.end()) {
                graph.vertices[internal_vertex].color = color;
                break;
            }
        }
    }

    // color boundary vertices
    for (auto boundary_vertex : boundary_vertices) {
        set<int> adj_colors;

        // lock current vertex and its neighbors
        for (int vertex = 0; vertex < graph.no_vertices; ++vertex) {
            if (graph.adj[boundary_vertex][vertex] == 1) {
                if (boundary_vertex != vertex) {
                    adj_colors.insert(graph.vertices[vertex].color);
                }
                graph.vertices[vertex].mtx.lock();
            }
        }

        // assign color
        for (int color = 0; color <= max_degree; ++color) {
            if (adj_colors.find(color) == adj_colors.end()) {
                graph.vertices[boundary_vertex].color = color;
                break;
            }
        }

        // unlock current vertex and its neighbors
        for (int vertex = 0; vertex < graph.no_vertices; ++vertex) {
            if (graph.adj[boundary_vertex][vertex] == 1) {
                graph.vertices[vertex].mtx.unlock();
            }
        }
    }
}

void colorMPIMaster(Graph& graph) {

}

void colorMPISlave(Graph& graph) {

}