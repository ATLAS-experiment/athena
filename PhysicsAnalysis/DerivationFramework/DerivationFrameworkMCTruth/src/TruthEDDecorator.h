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
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODEventShape/EventShape.h"
#include "xAODEventInfo/EventInfo.h"

// STL includes
#include <string>
#include <vector>

namespace DerivationFramework {

  class TruthEDDecorator : public extends<AthAlgTool, IAugmentationTool> {

  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "EventInfoName", "EventInfo", "EventInfo key"};
    SG::ReadHandleKeyArray<xAOD::EventShape> m_eventShapeKeys {this, "EventShapeKeys", {}, "Truth EventShape keys"};
    SG::WriteDecorHandleKeyArray<xAOD::EventInfo> m_eventDensityDecorKeys {this, "EnergyDensityDecorKeys", m_eventInfoKey, {}, "Truth energy density decoration keys"};
  }; /// class

} /// namespace

#endif
