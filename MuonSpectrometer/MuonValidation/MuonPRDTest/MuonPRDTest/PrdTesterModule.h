/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTER_PrdTesterModule_H
#define PRDTESTER_PrdTesterModule_H

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"
#include "StoreGate/ReadCondHandleKey.h"
namespace MuonPRDTest {
    using namespace MuonVal;
    
    class PrdTesterModule : public MuonTesterBranch {
    public:
        PrdTesterModule(MuonTesterTree& tree, const std::string& grp_name, bool useCondGeo, MSG::Level msglvl);

        virtual ~PrdTesterModule() = default;

        bool init() override final;

    protected:
        const Muon::IMuonIdHelperSvc* idHelperSvc() const;
        const MuonGM::MuonDetectorManager* getDetMgr(const EventContext& ctx) const;
        virtual bool declare_keys() = 0;

    private:
        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{"MuonDetectorManager"};
        ServiceHandle<StoreGateSvc> m_detStore{"StoreGateSvc/DetectorStore", name()};
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{"Muon::MuonIdHelperSvc/MuonIdHelperSvc", name()};
        const MuonGM::MuonDetectorManager* m_detMgr{nullptr};
        bool m_useCondDetMgr{false};
    };
}  // namespace MuonPRDTest
#endif
