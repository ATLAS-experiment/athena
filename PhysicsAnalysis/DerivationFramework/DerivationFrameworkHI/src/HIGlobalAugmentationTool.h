/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_HIGLOBALAUGMENTATIONTOOL_H
#define DERIVATIONFRAMEWORK_HIGLOBALAUGMENTATIONTOOL_H


#include<string>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AsgTools/ToolHandle.h"
#include <string>
#include <vector>
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODHIEvent/HIEventShapeContainer.h"

class IThinningSvc;

namespace DerivationFramework {

  class HIGlobalAugmentationTool : public extends<AthAlgTool, IAugmentationTool> {

  public:

    using base_class::base_class;

    // Athena algtool's Hooks
    virtual StatusCode  initialize() override final;
    virtual StatusCode  finalize() override final;

    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    Gaudi::Property<int> m_nHarmonic{this, "nHarmonic", 1, "Flow harmonic starting from v2"};
    Gaudi::Property<bool> m_doTopoClusDec{this, "doTopoClusDec", false, "Decorate with CaloTopoCluster FCal cut, non-HI mode only"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey { this, "EventInfoKey", "EventInfo", "" };
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TP_key{this, "InDetTrackParticlesKey", "InDetTrackParticles"};
    SG::ReadHandleKey<xAOD::HIEventShapeContainer> m_eventShapeKey{this, "HIEventShapeKey", "HIEventShape", ""};
    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_caloClusterKey{this, "CaloClusterKey", "", "Only needed if doTopoClusDec is true"};
    PublicToolHandleArray< InDet::IInDetTrackSelectionTool > m_trkSelTools{this, "TrackSelectionTools", {}, "Track selection tools (optional)"}; //!< track selection tool which can be optionally used for N_trk and sum pt cuts
    Gaudi::Property<std::vector<std::string>>  m_cutLevels{this, "cutLevels", {}, "Cut levels"};

    // Set up the decorators - TODO Should these be WriteDecorHandleKeys?
    std::vector< SG::AuxElement::Decorator< float >> m_decFCalEtA_Qnx;
    std::vector< SG::AuxElement::Decorator< float >> m_decFCalEtA_Qny;
    std::vector< SG::AuxElement::Decorator< float >> m_decFCalEtC_Qnx;
    std::vector< SG::AuxElement::Decorator< float >> m_decFCalEtC_Qny;

    std::vector< SG::AuxElement::Decorator< float >> m_decHalfFCalEtA_Qnx;
    std::vector< SG::AuxElement::Decorator< float >> m_decHalfFCalEtA_Qny;
    std::vector< SG::AuxElement::Decorator< float >> m_decHalfFCalEtC_Qnx;
    std::vector< SG::AuxElement::Decorator< float >> m_decHalfFCalEtC_Qny;

    std::vector< SG::AuxElement::Decorator< int >> m_decTrack_count;

  };

}


#endif
