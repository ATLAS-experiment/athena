/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTestR4/TesterModuleBase.h"
#include "GeoModelKernel/throwExcept.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/StoreGateSvc.h"

namespace MuonValR4 {
    TesterModuleBase::TesterModuleBase(MuonTesterTree& tree, 
                                       const std::string& grp_name, 
                                       MSG::Level msglvl) :
        MuonTesterBranch(tree, " R4 tester module " + grp_name) {
        setLevel(msglvl);
        m_idHelperSvc.retrieve().ignore();
    }
    const Muon::IMuonIdHelperSvc* TesterModuleBase::idHelperSvc() const { return m_idHelperSvc.get(); }
    const MuonGMR4::MuonDetectorManager* TesterModuleBase::getDetMgr() const {
        return m_detMgr;
    }
    const ActsTrk::GeometryContext& TesterModuleBase::getGeoCtx(const EventContext& ctx) const {
        SG::ReadHandle handle{m_geoCtxKey, ctx};
        if (!handle.isValid()) {
            THROW_EXCEPTION("Failed to retrieve "<<m_geoCtxKey.fullKey());
        }
        return *handle;
    }

    bool TesterModuleBase::init() {
        ServiceHandle<StoreGateSvc> detStore{"StoreGateSvc/DetectorStore", name()};
        if (!detStore.retrieve().isSuccess() || !detStore->retrieve(m_detMgr).isSuccess()) return false;       
        if (!declare_dependency(m_geoCtxKey)) return false;       
        if (!m_idHelperSvc.retrieve().isSuccess()) return false;
        return declare_keys();
    }
}  // namespace MuonPRDTest
