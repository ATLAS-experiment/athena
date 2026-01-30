/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_UNASSOCIATEDHITSDECORATOR_H
#define DERIVATIONFRAMEWORK_UNASSOCIATEDHITSDECORATOR_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthLinks/ElementLink.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"

#include "DerivationFrameworkInDet/MinBiasPRDAssociation.h"
#include "DerivationFrameworkInDet/IUnassociatedHitsGetterTool.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

namespace DerivationFramework {

  class UnassociatedHitsDecorator : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:

    Gaudi::Property<std::string>  m_sgName
    { this, "DecorationPrefix", "", ""};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "ContainerName", "EventInfo", ""};

    ToolHandle<IUnassociatedHitsGetterTool> m_UnassociatedHitsGetterTool
      { this, "UnassociatedHitsGetter", "" , ""};

    enum EIntDecor {knPixelUADecor,
      knBlayerUADecor,
      knPixelBarrelUADecor,
      knPixelEndCapAUADecor,
      knPixelEndCapCUADecor,
      knSCTUADecor,
      knSCTBarrelUADecor,
      knSCTEndCapAUADecor,
      knSCTEndCapCUADecor,
      knTRTUADecor,
      knTRTBarrelUADecor,
      knTRTEndCapAUADecor,
      knTRTEndCapCUADecor,
      kNIntDecor};
    std::vector<SG::WriteDecorHandleKey<xAOD::EventInfo> > m_intDecorKeys;
    // TODO
    //SG::WriteDecorHandleKeyArray<xAOD::EventInfo> m_intDecorKeys{this, "DecorationKeys", m_eventInfoKey, {} };

  };
}

#endif // DERIVATIONFRAMEWORK_UNASSOCIATEDHITSDECORATOR_H
