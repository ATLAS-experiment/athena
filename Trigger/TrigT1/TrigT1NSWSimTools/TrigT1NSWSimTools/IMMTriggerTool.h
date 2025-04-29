/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IMMTRIGGERTOOL_H
#define IMMTRIGGERTOOL_H 1

//basic includes
#include "GaudiKernel/IAlgTool.h"
#include "MuonRDO/NSW_TrigRawDataContainer.h"
#include "MuonTesterTree/MuonTesterTree.h"

// namespace for the NSW LVL1 related classes
namespace NSWL1 {

  class IMMTriggerTool: virtual public IAlgTool {

  public:
    DeclareInterfaceID(IMMTriggerTool, 1 ,0);
    virtual ~IMMTriggerTool() = default;

    virtual StatusCode attachBranches(MuonVal::MuonTesterTree &tree) = 0;
    virtual StatusCode runTrigger(const EventContext& ctx, Muon::NSW_TrigRawDataContainer* rdo, const bool do_MMDiamonds) const = 0;
  };
}
#endif
