/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HLTSEEDING_L1TRIGGERRESULTMAKER_H
#define HLTSEEDING_L1TRIGGERRESULTMAKER_H

// Local includes
#include "HLTSeeding/IRoIThresholdsTool.h"

// Trigger includes
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/jFexFwdElRoIContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"
#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/jFexLRJetRoIContainer.h"
#include "xAODTrigger/gFexJetRoIContainer.h"
#include "xAODTrigger/gFexGlobalRoIContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "xAODTrigger/TrigCompositeContainer.h"

// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

/** @class L1TriggerResultMaker
 *  @brief Algorithm creating L1TriggerResult and linking the relevant L1 xAOD collections to it
 **/
class L1TriggerResultMaker : public AthReentrantAlgorithm {
public:
  /// Standard constructor
  L1TriggerResultMaker(const std::string& name, ISvcLocator* svcLoc);

  // ------------------------- AthReentrantAlgorithm methods -------------------
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& eventContext) const override;

private:
  // ------------------------- Properties --------------------------------------
  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_l1TriggerResultWHKey {
    this, "L1TriggerResultWHKey", "L1TriggerResult",
    "Key of the output L1 Trigger Result"};

  // Muon RoIs
  SG::ReadHandleKeyArray<xAOD::MuonRoIContainer> m_muRoIKeys {
    this, "MuRoIKeys", {"LVL1MuonRoIs"},
    "Keys of the muon RoI container to be linked to L1 Trigger Result"};

  // eFex EM RoIs
  SG::ReadHandleKeyArray<xAOD::eFexEMRoIContainer> m_eFexEMRoIKeys {
    this, "eFexEMRoIKeys", {"L1_eEMRoI"},
    "Keys of the eFex EM RoI container to be linked to L1 Trigger Result"};

  // eFex Tau RoIs
  SG::ReadHandleKeyArray<xAOD::eFexTauRoIContainer> m_eFexTauRoIKeys {
    this, "eFexTauRoIKeys", {"L1_eTauRoI"},
    "Keys of the eFex Tau RoI container to be linked to L1 Trigger Result"};

  // jFex Fwd El RoIs
  SG::ReadHandleKeyArray<xAOD::jFexFwdElRoIContainer> m_jFexFwdElRoIKeys {
          this, "jFexFwdElRoIKeys", {"L1_jFexFwdElRoI"},
          "Keys of the jFex Fwd El RoI container to be linked to L1 Trigger Result"};

  // jFex Tau RoIs
  SG::ReadHandleKeyArray<xAOD::jFexTauRoIContainer> m_jFexTauRoIKeys {
    this, "jFexTauRoIKeys", {"L1_jFexTauRoI"},
    "Keys of the jFex Tau RoI container to be linked to L1 Trigger Result"};

  // jFex small-R Jet RoIs
  SG::ReadHandleKeyArray<xAOD::jFexSRJetRoIContainer> m_jFexSRJetRoIKeys {
    this, "jFexSRJetRoIKeys", {"L1_jFexSRJetRoI"},
    "Keys of the jFex small-R Jet RoI container to be linked to L1 Trigger Result"};

  // jFex large-R Jet RoIs
  SG::ReadHandleKeyArray<xAOD::jFexLRJetRoIContainer> m_jFexLRJetRoIKeys {
    this, "jFexLRJetRoIKeys", {"L1_jFexLRJetRoI"},
    "Keys of the jFex large-R Jet RoI container to be linked to L1 Trigger Result"};

  // gFex small-R Jet RoIs
  SG::ReadHandleKeyArray<xAOD::gFexJetRoIContainer> m_gFexSRJetRoIKeys {
    this, "gFexSRJetRoIKeys", {"L1_gFexSRJetRoI"},
    "Keys of the gFex small-R Jet RoI container to be linked to L1 Trigger Result"};

  // gFex large-R Jet RoIs
  SG::ReadHandleKeyArray<xAOD::gFexJetRoIContainer> m_gFexLRJetRoIKeys {
    this, "gFexLRJetRoIKeys", {"L1_gFexLRJetRoI"},
    "Keys of the gFex large-R Jet RoI container to be linked to L1 Trigger Result"};

  // gFex Scalar E (JwoJ) RoIs -- carries gFex SumET (Y component); MET (X)
  // is overwritten by the decoder from sqrt(METx^2 + METy^2) computed from
  // m_gMETComponentsJwojKeys, so both keys must be linked together for the
  // h_gFexMet round-trip histogram to reproduce.
  SG::ReadHandleKeyArray<xAOD::gFexGlobalRoIContainer> m_gScalarEJwojKeys {
    this, "gScalarEJwojKeys", {},
    "Keys of the gFex Scalar E (JwoJ) RoI container to be linked to L1 Trigger Result"};

  // gFex MET Components (JwoJ) RoIs -- carries METx / METy partials
  SG::ReadHandleKeyArray<xAOD::gFexGlobalRoIContainer> m_gMETComponentsJwojKeys {
    this, "gMETComponentsJwojKeys", {},
    "Keys of the gFex MET Components (JwoJ) RoI container to be linked to L1 Trigger Result"};

  // Key of the cTau container to create (if empty, cTau creation is disabled)
  SG::WriteHandleKey<xAOD::eFexTauRoIContainer> m_cTauRoIKey {
    this, "cTauRoIKey", "L1_cTauRoI",
    "Key of the cTau RoI container to be created (contains copies of eTaus matched to jTaus)"};

  // Key of the cTau decoration linking eFexTau to the matching jFexTau
  SG::WriteDecorHandleKey<xAOD::eFexTauRoIContainer> m_cjTauLinkKey {
    this, "cjTauLinkKey", "L1_cTauRoI.jTauLink",
    "Decoration for the link from eTau to the matching jTau"};

  // Threshold pattern tools
  ToolHandleArray<IRoIThresholdsTool> m_thresholdPatternTools {
    this, "ThresholdPatternTools", {},
    "Tools decorating RoI containers with threshold patterns"
  };

  // Placeholder for other L1 xAOD outputs:
  // - CTP result
  // - L1Topo result
  // - the remaining Run-3 L1Calo RoIs

  // ------------------------- Helper methods ----------------------------------
  /// Create the combined Tau container matching eTau to jTau
  StatusCode createCombinedTauRoIs(xAOD::TrigComposite& l1tr, const EventContext& eventContext) const;
};

#endif // HLTSEEDING_L1TRIGGERRESULTMAKER_H
