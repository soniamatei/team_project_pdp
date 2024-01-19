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
void colorThreads(Graph& graph, int no_threads) {
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
            colorPartitionThreads(graph, start, finish, max_degree);
        });
    }

    // wait until all threads finish coloring
    thread_pool.close();
}

void colorPartitionThreads(Graph& graph, int start, int finish, int max_degree) {
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


void sendVector(const std::vector<int>& vector, int dest) {
    // convert the vector to an array
    int size = vector.size();
    const int* data = vector.data();

    // send the size of the vector first
    MPI_Send(&size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

    // send the actual data
    MPI_Send(data, size, MPI_INT, dest, 1, MPI_COMM_WORLD);
}

std::vector<int> receiveVector(int src) {
    int size;

    // receive the size of the vector first
    MPI_Recv(&size, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // receive the actual data
    std::vector<int> receivedData(size);
    MPI_Recv(receivedData.data(), size, MPI_INT, src, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return receivedData;
}

void colorInternalsMPI(Graph& graph, int start, int finish, int max_degree) {
    // identify boundary & internal vertices
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

        if (!is_boundary) {
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
}