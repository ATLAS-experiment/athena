/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKVERTEXWEIGHTCALCULATORS_GNNVERTEXWEIGHTCALCULATOR_H
#define TRKVERTEXWEIGHTCALCULATORS_GNNVERTEXWEIGHTCALCULATOR_H

#include "StoreGate/ReadDecorHandleKey.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include <TrkVertexFitterInterfaces/IVertexWeightCalculator.h>
#include <xAODTracking/VertexContainer.h>

#include <memory>
#include <string>

/**
 * @brief A tool to select the primary vertex associated to the hard-scatter
 * using a GNN
 *
 * This tool uses a GNN to select the hard-scatter vertex from a collection of
 * primary vertices.
 * It reads the GNN score created by GNNVertexDecoratorAlg
 *
 */
class GNNVertexWeightCalculator final
    : public AthAlgTool,
      virtual public Trk::IVertexWeightCalculator {
public:
  GNNVertexWeightCalculator(const std::string& t,
                            const std::string& n,
                            const IInterface* p);

  virtual ~GNNVertexWeightCalculator() override;
  virtual StatusCode initialize() override;
  /**
   * @brief Estimate the compatibility of the vertex with a hard scatter vertex,
   * with respect to pileup vertices
   */
  virtual double
  estimateSignalCompatibility(const xAOD::Vertex &vertex) const override;

private:
  SG::ReadDecorHandleKey<xAOD::VertexContainer> m_gnnScoreKey{
      this, "GNNScoreKey", "PrimaryVertices_initial.HSGN2_phsvertex",
      "Name of the hard-scatter GNN decoration"};
};

#endif // TRKVERTEXWEIGHTCALCULATORS_GNNVERTEXWEIGHTCALCULATOR_H
