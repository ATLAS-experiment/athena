/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

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

  class TruthEDDecorator : public AthReentrantAlgorithm {

  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "EventInfoName", "EventInfo", "EventInfo key"};
    SG::ReadHandleKeyArray<xAOD::EventShape> m_eventShapeKeys {this, "EventShapeKeys", {}, "Truth EventShape keys"};
    SG::WriteDecorHandleKeyArray<xAOD::EventInfo> m_eventDensityDecorKeys {this, "EnergyDensityDecorKeys", m_eventInfoKey, {}, "Truth energy density decoration keys"};
  }; /// class

} /// namespace

#endif
