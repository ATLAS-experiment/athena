/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonPRDTest/PrdTesterModule.h>
#include <StoreGate/ReadCondHandle.h>
namespace MuonPRDTest {
    PrdTesterModule::PrdTesterModule(MuonTesterTree& tree, 
                                    const std::string& grp_name, 
                                    MSG::Level msglvl) :
        MuonTesterBranch(tree, " prd module " + grp_name) {
        setLevel(msglvl);
        m_idHelperSvc.retrieve().ignore();
    }
    const Muon::IMuonIdHelperSvc* PrdTesterModule::idHelperSvc() const { return m_idHelperSvc.get(); }
    const MuonGM::MuonDetectorManager* PrdTesterModule::getDetMgr(const EventContext& ctx) const {
        SG::ReadCondHandle handle{m_detMgrKey, ctx};
        if (!handle.isValid()) {
            ATH_MSG_ERROR("Failed to retrieve MuonDetectorManager " << m_detMgrKey.fullKey());
            return nullptr;
        }
        return handle.cptr();
    }
    bool PrdTesterModule::init() {
        return declare_dependency(m_detMgrKey) && 
               m_idHelperSvc.retrieve().isSuccess() && declare_keys();
    }
    bool PrdTesterModule::declare_keys() { return true; }
}  // namespace MuonPRDTest