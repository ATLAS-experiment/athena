#ifndef MUONCABLINGDATA_TgcCablingAlg_H
#define MUONCABLINGDATA_TgcCablingAlg_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCablingData/TgcCablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include <nlohmann/json.hpp>

namespace Muon {

class TgcCablingAlg : public AthReentrantAlgorithm {
public:
    TgcCablingAlg(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

private:
    StatusCode parsePayload(TgcCablingMap& cablingMap,
                            const nlohmann::json& payload) const;

    StatusCode findSLID(const nlohmann::json& stationBlock,
                        int stationEta,
                        int stationPhi,
                        int16_t& slid) const;

    ServiceHandle<IMuonIdHelperSvc> m_idHelperSvc{
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

    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyMap{
        this,
        "ReadKey",
        "/TGC/CABLING/MAP",
        "TGC cabling payload folder"
    };
};

}  // namespace Muon

#endif
