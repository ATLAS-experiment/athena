/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArIdCnv/LArOnlineIDDetDescrCnv.h"

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h" 

#include "IdDictDetDescr/IdDictManager.h"
#include "LArIdentifier/LArOnlineID.h"


long int
LArOnlineIDDetDescrCnv::repSvcType() const
{
  return (storageType());
}

StatusCode 
LArOnlineIDDetDescrCnv::initialize()
{
    // First call parent init
    ATH_CHECK( DetDescrConverter::initialize() );
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

StatusCode
LArOnlineIDDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    ATH_MSG_INFO("in createObj: creating a LArOnlineID helper object in the detector store");

    // Get the dictionary manager from the detector store
    const IdDictManager* idDictMgr;
    ATH_CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

    // create the helper
    LArOnlineID* online_id = new LArOnlineID;

    ATH_CHECK( idDictMgr->initializeHelper(*online_id) == 0 );

    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(online_id);

    return StatusCode::SUCCESS; 

}

//--------------------------------------------------------------------

long 
LArOnlineIDDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID& 
LArOnlineIDDetDescrCnv::classID() { 
    return ClassID_traits<LArOnlineID>::ID(); 
}

//--------------------------------------------------------------------
LArOnlineIDDetDescrCnv::LArOnlineIDDetDescrCnv(ISvcLocator* svcloc) 
    :
    DetDescrConverter(ClassID_traits<LArOnlineID>::ID(), svcloc, "LArOnlineIDDetDescrCnv")
{}



