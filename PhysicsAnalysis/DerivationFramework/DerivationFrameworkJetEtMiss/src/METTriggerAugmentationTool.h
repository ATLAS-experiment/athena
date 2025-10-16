/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef METTRIGGERAUGMENTATIONTOOL_H
#define METTRIGGERAUGMENTATIONTOOL_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTrigger/EnergySumRoI.h"
#include "xAODTrigger/JetRoIContainer.h"
#include <TH2.h>
#include <memory>

namespace DerivationFramework {

  class METTriggerAugmentationTool : public extends<AthAlgTool, IAugmentationTool> {
  public:
    METTriggerAugmentationTool(const std::string& t, const std::string& n, const IInterface* p);
    virtual StatusCode initialize();
    virtual StatusCode addBranches() const;
  private:
    SG::ReadHandleKey<xAOD::EnergySumRoI> m_L1METName{this, "L1METName", "LVL1EnergySumRoI"};
    SG::ReadHandleKey<xAOD::JetRoIContainer> m_L1JetName{this, "L1JetName", "LVL1JetRoIs"};
    SG::WriteHandleKey<xAOD::EnergySumRoI> m_outputName{this, "OutputName", "LVL1EnergySumRoI_KF"};
    Gaudi::Property<std::string> m_LUTFileName{this, "LUTFile", "LUT_data15.root"};
    std::unique_ptr<TH2> m_LUT{};
  };
}

#endif // METTRIGGERAUGMENTATIONTOOL_H
