/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/* Takashi Kubota - June 30, 2008 */
#include "TgcCoinDataContainerCnv.h"

// Gaudi
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"

// Athena
#include "StoreGate/StoreGateSvc.h"

// Id includes
#include "MuonIdHelpers/TgcIdHelper.h"
#include "MuonTrigCoinData/TgcCoinDataContainer.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"



TgcCoinDataContainerCnv::TgcCoinDataContainerCnv(ISvcLocator* svcloc) :
  TgcCoinDataContainerCnvBase(svcloc, "TgcCoinDataContainerCnv")
{
}

TgcCoinDataContainerCnv::~TgcCoinDataContainerCnv() = default;

StatusCode TgcCoinDataContainerCnv::initialize() {
    // Call base clase initialize
    ATH_CHECK( TgcCoinDataContainerCnvBase::initialize() );

    return StatusCode::SUCCESS;
}

TgcCoinDataContainer_PERS*    TgcCoinDataContainerCnv::createPersistent (Muon::TgcCoinDataContainer* transCont) {
    TgcCoinDataContainer_PERS *pixdc_p= m_TPConverter_tlp3.createPersistent( transCont, msg() );
    return pixdc_p;
}

Muon::TgcCoinDataContainer* TgcCoinDataContainerCnv::createTransient(const Token* token) {
    static const pool::Guid   p0_guid("F81C4564-B1C5-4053-A6F6-E0ED77907BE5"); // before t/p split
    static const pool::Guid   p1_guid("C312D3F5-60DB-41D5-895B-9FD4EF443E0B"); // with TgcCoinData_tlp1
    static const pool::Guid   p2_guid("524775D8-A66F-4AD3-912E-7D05389C1011"); // with TgcCoinData_tlp2
    static const pool::Guid   p3_guid("95BF89C7-1FFC-464F-A14D-742F9E874E56"); // with TgcCoinData_tlp3
    Muon::TgcCoinDataContainer* p_collection(nullptr);
    if( compareClassGuid(token, p3_guid) ) {
        poolReadObject< TgcCoinDataContainer_PERS >( m_TPConverter_tlp3 );
        p_collection = m_TPConverter_tlp3.createTransient( msg() );
    }
    else if( compareClassGuid(token, p2_guid) ) {
        poolReadObject< TgcCoinDataContainer_PERS >( m_TPConverter_tlp2 );
        p_collection = m_TPConverter_tlp2.createTransient( msg() );
    }
    else if( compareClassGuid(token, p1_guid) ) {
        poolReadObject< Muon::TgcCoinDataContainer_tlp1 >( m_TPConverter_tlp1 );
        p_collection = m_TPConverter_tlp1.createTransient( msg() );
    }
    else if( compareClassGuid(token, p0_guid) ) {
        throw std::runtime_error("Not currently supporting reading non TP-split CoinDatas");
    }
    else {
        throw std::runtime_error("Unsupported persistent version of TgcCoinDataContainer");

    }
    return p_collection;
}
