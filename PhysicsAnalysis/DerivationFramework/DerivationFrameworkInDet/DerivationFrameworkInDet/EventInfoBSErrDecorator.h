/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_EVENTINFOBSERRDECORATOR_H
#define DERIVATIONFRAMEWORK_EVENTINFOBSERRDECORATOR_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthLinks/ElementLink.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"
#include "StoreGate/ReadHandleKey.h"

#include "SCT_ConditionsTools/ISCT_ByteStreamErrorsTool.h"
#include "SCT_Cabling/ISCT_CablingTool.h"
#include "SCT_Cabling/SCT_OnlineId.h"

class AtlasDetectorID;
class SCT_ID;

namespace DerivationFramework {

  class EventInfoBSErrDecorator : public AthReentrantAlgorithm {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:

    Gaudi::Property<std::string> m_prefix
    { this,"DecorationPrefix", "", "" };
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "ContainerName", "EventInfo", ""};

    const SCT_ID*          m_sctId{};

    ToolHandle<ISCT_ByteStreamErrorsTool> m_byteStreamErrTool{this, "ByteStreamErrTool", "SCT_ByteStreamErrorsTool", "Tool to retrieve SCT ByteStream Errors"};
    ToolHandle<ISCT_CablingTool> m_cabling{this, "SCT_CablingTool", "SCT_CablingTool", "Tool to retrieve SCT Cabling"};

    enum EIntDecor {kSCT_BSErr_Ntot,
      kSCT_BSErr_bec,
      kSCT_BSErr_layer,
      kSCT_BSErr_eta,
      kSCT_BSErr_phi,
      kSCT_BSErr_side,
      kSCT_BSErr_rodid,
      kSCT_BSErr_channel,
      kSCT_BSErr_type,
      kNIntDecor};

    std::vector<SG::WriteDecorHandleKey<xAOD::EventInfo> > m_intDecorKeys;

  };
}

#endif // DERIVATIONFRAMEWORK_EVENTINFOBSERRDECORATOR_H
