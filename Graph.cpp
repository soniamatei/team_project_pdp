#include "ThreadPool.h"
#include "Graph.h"
#include <iostream>


Graph::Graph(int no_vertices) : no_vertices(no_vertices),
                                colors(no_vertices),
                                adj(no_vertices, std::vector<int>(no_vertices, 0))
{
    for (Color& color : colors) {
        color.color = -1;
    }
}

void Graph::addEdge(int vertex_1, int vertex_2) {
    adj[vertex_1][vertex_2] = 1;
    adj[vertex_2][vertex_1] = 1;
}

vector<pair<int, int>> Graph::colorThreads(int no_threads) {

    { // thread pool joins on destroy
        ThreadPool thread_pool(no_threads);

        for (int thread_index = 0; thread_index < no_threads; thread_index++) {
            int start = thread_index * no_vertices / no_threads;
            int finish = (thread_index + 1) * no_vertices / no_threads;
            cout << start << " " << finish << endl;
            thread_pool.enqueue([this, start, finish]() {
                this->colorInRange(start, finish);
            });
        }

        thread_pool.close();
    }

    vector<pair<int, int>> node_color;

    // create a vector with the nodes and corresponding colors
    for (int i = 0; i < no_vertices; ++i) {
        cout << colors[i].color;
        node_color.emplace_back(i, colors[i].color);
    }

    return node_color;
}

void Graph::colorInRange(int start, int finish) {

    // iterate through vertices in the given range
    for (int vertex = start; vertex < finish; vertex++) {
        if (colors[vertex].color == -1) {

            // find first available color
            // fictional color represented as a number in range [0, n)
            for (int color_index = 0; color_index < no_vertices; color_index++) {

                bool available = true;
                for (int i = 0; i < no_vertices; i++) {

                    // if there is an adjacent node with the given color, go to next color
                    if (adj[vertex][i] && colors[i].color == color_index) {
                        available = false;
                        break;
                    }
                }

                if(available) {

                    // verify one last time if the color hadn't been written since the check
                    unique_lock lock(colors[vertex].mtx);
                    cout << "a";
                    if (colors[vertex].color == -1) {
                        colors[vertex].color = color_index;
                        cout << "b" << colors[vertex].color;
                        lock.unlock();
                        break;
                    }
                    lock.unlock();
                }
            }
        }
    }
}