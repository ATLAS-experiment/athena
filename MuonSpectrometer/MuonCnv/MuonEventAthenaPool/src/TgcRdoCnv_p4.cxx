/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/TgcRdo.h"
#include "GaudiKernel/MsgStream.h"
#include "MuonEventAthenaPool/TgcRdo_p4.h"
#include "TgcRdoCnv_p4.h"
#include "TgcRawDataCnv_p4.h"
#include <bit>
#include <cstdint>

void
TgcRdoCnv_p4::transToPers(const TgcRdo* transColl, TgcRdo_p4* persColl, MsgStream &log) 
{
    if (log.level() <= MSG::DEBUG)
        log <<  MSG::DEBUG << " ***  Writing out TgcRdo P4" << endmsg;

    // Invoke vector converter from the base template
    TgcRdoCnv_p4_basetype::transToPers(transColl, persColl, log);

    persColl->m_version       = transColl->version();
    persColl->m_id            = transColl->identify();
    persColl->m_subDetectorId = transColl->subDetectorId();
    persColl->m_rodId         = transColl->rodId();
    persColl->m_triggerType   = transColl->triggerType();
    persColl->m_bcId          = transColl->bcId();
    persColl->m_l1Id          = transColl->l1Id();

    persColl->m_errors = std::bit_cast<uint32_t>(transColl->errors());

    persColl->m_rodStatus = std::bit_cast<uint32_t>(transColl->rodStatus());

    persColl->m_localStatus = std::bit_cast<uint32_t>(transColl->localStatus());


    persColl->m_orbit         = transColl->orbit();
}

void
TgcRdoCnv_p4::persToTrans(const TgcRdo_p4* persColl, TgcRdo* transColl, MsgStream &log)
{
    if (log.level() <= MSG::DEBUG)
        log << MSG::DEBUG  << " ***  Reading TgcRdo P4" << endmsg;

    // Invoke vector converter from the base template
    TgcRdoCnv_p4_basetype::persToTrans(persColl, transColl, log);

    transColl->setVersion      (persColl->m_version);
    transColl->setOnlineId(persColl->m_subDetectorId,
                           persColl->m_rodId);
    transColl->setTriggerType  (persColl->m_triggerType);
    transColl->setBcId         (persColl->m_bcId);
    transColl->setL1Id         (persColl->m_l1Id);
    transColl->setOrbit        (persColl->m_orbit);

    if(persColl->m_errors!=0) {transColl->setErrors       (persColl->m_errors);}
    transColl->setRodStatus    (persColl->m_rodStatus);
    transColl->setLocalStatus  (persColl->m_localStatus);
}

