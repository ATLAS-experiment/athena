/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCABLINGDATA_TgcCablingAlg_H
#define MUONCABLINGDATA_TgcCablingAlg_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "MuonCablingDataR4/TgcCablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include <nlohmann/json.hpp>

namespace MuonR4 {

class TgcCablingAlg : public AthCondAlgorithm {
public:
    using AthCondAlgorithm::AthCondAlgorithm;

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

private:
    StatusCode parsePayload(TgcCablingMap& cablingMap,
                            const nlohmann::json& payload) const;

    StatusCode findSLID(const nlohmann::json& stationBlock,
                        int stationEta,
                        int stationPhi,
                        int16_t& slid) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this,
        "MuonIdHelperSvc",
        "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
        "Muon ID helper service"
    };

    SG::WriteCondHandleKey<TgcCablingMap> m_writeKey{
        this,
        "WriteKey",
        "TgcCablingR4Map",
        "TGC cabling map condition object"
    };

    Gaudi::Property<std::string> m_jsonFile{
        this,
        "JSONFile",
        "",
        "Run-4 TGC cabling JSON file resolved through PathResolver"
    };
};

}  // namespace MuonR4

#endif