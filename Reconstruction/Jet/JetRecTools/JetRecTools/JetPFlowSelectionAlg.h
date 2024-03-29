// this is a -*- C++ -*- file
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////
/// \class JetPFlowSelectionAlg
///
/// Algorithm creates a filtered collection of PFlow Objects not matched to electrons/muons.
///

#ifndef JetPFlowSelectionAlg_H
#define JetPFlowSelectionAlg_H

#include "AnaAlgorithm/AnaReentrantAlgorithm.h"
#include "StoreGate/DataHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODPFlow/FlowElementContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODCore/AuxContainerBase.h"

class JetPFlowSelectionAlg : public EL::AnaReentrantAlgorithm { 

public:

  JetPFlowSelectionAlg(const std::string & n, ISvcLocator* l) : EL::AnaReentrantAlgorithm(n,l) {}
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;

  /// Athena algorithm's Hooks
  StatusCode  initialize() override;
  StatusCode  execute(const EventContext& ctx) const override;

private:

  bool checkElectronLinks(const std::vector < ElementLink< xAOD::ElectronContainer > >& FE_ElectronLinks) const;
  bool checkMuonLinks(const std::vector < ElementLink< xAOD::MuonContainer > >& FE_MuonLinks) const;

  Gaudi::Property<std::string> m_electronID{this,"electronID","LHMedium","Select electron ID"};
  Gaudi::Property<bool> m_removeNeutralElectronFE{this,"removeNeutralElectronFE",false,"Remove neutral FE matched to electrons"};

  Gaudi::Property<std::string> m_muonID{this,"muonID","Medium","Select muon ID"};
  Gaudi::Property<bool> m_removeNeutralMuonFE{this,"removeNeutralMuonFE",false,"Remove neutral FE matched to muons"};

  SG::ReadHandleKey<xAOD::FlowElementContainer> m_ChargedPFlowContainerKey = {this, "ChargedPFlowInputContainer", "", "The input Charged PFlow Objects"};
  SG::ReadHandleKey<xAOD::FlowElementContainer> m_NeutralPFlowContainerKey = {this, "NeutralPFlowInputContainer", "", "The input Neutral PFlow Objects"};
  SG::ReadHandleKey<xAOD::ElectronContainer> m_electronContainerKey = {this, "ElectronInputContainer", "", "The input electron container"};


  SG::WriteHandleKey<xAOD::FlowElementContainer> m_outputChargedPFlowHandleKey= {this, "ChargedPFlowOutputContainer", "GlobalPFlowChargedParticleFlowObjects", "The output filtered Charged PFlow Objects"};
  SG::WriteHandleKey<xAOD::FlowElementContainer> m_outputNeutralPFlowHandleKey= {this, "NeutralPFlowOutputContainer", "GlobalPFlowNeutralParticleFlowObjects", "The output filtered Neutral PFlow Objects"};

  SG::ReadDecorHandleKey<xAOD::FlowElementContainer> m_chargedFEElectronsReadDecorKey {this, "ChargedFEElectronsReadDecorKey", "JetETMissChargedParticleFlowObjects.FE_ElectronLinks", "Key for links from charged FE to electrons"};
  SG::ReadDecorHandleKey<xAOD::FlowElementContainer> m_chargedFEMuonsReadDecorKey {this, "ChargedFEMuonsReadDecorKey", "JetETMissChargedParticleFlowObjects.FE_MuonLinks", "Key for links from charged FE to muons"};

  SG::ReadDecorHandleKey<xAOD::FlowElementContainer> m_neutralFEElectronsReadDecorKey {this, "NeutralFEElectronsReadDecorKey", "JetETMissNeutralParticleFlowObjects.FE_ElectronLinks", "Key for links from neutral FE to electrons"};

  SG::ReadDecorHandleKey<xAOD::FlowElementContainer> m_neutralFEMuonsReadDecorKey {this, "NeutralFEMuonsReadDecorKey","JetETMissNeutralParticleFlowObjects.FE_MuonLinks", "Key for links from neutral FE to muons"};


  SG::ReadDecorHandleKey<xAOD::FlowElementContainer> m_neutralFEMuons_efrac_match_DecorKey{this,"FlowElementContainer_FE_efrac_matched_muon","JetETMissNeutralParticleFlowObjects.FE_efrac_matched_muon","ReadDecorHandleKey for the fraction of neutral FlowElements cluster energy used to match to Muons"};

  SG::ReadDecorHandleKey<xAOD::FlowElementContainer> m_chargedFE_energy_match_muonReadHandleKey{this,"FlowElementContainer_ChargedFE_energy_matched_muon","JetETMissChargedParticleFlowObjects.FE_efrac_matched_muon","ReadHandleKey for the fraction of neutral FlowElements cluster energy used to match to Muons"};


};

#endif
