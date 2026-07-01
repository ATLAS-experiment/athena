/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_JETTRACKFILTERINGALG_H
#define INDET_JETTRACKFILTERINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AsgTools/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "AthLinks/ElementLink.h"
#include "InDetTrackSystematicsTools/IInclusiveTrackFilterTool.h"
#include "InDetTrackSystematicsTools/IInDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/IJetTrackFilterTool.h"
#include "PATInterfaces/SystematicSet.h"

namespace InDet {

/// Filters ghost-track links on jets using efficiency/fake-rate tools.
///
/// Reads a ghost track decoration on the jet, applies the configured
/// filter tools to each referenced track, and writes surviving links
/// to a new jet decoration. One instance per filter syst.
///
/// Per-track dispatch mirrors TrackSystematicsAlg: LRT tracks are
/// evaluated by the LRT tool; STD tracks are evaluated by the STD
/// tool (and the Jet tool for TIDE systematics). If a tool is not
/// configured the corresponding track type passes unconditionally.
///
/// This additional algorithm exists to serve (at the time of writing)
/// the flavor tagging group. Tracks in flavor tagging are always
/// accessed through jets (as element links), and as such thinning the
/// element links is simpler to manage than duplicating the tracks as
/// view container.
class JetTrackFilteringAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

private:
  using IPC  = xAOD::IParticleContainer;
  using IPLV = std::vector<ElementLink<xAOD::IParticleContainer>>;

  PublicToolHandle<IInclusiveTrackFilterTool>  m_lrtFilterTool{this, "LRTFilterTool", ""};
  PublicToolHandle<IInDetTrackTruthFilterTool> m_stdFilterTool{this, "STDFilterTool", ""};
  PublicToolHandle<IJetTrackFilterTool>        m_jetFilterTool{this, "JetFilterTool", ""};

  SG::ReadHandleKey<xAOD::JetContainer> m_jetKey{
      this, "JetCollection", "", "Jet container"};
  SG::ReadDecorHandleKey<IPC> m_inGhostKey{
      this, "InGhostTracks", "",
      "Ghost track decoration to read from the jet"};
  SG::WriteDecorHandleKey<IPC> m_outGhostKey{
      this, "OutGhostTracks", "",
      "Ghost track decoration to write onto the jet (filtered subset)"};

  Gaudi::Property<std::string> m_systematicVariation{
      this, "SystematicVariation", "",
      "Systematic variation name (empty = nominal)"};

  CP::SystematicSet m_systSet;  // built in initialize(), const thereafter
};

} // namespace InDet

#endif // INDET_JETTRACKFILTERINGALG_H
