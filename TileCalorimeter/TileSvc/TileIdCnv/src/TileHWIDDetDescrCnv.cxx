/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TileIdCnv/TileHWIDDetDescrCnv.h"

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h" 

#include "IdDictDetDescr/IdDictManager.h"
#include "TileIdentifier/TileHWID.h"

StatusCode
TileHWIDDetDescrCnv::initialize()
{
    // First call parent init
    ATH_CHECK( DetDescrConverter::initialize() );
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

StatusCode
TileHWIDDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    ATH_MSG_INFO("in createObj: creating a TileHWID helper object in the detector store");

    // Get the dictionary manager from the detector store
    const IdDictManager* idDictMgr;
    ATH_CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

    // create the helper
    TileHWID* tilehw_id = new TileHWID;

    ATH_CHECK( idDictMgr->initializeHelper(*tilehw_id) == 0 );

    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(tilehw_id);

    return StatusCode::SUCCESS; 

}

//--------------------------------------------------------------------

long
TileHWIDDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

long
TileHWIDDetDescrCnv::repSvcType() const
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID& 
TileHWIDDetDescrCnv::classID() { 
    return ClassID_traits<TileHWID>::ID(); 
}

//--------------------------------------------------------------------
TileHWIDDetDescrCnv::TileHWIDDetDescrCnv(ISvcLocator* svcloc) 
    :
    DetDescrConverter(ClassID_traits<TileHWID>::ID(), svcloc, "TileHWIDDetDescrCnv")
{}

