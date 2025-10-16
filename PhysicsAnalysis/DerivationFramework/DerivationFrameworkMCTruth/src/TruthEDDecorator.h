/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @file TruthEDDecorator.h
 * @author Zach Marshall
 * @date Nov 2019
 * @brief tool to decorate EventInfo with truth-level energy density
*/

#ifndef DerivationFrameworkMCTruth_TruthEDDecorator_H
#define DerivationFrameworkMCTruth_TruthEDDecorator_H

// Base classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

// Members
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadHandleKeyArray.h"
#include "AsgDataHandles/WriteDecorHandleKeyArray.h"
#include "xAODEventShape/EventShape.h"
#include "xAODEventInfo/EventInfo.h"

// STL includes
#include <string>
#include <vector>

namespace DerivationFramework {

  class TruthEDDecorator : public extends<AthAlgTool, IAugmentationTool> {

  public:
    TruthEDDecorator(const std::string& t, const std::string& n, const IInterface* p);
    ~TruthEDDecorator();
    virtual StatusCode addBranches(const EventContext& ctx) const override final;
    StatusCode initialize() override final;

  private:

    Gaudi::Property<std::string> m_ed_suffix {this, "DecorationSuffix", "_rho"};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "EventInfoName", "EventInfo", "EventInfo key"};
    SG::ReadHandleKeyArray<xAOD::EventShape> m_eventShapeKeys {this, "EventShapeKeys", {}, "Truth EventShape keys"};    
    SG::WriteDecorHandleKeyArray<xAOD::EventInfo> m_eventDensityDecorKeys {this, "EnergyDensityDecorKeys", {}, "Truth energy density decoration keys"};
  }; /// class

} /// namespace

#endif
