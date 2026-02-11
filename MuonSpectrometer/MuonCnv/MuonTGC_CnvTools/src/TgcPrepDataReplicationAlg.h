/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TgcPrepDataReplicationAlg_h
#define TgcPrepDataReplicationAlg_h

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonTGC_CnvTools/ITgcPrepDataReplicationTool.h"

namespace Muon {

class TgcPrepDataReplicationAlg : public AthReentrantAlgorithm {
   public:
    TgcPrepDataReplicationAlg(const std::string& name,
                              ISvcLocator* pSvcLocator);  // Constructor

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

   private:
    /** ITgcPrepDataReplicationTool.h */
    ToolHandle<ITgcPrepDataReplicationTool> m_tool{
        this, "Tool", "Muon::TgcPrepDataReplicationTool"};
};
}  // namespace Muon

#endif  // TgcPrepDataReplicationAlg_h
