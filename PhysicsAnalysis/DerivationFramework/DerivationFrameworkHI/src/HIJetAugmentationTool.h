/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_HIJETAUGMENTATIONTOOL_H
#define DERIVATIONFRAMEWORK_HIJETAUGMENTATIONTOOL_H

#include <string>
#include <vector>

// Gaudi & Athena basics
#include "AsgTools/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetInterface/IJetUpdateJvt.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"

class IThinningSvc;

namespace DerivationFramework {

  class HIJetAugmentationTool : public extends<AthAlgTool, IAugmentationTool> {

  public:
    HIJetAugmentationTool(const std::string& t, const std::string& n, const IInterface* p);
    ~HIJetAugmentationTool();

    // Athena algtool's Hooks
    StatusCode  initialize();
    StatusCode  finalize();

    virtual StatusCode addBranches(const EventContext& ctx) const;

  private:

    Gaudi::Property<float> m_deltaR{
        this, "DeltaRJetMatching", 0.3,
        "Maximum distance in eta-phi between two matched jets"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey",
                                                      "EventInfo", ""};
    SG::ReadHandleKey<xAOD::JetContainer> m_hiJet_key{this, "HIJetContainerKey",
                                                      "AntiKt4HIJets"};
    SG::ReadHandleKey<xAOD::JetContainer> m_caloJet_key{
        this, "CaloJetContainerKey", "AntiKt4EMTopoJets"};

    // Tools
    PublicToolHandle<IJetUpdateJvt> m_jvtUpdateTool{
        this, "JVTToolEMTopo", "JetVertexTaggerTool",
        "JVT tool for EMTopo jets"}; //!< JVT update tool

    // Set up the decorators - TODO Should these be WriteDecorHandleKeys?
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_jvtMatchedKey{
        this, "JVTMatchedName", "JvtMatched",
        "SG Key for JVT AuxData"};

  };

}
#endif
