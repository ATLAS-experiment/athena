/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// **********************************
// Utils for the ExaTrkX algorithm. 
// @author xiangyang.ju@cern.ch
//***********************************

#pragma once 
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

namespace ExaTrkXUtils {

void buildEdges(
    const std::vector<float>& embedFeatures,
    std::vector<int64_t>& rowIndices,
    std::vector<int64_t>& colIndices,
    int64_t numSpacepoints,
    int embeddingDim,    // dimension of embedding space
    float rVal, // radius of the ball
    int kVal    // number of nearest neighbors
);

template <typename vertex_t, typename weight_t, typename label_t>
void weaklyConnectedComponents(
vertex_t numNodes,
std::vector<vertex_t>& rowIndices,
std::vector<vertex_t>& colIndices,
std::vector<weight_t>& edgeWeights,
std::vector<label_t>& trackLabels,
float edge_cut = 0.75
) 
{
    typedef boost::adjacency_list<
      boost::vecS,            // edge list
      boost::vecS,            // vertex list
      boost::undirectedS,     // directedness
      boost::no_property,     // property associated with vertices
      float                  // property associated with edges
    > Graph; 

    Graph g(numNodes);
    for(size_t idx=0; idx < rowIndices.size(); ++idx) {
        if (edgeWeights[idx] > edge_cut) {
            boost::add_edge(rowIndices[idx], colIndices[idx], edgeWeights[idx], g);
        }
    }
    boost::connected_components(g, &trackLabels[0]);
}

using vertex_t = int;
using weight_t = float;

void CCandWalk(
    vertex_t numSpacepoints,
    const std::vector<int64_t>& rowIndices,
    const std::vector<int64_t>& colIndices,
    const std::vector<weight_t>& edgeWeights,
    std::vector<std::vector<uint32_t> >& tracks,
    float ccCut, float walkMin, float walkMax
);

// Define the graph using Boost's adjacency_list
typedef boost::property<boost::vertex_name_t, vertex_t> vertex_p;
typedef boost::property<boost::edge_weight_t, weight_t> edge_p;
typedef boost::adjacency_list<
    boost::vecS, boost::vecS,
    boost::bidirectionalS,
    vertex_p,
    edge_p,
    boost::no_property> Graph;

typedef boost::adjacency_list<
    boost::vecS, boost::vecS,
    boost::undirectedS,
    vertex_p,
    boost::no_property,
    boost::no_property> UndirectedGraph;

typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::graph_traits<Graph>::edge_descriptor Edge;

// define helper functions for CCandWalk. 
std::vector<std::vector<vertex_t>> getSimplePath(const UndirectedGraph& G);

std::vector<vertex_t> findNextNode(
    const Graph &G,
    vertex_t current_hit,
    float th_min,
    float th_add);

std::vector<std::vector<vertex_t>> buildRoads(
    const Graph &G,
    vertex_t starting_node,
    std::function<std::vector<vertex_t>(const Graph&, vertex_t)> next_node_fn,
    std::map<vertex_t, bool>& used_hits);


Graph cleanupGraph(const Graph& G, float cc_cut);

void calculateEdgeFeatures(const std::vector<float>& gNodeFeatures,
    int64_t numSpacepoints,
    const std::vector<int64_t>& rowIndices,
    const std::vector<int64_t>& colIndices,
    std::vector<float>& edgeFeatures);

} // end of ExaTrkXUtils
