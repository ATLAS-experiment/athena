/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArIdCnv/LArOnline_SuperCellIDDetDescrCnv.h"

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h" 

#include "IdDictDetDescr/IdDictManager.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"


long int
LArOnline_SuperCellIDDetDescrCnv::repSvcType() const
{
  return (storageType());
}

StatusCode 
LArOnline_SuperCellIDDetDescrCnv::initialize()
{
    // First call parent init
    ATH_CHECK( DetDescrConverter::initialize() );
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

StatusCode
LArOnline_SuperCellIDDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    ATH_MSG_INFO("in createObj: creating a LArOnline_SuperCellID helper object in the detector store");

    // Get the dictionary manager from the detector store
    const IdDictManager* idDictMgr;
    ATH_CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

    // create the helper
    LArOnline_SuperCellID* online_id = new LArOnline_SuperCellID;

    ATH_CHECK( idDictMgr->initializeHelper(*online_id) == 0 );

    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(online_id);

    return StatusCode::SUCCESS; 

}

//--------------------------------------------------------------------

long 
LArOnline_SuperCellIDDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID& 
LArOnline_SuperCellIDDetDescrCnv::classID() { 
    return ClassID_traits<LArOnline_SuperCellID>::ID(); 
}

//--------------------------------------------------------------------
LArOnline_SuperCellIDDetDescrCnv::LArOnline_SuperCellIDDetDescrCnv(ISvcLocator* svcloc) 
    :
    DetDescrConverter(ClassID_traits<LArOnline_SuperCellID>::ID(), svcloc, "LArOnline_SuperCellIDDetDescrCnv")
{}



