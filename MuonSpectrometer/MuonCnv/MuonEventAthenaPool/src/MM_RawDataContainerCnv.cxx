/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MM_RawDataContainerCnv.h"
#include "StoreGate/StoreGateSvc.h"
#include "MuonIdHelpers/MmIdHelper.h"


MM_RawDataContainerCnv::MM_RawDataContainerCnv(ISvcLocator* svcloc) :
  MM_RawDataContainerCnvBase(svcloc, "MM_RawDataContainerCnv")
{
}

MM_RawDataContainerCnv::~MM_RawDataContainerCnv() = default;

StatusCode MM_RawDataContainerCnv::initialize() {
  // Call base clase initialize
  ATH_CHECK( MM_RawDataContainerCnvBase::initialize() );

  // Get the helper from the detector store
  const MmIdHelper *idHelper;
  ATH_CHECK( detStore()->retrieve(idHelper) );

  m_TPConverter_p1.initialize(idHelper);
  m_TPConverter_p2.initialize(idHelper);
  m_TPConverter_p3.initialize(idHelper);

  return StatusCode::SUCCESS;
}

MM_RawDataContainer_PERS*    MM_RawDataContainerCnv::createPersistent (Muon::MM_RawDataContainer* transCont) {
  return m_TPConverter_p3.createPersistent( transCont, msg() );
}

Muon::MM_RawDataContainer*
MM_RawDataContainerCnv::createTransient(const Token* token)
{
  using namespace Muon;

  MM_RawDataContainer *transCont = nullptr;
  static const pool::Guid	p1_guid("5F202045-CE2C-4AD4-96BA-7DA18053B90F");
  static const pool::Guid	p2_guid("A49EBDAC-A190-4198-95DF-BF75FBBB487F");
  static const pool::Guid	p3_guid("229DDB7E-59D3-4BE5-B3D5-B873EBC5C9AA");

  if( compareClassGuid(token, p1_guid) ) {
    std::unique_ptr< MM_RawDataContainer_p1 >  cont( this->poolReadObject<MM_RawDataContainer_p1>() );
    const MM_RawDataContainer_p1* constCont = cont.get();
    transCont =  m_TPConverter_p1.createTransient( constCont, msg() );
  }
  // ----------------------------------------------
  // p2 has the relBCID included
  else  if( compareClassGuid(token, p2_guid) ) {
    std::unique_ptr< MM_RawDataContainer_p2 >  cont( this->poolReadObject<MM_RawDataContainer_p2>() );
    const MM_RawDataContainer_p2* constCont = cont.get();
    transCont =  m_TPConverter_p2.createTransient( constCont, msg() );
  } 
  // ----------------------------------------------
  // p3 has timeAndChargeInCounts switch included
  else  if( compareClassGuid(token, p3_guid) ) {
    std::unique_ptr< MM_RawDataContainer_p3 >  cont( this->poolReadObject<MM_RawDataContainer_p3>() );
    const MM_RawDataContainer_p3* constCont = cont.get();
    transCont =  m_TPConverter_p3.createTransient( constCont, msg() );
  } 
  // ---------------------------------------------- 
  else {
    throw std::runtime_error("Unsupported persistent version of MM Raw Data (RDO) container");
  }
  return transCont;
}
