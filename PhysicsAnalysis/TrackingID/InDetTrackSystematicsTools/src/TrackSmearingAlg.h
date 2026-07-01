/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_TRACKSMEARINGALG_H
#define INDET_TRACKSMEARINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AsgTools/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "InDetTrackSystematicsTools/IInDetTrackSmearingTool.h"
#include "InDetTrackSystematicsTools/IInDetTrackBiasingTool.h"
#include "PATInterfaces/SystematicSet.h"

namespace InDet {

/// Makes a shallow copy of the input track collection and applies
/// smearing and (optionally) biasing tools in-place, using the
/// reentrant tool API.  One algorithm instance per systematic variation;
/// the underlying tools are shared across instances.
class TrackSmearingAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

private:
  PublicToolHandle<IInDetTrackSmearingTool> m_smearingTool{this, "SmearingTool", ""};
  PublicToolHandle<IInDetTrackBiasingTool>  m_biasingTool{this, "BiasingTool", ""};
  SG::ReadHandleKey<xAOD::TrackParticleContainer>  m_inTrackKey{
      this, "InputTrackContainer", "InDetTrackParticles", ""};
  SG::WriteHandleKey<xAOD::TrackParticleContainer> m_outTrackKey{
      this, "OutputTrackContainer", "InDetTrackParticles_smeared", ""};
  Gaudi::Property<std::string> m_systematicVariation{
      this, "SystematicVariation", "",
      "Systematic variation name (empty = nominal)"};

  // Built from m_systematicVariation in initialize(), const thereafter.
  // Passed to the reentrant applyCorrection(track, syst) overload in execute().
  CP::SystematicSet m_systSet;
};

} // namespace InDet

#endif // INDET_TRACKSMEARINGALG_H
