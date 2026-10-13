/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKVERTEXWEIGHTCALCULATORS_VERTEXWEIGHTCALCULATOR_H
#define TRKVERTEXWEIGHTCALCULATORS_VERTEXWEIGHTCALCULATOR_H

#include "StoreGate/ReadDecorHandleKey.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include <TrkVertexFitterInterfaces/IVertexWeightCalculator.h>
#include <xAODTracking/VertexContainer.h>

#include <memory>
#include <string>

/**
 * @brief A tool to select the primary vertex associated to the hard-scatter
 * using NN-based tool (e.g PVNN or HGamNN)
 *
 */
class VertexWeightCalculator final
    : public AthAlgTool,
      virtual public Trk::IVertexWeightCalculator {
public:
  VertexWeightCalculator(const std::string& t,
                            const std::string& n,
                            const IInterface* p);

  virtual ~VertexWeightCalculator() override;
  virtual StatusCode initialize() override;
  /**
   * @brief Estimate the compatibility of the vertex with a hard scatter vertex,
   * with respect to pileup vertices
   */
  virtual double
  estimateSignalCompatibility(const xAOD::Vertex &vertex) const override;

private:
  SG::ReadDecorHandleKey<xAOD::VertexContainer> m_ScoreKey{
      this, "ScoreKey", "",
      "Name of the hard-scatter decoration"};
};

#endif // TRKVERTEXWEIGHTCALCULATORS_VERTEXWEIGHTCALCULATOR_H
