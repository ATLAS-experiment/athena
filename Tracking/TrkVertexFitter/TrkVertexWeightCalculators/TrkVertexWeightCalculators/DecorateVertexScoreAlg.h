/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DECORATEVERTEXSCOREALG_H
#define DECORATEVERTEXSCOREALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgTools/ToolHandle.h>
#include <TrkVertexFitterInterfaces/IVertexWeightCalculator.h>
#include <xAODTracking/VertexContainerFwd.h>

#include <string>

/**
 * @brief An algorithm to decorate vertices with a score
 *
 * This algorithm uses the VertexWeightCalculator to decorate vertices with a
 * score. The score should be related to the classification of the vertex as
 * coming from the hard-scatter interaction or pileup.
 *
 */
class DecorateVertexScoreAlg : public EL::AnaAlgorithm {
 public:
  DecorateVertexScoreAlg(const std::string& name,
                         ISvcLocator* svcLoc = nullptr);

  virtual StatusCode initialize() override;
  StatusCode execute() override;

 private:
  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainer_key{
      this, "VertexContainer", "PrimaryVertices", "The input Vertex container"};

  SG::WriteDecorHandleKey<xAOD::VertexContainer> m_vertexScoreDecor_key{
      this, "VertexScoreDecor", m_vertexContainer_key, "score",
      "The output decoration for the score vertices"};

  ToolHandle<Trk::IVertexWeightCalculator> m_vertexWeightCalculator{
      this, "VertexWeightCalculator", "BDTVertexWeightCalculator",
      "The tool to compute the score"};
};

#endif  // DECORATEVERTEXSCOREALG_H