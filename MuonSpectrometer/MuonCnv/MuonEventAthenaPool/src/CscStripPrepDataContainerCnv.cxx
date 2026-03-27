/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CscStripPrepDataContainerCnv.h"

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"
#include "MuonPrepRawData/CscStripPrepDataContainer.h"

CscStripPrepDataContainerCnv::CscStripPrepDataContainerCnv(ISvcLocator* svcloc) :
  CscStripPrepDataContainerCnvBase(svcloc, "CscStripPrepDataContainerCnv")
{
}

CscStripPrepDataContainerCnv::~CscStripPrepDataContainerCnv() = default;

StatusCode CscStripPrepDataContainerCnv::initialize() {
    // Call base clase initialize
    ATH_CHECK( CscStripPrepDataContainerCnvBase::initialize() );

    return StatusCode::SUCCESS;
}

CscStripPrepDataContainer_PERS*    CscStripPrepDataContainerCnv::createPersistent (Muon::CscStripPrepDataContainer* transCont) {
    CscStripPrepDataContainer_PERS *pixdc_p= m_TPConverter.createPersistent( transCont, msg() );
    return pixdc_p;
}

Muon::CscStripPrepDataContainer* CscStripPrepDataContainerCnv::createTransient(const Token* token) {
    static const pool::Guid   p0_guid("A41C9D99-F977-43B5-8DFC-819F057A9136"); // before t/p split
    static const pool::Guid   p1_guid("6075244C-C6BB-4E24-B711-E7E4ED0F7462"); // with CscStripPrepData_tlp1

    Muon::CscStripPrepDataContainer* p_collection(nullptr);
    if( compareClassGuid(token, p1_guid) ) {
        std::unique_ptr< CscStripPrepDataContainer_PERS >  p_coll( poolReadObject< CscStripPrepDataContainer_PERS >(token) );
        p_collection = m_TPConverter.createTransient( p_coll.get(), msg() );
    }

    else if( compareClassGuid(token, p0_guid) ) {
        throw std::runtime_error("Not currently supporting reading non TP-split PRDs");
    }
    else {
        throw std::runtime_error("Unsupported persistent version of CscStripPrepDataContainer");

    }
    return p_collection;
}
