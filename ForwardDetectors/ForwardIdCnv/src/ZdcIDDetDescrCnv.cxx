/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 ForwardIdCnv package
 -----------------------------------------
 ***************************************************************************/

#include "ZdcIDDetDescrCnv.h"

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/MsgStream.h"

#include "IdDictDetDescr/IdDictManager.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictMgr.h"
#include "ZdcIdentifier/ZdcID.h"


//--------------------------------------------------------------------

long int   
ZdcIDDetDescrCnv::repSvcType() const
{
  return (storageType());
}

//--------------------------------------------------------------------

StatusCode 
ZdcIDDetDescrCnv::initialize()
{
    ATH_CHECK( DetDescrConverter::initialize() );
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

StatusCode
ZdcIDDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    // Get the dictionary manager from the detector store
    const IdDictManager* idDictMgr = nullptr;
    ATH_CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

    // Only initialize helper if it is the first pass or if there is a
    // change in the the file or tag
    bool initHelper              = false;

    const IdDictMgr* mgr          = idDictMgr->manager();

    // Internal InDet id tag
    std::string   inDetIDTag      = mgr->tag();

    // DoChecks flag
    bool doChecks                 = mgr->do_checks();

    IdDictDictionary* dict = mgr->find_dictionary("InnerDetector");  
    if (!dict) {
	ATH_MSG_ERROR("unable to find idDict for InnerDetector");
	return StatusCode::FAILURE;
    }

    // File to be read for InDet ids
    std::string   inDetIDFileName = dict->file_name();

    // Tag of RDB record for InDet ids
    std::string   inDetIdDictTag  = dict->dict_tag();


    if (m_zdcId) {

	// Zdc id helper already exists - second pass. Check for a
	// change 
	if (inDetIDTag != m_inDetIDTag) { 
	    // Internal InDet id tag
	    initHelper = true;
	    ATH_MSG_DEBUG(" Changed internal InDet id tag: " << inDetIDTag);
	}
	if (inDetIDFileName != m_inDetIDFileName) {
	    // File to be read for InDet ids
	    initHelper = true;
	    ATH_MSG_DEBUG(" Changed InDetFileName:" << inDetIDFileName);
	}
	if (inDetIdDictTag != m_inDetIdDictTag) {
	    // Tag of RDB record for InDet ids
	    initHelper = true;
	    ATH_MSG_DEBUG(" Changed InDetIdDictTag: " << inDetIdDictTag);
	}
	if (doChecks != m_doChecks) {
	    // DoChecks flag
	    initHelper = true;
	    ATH_MSG_DEBUG(" Changed doChecks flag: " << doChecks);
        }
    }
    else {
	// create the helper
	m_zdcId = new ZdcID;
	initHelper = true;
        // add in message service for printout
        m_zdcId->setMessageSvc(msgSvc());
    }
    
    if (initHelper) {
        ATH_CHECK( idDictMgr->initializeHelper(*m_zdcId) == 0 );

        // Save state:
        m_inDetIDTag      = inDetIDTag;
        m_inDetIDFileName = inDetIDFileName;
        m_inDetIdDictTag  = inDetIdDictTag;
        m_doChecks        = doChecks;
    }
    
    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(m_zdcId);

    return StatusCode::SUCCESS; 

}

//--------------------------------------------------------------------

long
ZdcIDDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID& 
ZdcIDDetDescrCnv::classID() { 
    return ClassID_traits<ZdcID>::ID(); 
}

//--------------------------------------------------------------------
ZdcIDDetDescrCnv::ZdcIDDetDescrCnv(ISvcLocator* svcloc) 
    :
    DetDescrConverter(ClassID_traits<ZdcID>::ID(), svcloc, "ZdcIDDetDescrCnv"),
    m_zdcId(0),
    m_doChecks(false)

{}



