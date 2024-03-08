/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcRDO_Decoder.h"

#include "MuonDigitContainer/RpcDigit.h"
#include "MuonRDO/RpcFiredChannel.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"

Muon::RpcRDO_Decoder::RpcRDO_Decoder(const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type, name, parent) {}

StatusCode Muon::RpcRDO_Decoder::initialize() {
    ATH_MSG_DEBUG("initialize");
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

std::vector<std::unique_ptr<RpcDigit>> Muon::RpcRDO_Decoder::getDigit(const RpcFiredChannel* fChan, uint16_t& sectorID, uint16_t& padId, uint16_t& cmaId,
                                                       const RpcCablingCondData* rpcCab) const {
    std::vector<std::unique_ptr<RpcDigit>> rpcDigitVec{};

    uint16_t side = (sectorID < 32) ? 0 : 1;
    uint16_t slogic = sectorID - side * 32;
    uint16_t ijk = fChan->ijk();
    uint16_t channel = fChan->channel();

    float time = (fChan->bcid() - m_bczero) * 25 + (fChan->time() + 0.5 - 2) * 3.125;

    // skip the trigger hits
    if (ijk == 7) { return rpcDigitVec; }

    // Get the list of offline channels corresponding to the
    // online identifier
    std::list<Identifier> idList = rpcCab->give_strip_id(side, slogic, padId, cmaId, ijk, channel, &m_idHelperSvc->rpcIdHelper());

    rpcDigitVec.reserve(idList.size());
    for (const Identifier& stripOfflineId: idList) {
        // and add the digit to the collection       
        std::unique_ptr<RpcDigit> rpcDigit = std::make_unique<RpcDigit>(stripOfflineId, time);
        rpcDigitVec.push_back(std::move(rpcDigit));
    }

    return rpcDigitVec;
}

std::vector<Identifier> Muon::RpcRDO_Decoder::getOfflineData(const RpcFiredChannel* fChan, uint16_t& sectorID, uint16_t& padId,
                                                              uint16_t& cmaId, double& time, const RpcCablingCondData* rpcCab) const {
    std::vector<Identifier> rpcIdVec{};

    uint16_t side = (sectorID < 32) ? 0 : 1;
    uint16_t slogic = sectorID - side * 32;
    uint16_t ijk = fChan->ijk();
    uint16_t channel = fChan->channel();

    time = (fChan->bcid() - m_bczero) * 25 + (fChan->time() + 0.5 - 2) * 3.125;

    // skip the trigger hits
    if (ijk == 7) { return rpcIdVec; }

    // Get the list of offline channels corresponding to the
    // online identifier
    std::list<Identifier> idList = rpcCab->give_strip_id(side, slogic, padId, cmaId, ijk, channel, &m_idHelperSvc->rpcIdHelper());

    rpcIdVec.assign(idList.begin(), idList.end());

    return rpcIdVec;
}
