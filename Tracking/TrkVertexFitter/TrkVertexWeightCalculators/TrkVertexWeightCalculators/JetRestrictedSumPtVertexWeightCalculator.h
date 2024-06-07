/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrkVertexWeightCalculator_JetRestrictedSumPtVertexWeightCalculator_H
#define TrkVertexWeightCalculator_JetRestrictedSumPtVertexWeightCalculator_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkVertexFitterInterfaces/IVertexWeightCalculator.h"

#include "ParticlesInConeTools/ITrackParticlesInConeTool.h"

// xAOD include
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/VertexFwd.h"
#include "xAODJet/JetContainer.h"

/**
 * @class Trk::JetRestrictedSumPtVertexWeightCalculator
 *
 * @author Johanna Bronner, November 2009
 *
 * ---------------------------------------------------
 * Changes:
 *
 * David Shope <david.richard.shope@cern.ch> (2016-06-01)
 *
 *   EDM Migration to xAOD - remove method using VxCandidate
 *
 */

namespace Trk {

class JetRestrictedSumPtVertexWeightCalculator
  : public AthAlgTool
  , virtual public IVertexWeightCalculator
{
public:
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /**
   * constructor
   */

  JetRestrictedSumPtVertexWeightCalculator(const std::string& t,
                              const std::string& n,
                              const IInterface* p);

  /**
   * destructor
   */

  virtual ~JetRestrictedSumPtVertexWeightCalculator() = default;

  /**
   * WeightCalculator
   */

  virtual double estimateSignalCompatibility(const xAOD::Vertex& vertex) const override final;

private:
  /**
   * Flag to Set SumPt^2 not SumPt as selection criteria
   */
  Gaudi::Property<bool> m_doSumPt2Selection{ this, "DoSumPt2Selection", true };
  Gaudi::Property<float> m_cone_dR{ this, "JetConeDeltaR", 0.4 };
  Gaudi::Property<float> m_jet_ptmin{ this, "JetMinPt", 20e3 };
  ToolHandle<xAOD::ITrackParticlesInConeTool> m_tracksInCone{ this, "TracksInConeTool", "xAOD::TrackParticlesInConeTool/TrackParticlesInConeTool" };
  SG::ReadHandleKey<xAOD::JetContainer> m_jetContKey{ this, "JetContainer", "AntiKt4EMTopoJets", "Name of the jet container" };

}; // end of class description
} // end of namespace definition

#endif
