/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTGC_CABLING_TGCCABLINGCONDALG_H
#define MUONTGC_CABLING_TGCCABLINGCONDALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"
#include "StoreGate/WriteCondHandleKey.h"

namespace Muon {
class TgcCablingCondAlg : public AthCondAlgorithm {
   public:
    using AthCondAlgorithm::AthCondAlgorithm;
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

   private:
    SG::WriteCondHandleKey<TgcCablingMap> m_writeKey{this, "writeKey",
                                                     "MuonTgc_CablingMap"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    IntegerProperty m_AsideId{this, "AsideId", 103};
    IntegerProperty m_CsideId{this, "CsideId", 104};

    StringProperty m_databaseASDToPP{this, "databaseASDToPP",
                                     "MuonTGC_Cabling_ASD2PP.db"};
    StringProperty m_databaseInPP{this, "databaseInPP",
                                  "MuonTGC_Cabling_PP.db"};
    StringProperty m_databasePPToSL{this, "databasePPToSL",
                                    "MuonTGC_Cabling_PP2SL.db"};
    StringProperty m_databaseSLBToROD{this, "databaseSLBToROD",
                                      "MuonTGC_Cabling_SLB2ROD.db"};
    StringProperty m_databaseASDToPPdiff{this, "databaseASDtoPPdiff",
                                         "ASD2PP_diff_12_OFL.db"};
};
}  // namespace Muon

#endif