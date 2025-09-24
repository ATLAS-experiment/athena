/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IPADEMULATORTOOL_H
#define IPADEMULATORTOOL_H 1

#include "GaudiKernel/IAlgTool.h"
#include "MuonRDO/NSW_PadTriggerDataContainer.h"
#include "MuonTesterTree/MuonTesterTree.h"

namespace NSWL1 {

  class IPadEmulatorTool : virtual public IAlgTool {

  public:
    DeclareInterfaceID(IPadEmulatorTool, 1 ,0);
    virtual ~IPadEmulatorTool() = default;

    virtual StatusCode attachBranches(MuonVal::MuonTesterTree &tree) = 0;
    virtual StatusCode emulate(const EventContext& ctx, Muon::NSW_PadTriggerDataContainer* out) const = 0;
  };
}
#endif
