/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtRDO_Decoder.h"

using namespace Muon;

MdtRDO_Decoder::MdtRDO_Decoder(const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type, name, parent) {
    declareInterface<IMDT_RDO_Decoder>(this);
}

StatusCode MdtRDO_Decoder::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_readKey.initialize());
    return StatusCode::SUCCESS;
}

std::unique_ptr<MdtDigit> MdtRDO_Decoder::getDigit(const EventContext&ctx,
                                                   const MdtAmtHit& amtHit, 
                                                   uint16_t subdetId, uint16_t mrodId, uint16_t csmId) const {
    SG::ReadCondHandle<MuonMDT_CablingMap> readHandle{m_readKey, ctx};
    const MuonMDT_CablingMap* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return nullptr;
    }
    MuonMDT_CablingMap::CablingData cabling_data{};
    cabling_data.tdcId = amtHit.tdcId();
    cabling_data.channelId = amtHit.channelId();
    cabling_data.subdetectorId = subdetId;
    cabling_data.mrod = mrodId;
    cabling_data.csm = csmId;

    uint16_t coarse = amtHit.coarse();
    uint16_t fine = amtHit.fine();
    int width = static_cast<int>(amtHit.width());

    if (!readCdo->getOfflineId(cabling_data, msgStream())) return nullptr;
    Identifier chanId;
    if (!readCdo->convert(cabling_data, chanId, false)) return nullptr;
    int tdcCounts = coarse * 32 + fine;
    return std::make_unique<MdtDigit>(chanId, tdcCounts, width, amtHit.isMasked());
}

