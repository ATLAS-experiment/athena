/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "STGC_RawDataContainerCnv.h"
#include "StoreGate/StoreGateSvc.h"
#include "MuonIdHelpers/sTgcIdHelper.h"

STGC_RawDataContainerCnv::STGC_RawDataContainerCnv(ISvcLocator* svcloc) :
  STGC_RawDataContainerCnvBase(svcloc, "STGC_RawDataContainerCnv")
{
}

STGC_RawDataContainerCnv::~STGC_RawDataContainerCnv() = default;

StatusCode STGC_RawDataContainerCnv::initialize() {
  // Call base clase initialize
  ATH_CHECK( STGC_RawDataContainerCnvBase::initialize() );

  // Get the helper from the detector store
  const sTgcIdHelper *idHelper;
  ATH_CHECK( detStore()->retrieve(idHelper) );
  m_TPConverter_p1.initialize(idHelper);
  m_TPConverter_p2.initialize(idHelper);
  m_TPConverter_p3.initialize(idHelper);

  return StatusCode::SUCCESS;
}

STGC_RawDataContainer_PERS*    STGC_RawDataContainerCnv::createPersistent (Muon::STGC_RawDataContainer* transCont) {
  return m_TPConverter_p3.createPersistent( transCont, msg() );
}

Muon::STGC_RawDataContainer*
STGC_RawDataContainerCnv::createTransient(const Token* token)
{
  using namespace Muon;

  STGC_RawDataContainer *transCont = nullptr;
  static const pool::Guid	p3_guid("693ACD72-6796-4251-A932-9ABAF679A2B3");
  static const pool::Guid	p2_guid("F66FDF31-1BFD-43DE-B793-93635D98597E");
  static const pool::Guid	p1_guid("E9229710-DB8A-447E-9546-4BAB079C7547");

  if( compareClassGuid(token, p3_guid) ) {
    std::unique_ptr< STGC_RawDataContainer_p3 >  cont( this->poolReadObject<STGC_RawDataContainer_p3>(token) );
    const STGC_RawDataContainer_p3* constCont = cont.get();
    transCont =  m_TPConverter_p3.createTransient( constCont, msg() );

  } else if( compareClassGuid(token, p2_guid) ) {
    std::unique_ptr< STGC_RawDataContainer_p2 >  cont( this->poolReadObject<STGC_RawDataContainer_p2>(token) );
    const STGC_RawDataContainer_p2* constCont = cont.get();
    transCont =  m_TPConverter_p2.createTransient( constCont, msg() );
    
  } else if( compareClassGuid(token, p1_guid) ) {
    std::unique_ptr< STGC_RawDataContainer_p1 >  cont( this->poolReadObject<STGC_RawDataContainer_p1>(token) );
    const STGC_RawDataContainer_p1* constCont = cont.get();
    transCont =  m_TPConverter_p1.createTransient( constCont, msg() );
    
  } else {
    throw std::runtime_error("Unsupported persistent version of STGC Raw Data (RDO) container");
  }
  return transCont;
}
