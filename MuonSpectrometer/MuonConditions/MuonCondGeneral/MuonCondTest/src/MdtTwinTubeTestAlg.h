/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MdtTwinTubeTestAlg_H
#define MdtTwinTubeTestAlg_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonCablingData/TwinTubeMap.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

namespace Muon{
    class MdtTwinTubeTestAlg: public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            StatusCode initialize() override  final;
            StatusCode execute(const EventContext& ctx) const;
        private:
            ServiceHandle<IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            SG::ReadCondHandleKey<TwinTubeMap> m_readKey{this, "CablingKey", "MdtTwinTubeMap", "Key of output Mdt TwinTube Map"};

            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
    };
}
#endif