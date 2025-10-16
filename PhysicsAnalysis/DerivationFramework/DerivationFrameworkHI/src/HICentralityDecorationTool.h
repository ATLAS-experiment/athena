/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_HICENTRALITYDECORATIONTOOL_H
#define DERIVATIONFRAMEWORK_HICENTRALITYDECORATIONTOOL_H

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODEventInfo/EventInfo.h"
#include <vector>
#include <string>


namespace DerivationFramework {

  class HICentralityDecorationTool : public extends<AthAlgTool, IAugmentationTool> {

  public:
    HICentralityDecorationTool(const std::string& type, const std::string& name, const IInterface* parent);

    // Athena algtool's Hooks
    StatusCode  initialize() override final;

    virtual StatusCode addBranches(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey { this, "EventInfoKey", "EventInfo", "" };
    Gaudi::Property<std::string> m_centralityDefinitionFile{this, "centralityDefinitionFile", "HIEventUtils/HeavyIonAnalysis2015_centrality_cuts_Gv32_proposed.txt", "File for centrality definitions"};
    // Member variables to hold centrality definitions
    std::vector<float> m_centralityPercentiles;
    std::vector<float> m_fCalValues;
  };

}

#endif
