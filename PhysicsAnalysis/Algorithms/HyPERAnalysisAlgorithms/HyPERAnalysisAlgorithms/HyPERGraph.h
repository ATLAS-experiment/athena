/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HYPERANALYSISALGORITHMS_HYPERGRAPH_H
#define HYPERANALYSISALGORITHMS_HYPERGRAPH_H

#include <math.h>

#include <iostream>
#include <list>
#include <map>
#include <vector>

#include "HyPERAnalysisAlgorithms/GraphBase.h"

namespace EventReco {
// @class HyPERGraph
// @brief This class is in charge of storing the graph structure and features of
// the HyPER model. It inherits from the GraphBase class.
class HyPERGraph : public GraphBase {
  using HyperEdgeIndex = std::vector<int64_t>;

 public:
  HyPERGraph() : GraphBase() {};
  virtual ~HyPERGraph() = default;

  virtual void addNode(const Features& attributes) override;
  virtual void addEdge(int64_t source, int64_t target,
                       const Features& attributes) override;
  virtual void addGlobal(const Features& attributes) override;

  void buildEdgeIndices();
  void buildHyperEdges(int64_t order);
  const std::vector<EdgeIndex>& getEdgeIndicesVector() const {
    return m_edgeIndices;
  }
  HyperEdgeIndex getHyperEdgeIndices(std::size_t index) const {
    return m_hyperEdgeIndices[index];
  }
  int64_t nHyperEdges() const { return m_nHyperEdges; }
  int64_t hyperEdgeOrder() const { return m_hyperEdgeOrder; }

  virtual void printGraph() const;
  virtual void printGraphInputsForValidation() const;
  virtual void clearGraph();

 protected:
  std::vector<EdgeIndex> m_edgeIndices = {};
  std::vector<HyperEdgeIndex> m_hyperEdgeIndices = {};
  int64_t m_nHyperEdges = 0;
  int64_t m_hyperEdgeOrder = 0;
};
}  // namespace EventReco

#endif  // HYPERANALYSISALGORITHMS_HYPERGRAPH_H
