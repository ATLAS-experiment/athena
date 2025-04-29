/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ExaTrkXUtils.hpp"
#include <queue>       // std::priority_queue

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graph_traits.hpp>

namespace ExaTrkXUtils {

void buildEdges(
    const std::vector<float>& embedFeatures,
    std::vector<int64_t>& senders,
    std::vector<int64_t>& receivers,
    int64_t numSpacepoints,
    int embeddingDim,    // dimension of embedding space
    float rVal, // radius of the ball
    int kVal    // number of nearest neighbors
){
    // calculate the distances between two spacepoints in the embedding space
    // and keep the k-nearest neighbours within the radius r
    // the distance is calculated using the L2 norm
    // the edge list is with dimension of [2, number-of-edges]
    // the first row is the source node index
    // the second row is the target node index
    // computing complexity is O(N^2 * D) where N is the number of spacepoints and D is the embedding dimension.
    // space complexity is O(N * (D + k)).

    // Helper lambda to calculate squared distance between two points
    auto squaredDistance = [&](int64_t i, int64_t j) {
        float dist = 0.0;
        for (int d = 0; d < embeddingDim; ++d) {
            float diff = embedFeatures[i * embeddingDim + d] - embedFeatures[j * embeddingDim + d];
            dist += diff * diff;
        }
        return dist;
    };

    // Radius squared to avoid taking square root repeatedly
    float radiusSquared = rVal * rVal;

    for (int64_t i = 0; i < numSpacepoints; i++) {
        // Min-heap (priority queue) to store nearest neighbors
        std::priority_queue<std::pair<float, int64_t>> nearestNeighbors;

        for (int64_t j = i + 1; j < numSpacepoints; j++) {

            float distSquared = squaredDistance(i, j);
            if (distSquared <= radiusSquared) {
                nearestNeighbors.push({distSquared, j});
                // Maintain top k neighbors in the heap
                if (nearestNeighbors.size() > (unsigned long) kVal) {
                    nearestNeighbors.pop();
                }
            }
        }
        // Add the k-nearest neighbors to the edge list
        while (!nearestNeighbors.empty()) {
            senders.push_back(i);
            receivers.push_back(nearestNeighbors.top().second);
            nearestNeighbors.pop();
        }
    }
}

void CCandWalk(
    vertex_t numSpacepoints,
    const std::vector<int64_t>& rowIndices,
    const std::vector<int64_t>& colIndices,
    const std::vector<weight_t>& edgeWeights,
    std::vector<std::vector<uint32_t> >& tracks,
    float ccCut, float walkMin, float walkMax
){
    Graph G;
    std::map<vertex_t, bool> used_hits;
    // use the space point ID as the vertex name.
    for (vertex_t i = 0; i < numSpacepoints; i++) {
        add_vertex(i, G);
        used_hits[i] = false;
    }
    for(size_t idx=0; idx < rowIndices.size(); ++idx) {
        add_edge(rowIndices[idx], colIndices[idx], edgeWeights[idx], G);
    }

    // remove isolated vertices and edges with weight <= ccCut
    Graph newG = cleanupGraph(G, ccCut);

    UndirectedGraph ugraph;
    // add vertices from newG to ugraph.
    for (auto v : boost::make_iterator_range(vertices(newG))) {
        auto name = boost::get(boost::vertex_name, newG, v);
        add_vertex(name, ugraph);
    }
    // add edges from newG to ugraph.
    auto [edge_b, edge_e] = boost::edges(newG);
    for (auto it = edge_b; it != edge_e; ++it) {
        int source = boost::source(*it, newG);
        int target = boost::target(*it, newG);
        add_edge(source, target, ugraph);
    }

    std::vector<std::vector<vertex_t>> sub_graphs = getSimplePath(ugraph);
    // mark the used hits.
    for (const auto& track : sub_graphs) {
        for (auto hit_id : track) {
            used_hits[hit_id] = true;
        }
    }

    std::vector<Vertex> topo_order;
    boost::topological_sort(newG, std::back_inserter(topo_order));

    // Define the next_hit function
    auto next_node_fn = [&](const Graph &G, vertex_t current_hit) {
        return findNextNode(G, current_hit, walkMin, walkMax);
    };

    // Traverse the nodes in topological order
    for(auto it = topo_order.rbegin(); it != topo_order.rend(); ++it) {
        auto node_id = *it;
        int hit_id = boost::get(boost::vertex_name, newG, node_id);
        if (used_hits[hit_id]) continue;

        // Build roads (tracks) starting from the current node
        auto roads = buildRoads(newG, node_id, next_node_fn, used_hits);
        used_hits[node_id] = true;
        if (roads.empty()) {
            continue;
        }

        // Find the longest road and remove the last element (-1)
        std::vector<int>& longest_road = *std::max_element(roads.begin(), roads.end(),
            [](const std::vector<int> &a, const std::vector<int> &b) {
                return a.size() < b.size();
            });

        if (longest_road.size() >= 3) {
            std::vector<vertex_t> track;
            for (int node_id : longest_road) {
                auto hit_id = boost::get(boost::vertex_name, newG, node_id);
                used_hits[hit_id] = true;
                track.push_back(hit_id);
            }
            sub_graphs.push_back(track);
        }
    }

    // copy subgraph to tracks.
    tracks.clear();
    for (const auto& track : sub_graphs) {
        std::vector<uint32_t> this_track{track.begin(), track.end()};
        tracks.push_back(this_track);
    }
}

// implement the helper functions
std::vector<std::vector<vertex_t>> getSimplePath(const UndirectedGraph& G) {
    std::vector<std::vector<vertex_t>> final_tracks;
    // Get weakly connected components
    std::vector<vertex_t> component(num_vertices(G));
    size_t num_components = boost::connected_components(G, &component[0]);

    std::vector<std::vector<Vertex> > component_groups(num_components);
    for(size_t i = 0; i < component.size(); ++i) {
        component_groups[component[i]].push_back(i);
    }

    // loop over the sorted groups.
    for(const auto& sub_graph : component_groups) {
        if (sub_graph.size() < 3) {
            continue;
        }
        bool is_signal_path = true;
        // Check if all nodes in the sub_graph are signal paths
        for (int node : sub_graph) {
            if (degree(node, G) > 2) {
                is_signal_path = false;
                break;
            }
        }

        // If it's a signal path, collect the hit_ids
        if (is_signal_path) {
            std::vector<vertex_t> track;
            for (int node : sub_graph) {
                vertex_t hit_id = boost::get(boost::vertex_name, G, node);
                track.push_back(hit_id);
            }
            final_tracks.push_back(track);
        }
    }
    return final_tracks;
}

std::vector<vertex_t> findNextNode(
    const Graph &G,
    vertex_t current_hit,
    float th_min,
    float th_add
){
    std::vector<vertex_t> next_hits;
    auto [begin, end] = boost::out_edges(current_hit, G);

    std::vector<std::pair<vertex_t, double>> neighbors_scores;
    for (auto it = begin; it != end; ++it) {
        vertex_t neighbor = target(*it, G);
        auto score = boost::get(boost::edge_weight, G, *it);

        if (neighbor == current_hit || score <= th_min) continue;
        neighbors_scores.push_back({neighbor, score});
    }

    if (neighbors_scores.empty()) return {};

    // Find the best neighbor
    auto best_neighbor = *std::max_element(neighbors_scores.begin(), neighbors_scores.end(),
                                      [](const std::pair<int, double> &a, const std::pair<int, double> &b) {
                                          return a.second < b.second;
                                      });

    // Add neighbors with score > th_add
    for (const auto &neighbor : neighbors_scores) {
        if (neighbor.second > th_add) {
            next_hits.push_back(neighbor.first);
        }
    }

    // If no neighbors were added, add the best neighbor
    if (next_hits.empty()) {
        next_hits.push_back(best_neighbor.first);
    }

    return next_hits;
}

std::vector<std::vector<vertex_t>> buildRoads(
    const Graph &G,
    vertex_t starting_node,
    std::function<std::vector<vertex_t>(const Graph&, vertex_t)> next_node_fn,
    std::map<vertex_t, bool>& used_hits
){
    std::vector<std::vector<int>> path = {{starting_node}};

    while (true) {
        std::vector<std::vector<int>> new_path;
        bool is_all_done = true;
        // loop over each path and extend it.
        for (const auto &pp : path) {
            vertex_t start = pp.back();

            if (start == -1) {
                new_path.push_back(pp);
                continue;
            }

            auto next_hits = next_node_fn(G, start);
            // remove used hits.
            next_hits.erase(std::remove_if(next_hits.begin(), next_hits.end(),
                [&](int node_id) {
                    auto hit_id = boost::get(boost::vertex_name, G, node_id);
                    return used_hits[hit_id];
                }), next_hits.end());

            if (next_hits.empty()) {
                new_path.push_back(pp);
            } else {
                is_all_done = false;
                for (int nh : next_hits) {
                    std::vector<int> pp_extended = pp;
                    pp_extended.push_back(nh);
                    new_path.push_back(pp_extended);
                }
            }
        }

        path = new_path;
        if (is_all_done) break;
    }
    return path;
}

Graph cleanupGraph(const Graph& G, float ccCut) {
    // remove fake edges and isolated vertices.
    Graph newG;

    // add vertices of G to newG, including the vertex name.
    std::map<vertex_t, vertex_t> old_vertex_to_new;
    vertex_t old_vertex_id = 0;
    vertex_t new_vertex_id = 0;
    for (auto v : boost::make_iterator_range(vertices(G))) {
        auto name = boost::get(boost::vertex_name, G, v);
        if (in_degree(v, G) == 0 && out_degree(v, G) == 0) {
            old_vertex_id ++;
            continue; // remove isolated vertices.
        }
        add_vertex(name, newG);
        old_vertex_to_new[old_vertex_id] = new_vertex_id;
        new_vertex_id ++;
        old_vertex_id ++;
    }
    // add edges of G to newG.
    auto [edge_b, edge_e] = boost::edges(G);
    for (auto it = edge_b; it != edge_e; ++it) {
        auto source = boost::source(*it, G);
        auto target = boost::target(*it, G);
        source = old_vertex_to_new[source];
        target = old_vertex_to_new[target];
        auto weight = boost::get(boost::edge_weight, G, *it);
        if (weight <= ccCut) continue;
        add_edge(source, target, weight, newG);
    }
    return newG;
}

void calculateEdgeFeatures(const std::vector<float>& gNodeFeatures,
    int64_t numSpacepoints,
    const std::vector<int64_t>& rowIndices,
    const std::vector<int64_t>& colIndices,
    std::vector<float>& edgeFeatures
){
    // calculate the edge features from the node features.
    // the edge features are: [dr, dphi, dz, deta, phislope, rphislope]
    // dr: difference in r.
    // dphi: difference in phi.
    // dz: difference in z.
    // deta: difference in eta.
    // phislope: dphi / dr.
    // rphislope: (r[dst] + r[src]) / 2 * phislope.
    // So edge features are with dimension of [6, number-of-edges].
    // We assume the first 4 node features are r, phi, z, eta.

    int64_t numFeatures = gNodeFeatures.size() / numSpacepoints;
    edgeFeatures.clear();
    edgeFeatures.reserve(rowIndices.size() * 6);
    auto diffPhi = [](float phi1, float phi2) {
        float dphi = (phi1 - phi2) * M_PI;
        if (dphi > M_PI) {
            dphi -= 2 * M_PI;
        } else if (dphi <= -M_PI) {
            dphi += 2 * M_PI;
        }
        return dphi / M_PI;
    };
    for (size_t idx = 0; idx < rowIndices.size(); ++idx) {
        int64_t src = rowIndices[idx];
        int64_t dst = colIndices[idx];
        std::vector<float> src_features(gNodeFeatures.begin() + src * numFeatures, 
                                        gNodeFeatures.begin() + src * numFeatures + 4);
        std::vector<float> dst_features(gNodeFeatures.begin() + dst * numFeatures, 
                                        gNodeFeatures.begin() + dst * numFeatures + 4);

        float dr = dst_features[0] - src_features[0];
        float dphi = diffPhi(dst_features[1], src_features[1]);
        float dz = dst_features[2] - src_features[2];
        float deta = dst_features[3] - src_features[3];
        float phislope = (fabs(dr) > 1e-7) ? dphi / dr : 0.;
        float rphislope = (fabs(phislope) > 1e-7) ? (dst_features[0] + src_features[0]) / 2 * phislope : 0.0; 

        edgeFeatures.push_back(dr);
        edgeFeatures.push_back(dphi);
        edgeFeatures.push_back(dz);
        edgeFeatures.push_back(deta);
        edgeFeatures.push_back(phislope);
        edgeFeatures.push_back(rphislope);
    }
}

} // end of ExaTrkXUtils
