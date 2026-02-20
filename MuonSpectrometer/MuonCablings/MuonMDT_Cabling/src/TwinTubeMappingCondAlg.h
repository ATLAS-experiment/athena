/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONMDT_CABLING_TWINTUBEMAPPINGALG_H
#define MUONMDT_CABLING_TWINTUBEMAPPINGALG_H

#include <AthenaBaseComps/AthCondAlgorithm.h>
#include <AthenaPoolUtilities/CondAttrListCollection.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/WriteCondHandleKey.h>

#include "MuonCablingData/TwinTubeMap.h"




namespace Muon{
    class TwinTubeMappingCondAlg : public AthCondAlgorithm {
        public:
            using AthCondAlgorithm::AthCondAlgorithm;
            ~TwinTubeMappingCondAlg() = default;
            StatusCode initialize() override final;
            StatusCode execute(const EventContext& ctx) const override final;

        private:
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            SG::ReadCondHandleKey<CondAttrListCollection> m_readKey{this, "FolderName", ""};
            SG::WriteCondHandleKey<TwinTubeMap> m_writeKey{this, "WriteKey", "MdtTwinTubeMap", "Key of output Mdt TwinTube Map"};

            Gaudi::Property<std::string> m_extJSONFile{this, "JSONFile", "",
                                                        "Specify an external JSON file containing the cabling information."};

            Gaudi::Property<double> m_hvDelay{this, "HVDelayTime", 6.*Gaudi::Units::ns,
                                              "Delay time between twin tube and primary tube"};
    };
}
#endif
