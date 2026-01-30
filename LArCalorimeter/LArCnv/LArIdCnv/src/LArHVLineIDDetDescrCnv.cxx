/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArIdCnv/LArHVLineIDDetDescrCnv.h"

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h" 

#include "IdDictDetDescr/IdDictManager.h"
#include "LArIdentifier/LArHVLineID.h"


long int
LArHVLineIDDetDescrCnv::repSvcType() const
{
  return (storageType());
}

StatusCode 
LArHVLineIDDetDescrCnv::initialize()
{
    // First call parent init
    ATH_CHECK( DetDescrConverter::initialize() );
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

StatusCode
LArHVLineIDDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    ATH_MSG_INFO("in createObj: creating a LArHVLineID helper object in the detector store");

    // Get the dictionary manager from the detector store
    const IdDictManager* idDictMgr;
    ATH_CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

    // create the helper
    LArHVLineID* hvline_id = new LArHVLineID;

    ATH_CHECK( idDictMgr->initializeHelper(*hvline_id) == 0 );

    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(hvline_id);
    return StatusCode::SUCCESS; 

}

//--------------------------------------------------------------------

long 
LArHVLineIDDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID& 
LArHVLineIDDetDescrCnv::classID() { 
    return ClassID_traits<LArHVLineID>::ID(); 
}

//--------------------------------------------------------------------
LArHVLineIDDetDescrCnv::LArHVLineIDDetDescrCnv(ISvcLocator* svcloc) 
    :
    DetDescrConverter(ClassID_traits<LArHVLineID>::ID(), svcloc, "LArHVLineIDDetDescrCnv")
{}



