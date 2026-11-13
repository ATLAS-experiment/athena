/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/TgcL1Rdo.h"
#include "GaudiKernel/MsgStream.h"
#include "MuonEventAthenaPool/TgcL1Rdo_p1.h"
#include "TgcL1RdoCnv_p1.h"
#include "TgcL1RawDataCnv_p1.h"
#include <bit>
#include <cstdint>


void
TgcL1RdoCnv_p1::transToPers(const TgcL1Rdo* transColl, TgcL1Rdo_p1* persColl, MsgStream &log) 
{
    if (log.level() <= MSG::DEBUG)
        log <<  MSG::DEBUG << " ***  Writing out TgcL1Rdo P1" << endmsg;

    // Invoke vector converter from the base template
    TgcL1RdoCnv_p1_basetype::transToPers(transColl, persColl, log);

    persColl->m_version       = transColl->version();
    persColl->m_id            = transColl->identify();
    persColl->m_subDetectorId = transColl->subDetectorId();
    persColl->m_srodId        = transColl->srodId();
    persColl->m_triggerType   = transColl->triggerType();
    persColl->m_bcId          = transColl->bcId();
    persColl->m_l1Id          = transColl->l1Id();

    persColl->m_errors = std::bit_cast<uint32_t>(transColl->errors());

    persColl->m_srodStatus = std::bit_cast<uint32_t>(transColl->srodStatus());

    persColl->m_localStatus = std::bit_cast<uint32_t>(transColl->localStatus());

    persColl->m_orbit         = transColl->orbit();
}

void
TgcL1RdoCnv_p1::persToTrans(const TgcL1Rdo_p1* persColl, TgcL1Rdo* transColl, MsgStream &log)
{
    if (log.level() <= MSG::DEBUG)
        log << MSG::DEBUG  << " ***  Reading TgcL1Rdo P1" << endmsg;

    // Invoke vector converter from the base template
    TgcL1RdoCnv_p1_basetype::persToTrans(persColl, transColl, log);

    transColl->setVersion      (persColl->m_version);
    transColl->setOnlineId(persColl->m_subDetectorId,
                           persColl->m_srodId);
    transColl->setTriggerType  (persColl->m_triggerType);
    transColl->setBcId         (persColl->m_bcId);
    transColl->setL1Id         (persColl->m_l1Id);
    transColl->setOrbit        (persColl->m_orbit);

    if(persColl->m_errors!=0) {transColl->setErrors       (persColl->m_errors);}
    transColl->setSRodStatus    (persColl->m_srodStatus);
    transColl->setLocalStatus  (persColl->m_localStatus);
}

