/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SiliconIDDetDescrCnv.h"

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h" 

#include "IdDictDetDescr/IdDictManager.h"
#include "InDetIdentifier/SiliconID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"


//--------------------------------------------------------------------

long int   
SiliconIDDetDescrCnv::repSvcType() const
{
  return (storageType());
}

//--------------------------------------------------------------------

StatusCode 
SiliconIDDetDescrCnv::initialize()
{
    ATH_CHECK( DetDescrConverter::initialize() );
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

StatusCode
SiliconIDDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    // Get the dictionary manager from the detector store
    const IdDictManager* idDictMgr = nullptr;
    ATH_CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

    // Get both Pixel and SCT id helpers
    const PixelID* pixelID = nullptr;
    ATH_CHECK( detStore()->retrieve(pixelID, "PixelID") );

    const SCT_ID* sctID = nullptr;
    ATH_CHECK( detStore()->retrieve(sctID, "SCT_ID") );

    if (!m_siliconId) {
	    // create the helper only once
	    ATH_MSG_DEBUG(" Create SiliconID. ");
	    m_siliconId = new SiliconID(pixelID, sctID);
    }
    
    ATH_CHECK( idDictMgr->initializeHelper(*m_siliconId) == 0 );

    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(m_siliconId);

    return StatusCode::SUCCESS; 

}

//--------------------------------------------------------------------

long
SiliconIDDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID& 
SiliconIDDetDescrCnv::classID() { 
    return ClassID_traits<SiliconID>::ID(); 
}

//--------------------------------------------------------------------
SiliconIDDetDescrCnv::SiliconIDDetDescrCnv(ISvcLocator* svcloc) 
    :
    DetDescrConverter(ClassID_traits<SiliconID>::ID(), svcloc, "SiliconIDDetDescrCnv"),
    m_siliconId(nullptr)

{}



