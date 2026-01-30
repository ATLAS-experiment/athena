/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 InDet DetDescrCnv package
 -----------------------------------------
 ***************************************************************************/

#include "AtlasIDDetDescrCnv.h"

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "AthenaKernel/StorableConversions.h"
#include "StoreGate/StoreGateSvc.h"

#include "IdDictDetDescr/IdDictManager.h"
#include "AtlasDetDescr/AtlasDetectorID.h"

//--------------------------------------------------------------------

long int   
AtlasIDDetDescrCnv::repSvcType() const
{
  return (storageType());
}

//--------------------------------------------------------------------

StatusCode 
AtlasIDDetDescrCnv::initialize()
{
    // First call parent init
    ATH_CHECK( DetDescrConverter::initialize() );

    // The following is an attempt to "bootstrap" the loading of a
    // proxy for AtlasDetectorID into the detector store. However,
    // AtlasIDDetDescrCnv::initialize is NOT called by the conversion
    // service.  So for the moment, this cannot be use. Instead the
    // DetDescrCnvSvc must do the bootstrap from a parameter list.


//      // Add InDet_DetDescrManager proxy as entry point to the detector store
//      // - this is ONLY needed for the manager of each system
//      sc = addToDetStore(classID(), "PidelID");
//      if (sc.isFailure()) {
//  	log << MSG::FATAL << "Unable to add proxy for AtlasDetectorID to the Detector Store!" << endmsg;
//  	return StatusCode::FAILURE;
//      } else {}

    return StatusCode::SUCCESS; 
}


//--------------------------------------------------------------------

StatusCode
AtlasIDDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    ATH_MSG_INFO("in createObj: creating a AtlasDetectorID helper object in the detector store");

    // Get the dictionary manager from the detector store
    const IdDictManager* idDictMgr;
    ATH_CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

    // create the helper
    auto atlas_id = std::make_unique<AtlasDetectorID>("IdDict", "");

    ATH_CHECK( idDictMgr->initializeHelper(*atlas_id) == 0 );

    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(std::move(atlas_id));

    return StatusCode::SUCCESS; 

}

//--------------------------------------------------------------------

long int
AtlasIDDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID& 
AtlasIDDetDescrCnv::classID() { 
    return ClassID_traits<AtlasDetectorID>::ID(); 
}

//--------------------------------------------------------------------
AtlasIDDetDescrCnv::AtlasIDDetDescrCnv(ISvcLocator* svcloc) 
  : DetDescrConverter(ClassID_traits<AtlasDetectorID>::ID(), svcloc, "AtlasIDDetDescrCnv")
{}



