/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef FAKERATEFILTERALG_H
#define FAKERATEFILTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthContainers/ConstDataVector.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ShallowCopyDecorDeps.h"
#include "AsgTools/ToolHandle.h"
#include "InDetTrackSystematicsTools/IInDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/IJetTrackFilterTool.h"
#include "InDetTrackSystematicsTools/ITrackFilterTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/SystematicVariation.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

namespace InDet {

  /// Applies inclusive and/or TIDE fake rate filter tools to tracks,
  /// writing surviving tracks to a new TrackParticleContainer.
  /// Tracks inside jets (DeltaR < DeltaRCut) are filtered by the
  /// JetFilterTool
  class FakeRateFilterAlg : public AthReentrantAlgorithm {
  public:
    FakeRateFilterAlg(const std::string& name, ISvcLocator* pSvcLocator);
    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

  private:
    // Inclusive fake rate tool (for tracks outside jets)
    ToolHandle<InDet::IInDetTrackTruthFilterTool> m_inclusiveFilterTool{
      this, "InclusiveFilterTool", "",
      "Inclusive fake rate tool (tracks outside jets)"};

    // TIDE fake rate tool (for tracks inside jets)
    ToolHandle<InDet::IJetTrackFilterTool> m_jetFilterTool{
      this, "JetFilterTool", "",
      "TIDE fake rate tool (tracks inside jets)"};

    ToolHandle<InDet::ITrackFilterTool> m_trackFilterTool{
      this, "TrackFilterTool", "",
      "Combined inclusive and jet track filter tool"
    };

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inTrackKey{
      this, "InputTrackContainer", "InDetTrackParticles",
      "Input track container"};

    SG::WriteHandleKey<xAOD::TrackParticleContainer> m_outTrackKey{
      this, "OutputTrackContainer", "InDetTrackParticles_Filtered",
      "Output track container"};

    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey{
      this, "JetContainer", "AntiKt4EMTopoJets",
      "Jet container"};

    SG::ShallowCopyDecorDeps<xAOD::TrackParticleContainer> m_decorDeps{
      this, "DecorDeps", {},
      "Decorations to propagate"};

    Gaudi::Property<double> m_deltaR{
      this, "DeltaRCut", 0.4,
      "DeltaR cone to define in-jet"};

      Gaudi::Property<std::string> m_systematicVariation{
      this, "SystematicVariation", "",
      "Systematic variation to apply"};
  };

} // namespace InDet
#endif
