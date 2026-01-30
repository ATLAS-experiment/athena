/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 LAr DetDescrCnv package
 -----------------------------------------
 ***************************************************************************/


#include "LArIdCnv/LArElectrodeIDDetDescrCnv.h"

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h" 

#include "IdDictDetDescr/IdDictManager.h"
#include "LArIdentifier/LArElectrodeID.h"


long int
LArElectrodeIDDetDescrCnv::repSvcType() const
{
  return (storageType());
}

StatusCode 
LArElectrodeIDDetDescrCnv::initialize()
{
    // First call parent init
    ATH_CHECK( DetDescrConverter::initialize() );
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

StatusCode
LArElectrodeIDDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    ATH_MSG_INFO("in createObj: creating a LArElectrodeID helper object in the detector store");

    // Get the dictionary manager from the detector store
    const IdDictManager* idDictMgr;
    ATH_CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

    // create the helper
    LArElectrodeID* electrode_id = new LArElectrodeID;

    ATH_CHECK( idDictMgr->initializeHelper(*electrode_id) == 0 );

    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(electrode_id);
    return StatusCode::SUCCESS; 

}

//--------------------------------------------------------------------

long 
LArElectrodeIDDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID& 
LArElectrodeIDDetDescrCnv::classID() { 
    return ClassID_traits<LArElectrodeID>::ID(); 
}

//--------------------------------------------------------------------
LArElectrodeIDDetDescrCnv::LArElectrodeIDDetDescrCnv(ISvcLocator* svcloc) 
    :
    DetDescrConverter(ClassID_traits<LArElectrodeID>::ID(), svcloc, "LArElectrodeIDDetDescrCnv")
{}
