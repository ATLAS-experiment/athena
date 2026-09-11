#ifndef TOPHYPER_GRAPH_H
#define TOPHYPER_GRAPH_H

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Diego Baron

#include <cstdint>
#include <utility>
#include <vector>

namespace EventReco {
// Define some types used across the graph.
using Features = std::vector<float>;
using EdgeIndex = std::pair<int64_t, int64_t>;
// @class Node
// @brief This class stores a node in the graph.
class Node {
 public:
  Node() = default;
  Node(const Features& attributes, int64_t nodeIndex)
      : m_nodeFeatures(attributes), m_nodeIndex(nodeIndex) {};
  const Features& getFeatures() const { return m_nodeFeatures; };
  Features& getFeatures() { return m_nodeFeatures; };
  int64_t getNodeIndex() const { return m_nodeIndex; };

 private:
  Features m_nodeFeatures = {};
  int64_t m_nodeIndex = -1;
};
// @class Edge
// @brief This class stores an edge in the graph.
class Edge {
 public:
  Edge() = default;
  Edge(int64_t source, int64_t target, const Features& attributes)
      : m_edgeFeatures(attributes),
        m_edgeIndex(std::make_pair(source, target)) {};
  const Features& getFeatures() const { return m_edgeFeatures; };
  EdgeIndex getIndices() const { return m_edgeIndex; };

 private:
  Features m_edgeFeatures = {};
  EdgeIndex m_edgeIndex = {};
};
// @class Global
// @brief This class stores the global features of the graph.
class Global {
 public:
  Global() = default;
  Global(const Features& attributes) : m_globalFeatures(attributes) {};
  const Features& getFeatures() const { return m_globalFeatures; };

 private:
  Features m_globalFeatures = {};
};

// @class GraphBase
// @brief This class is the base class for the graph objects.
// It represents a graph on an event by event basis.
// It is used to store the graph structure and features of the graph.
// Used for evaluation.
class GraphBase {
 public:
  GraphBase() = default;
  // Methods to build the graph
  virtual void addNode(const Features& attributes) = 0;
  virtual void addEdge(int64_t source, int64_t target,
                       const Features& attributes) = 0;
  virtual void addGlobal(const Features& attributes) = 0;
  // Methods to get the graph features
  Features getNodeFeats(std::size_t index) const {
    return m_nodes[index].getFeatures();
  };
  Features& getNodeFeats(std::size_t index) {
    return m_nodes[index].getFeatures();
  };
  Features getEdgeFeats(std::size_t index) const {
    return m_edges[index].getFeatures();
  };
  Features getGlobalFeats() const { return m_global.getFeatures(); };
  EdgeIndex getEdgeIndicesVector(std::size_t index) const {
    return m_edges[index].getIndices();
  };

  int64_t nNodes() const { return m_nNodes; };
  int64_t nEdges() const { return m_nEdges; };
  int64_t nGlobal() const { return m_global.getFeatures().size(); };

 protected:
  Global m_global;
  std::vector<Node> m_nodes;
  std::vector<Edge> m_edges;
  int64_t m_nNodes = 0;
  int64_t m_nEdges = 0;
};
}  // namespace EventReco

#endif
