/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_IDDetDescrCnv.h"

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h" 

#include "IdDictDetDescr/IdDictManager.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictMgr.h"
#include "InDetIdentifier/SCT_ID.h"


//--------------------------------------------------------------------

long int   
SCT_IDDetDescrCnv::repSvcType() const
{
  return (storageType());
}

//--------------------------------------------------------------------

StatusCode 
SCT_IDDetDescrCnv::initialize()
{
    // First call parent init
    ATH_CHECK( DetDescrConverter::initialize() );
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

StatusCode
SCT_IDDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    // Get the dictionary manager from the detector store
    const IdDictManager* idDictMgr;
    ATH_CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

    // Only create new helper if it is the first pass or if there is a
    // change in the the file or tag
    bool initHelper               = false;

    const IdDictMgr* mgr          = idDictMgr->manager();

    // Internal InDet id tag
    const std::string &  inDetIDTag      = mgr->tag();

    // DoChecks flag
    bool doChecks                 = mgr->do_checks();

    const IdDictDictionary* dict = mgr->find_dictionary("InnerDetector");
    if (!dict) {
        ATH_MSG_ERROR("unable to find idDict for InnerDetector");
        return StatusCode::FAILURE;
    }

    // File to be read for InDet ids
    const std::string &  inDetIDFileName = dict->file_name();

    // Tag of RDB record for InDet ids
    const std::string &  inDetIdDictTag  = dict->dict_tag();


    if (m_sctId) {

	// SCT id helper already exists - second pass. Check for a
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
	m_sctId = new SCT_ID;
	initHelper = true;
        // add in message service for printout
        m_sctId->setMessageSvc(msgSvc());
    }
    
    if (initHelper) {
        ATH_CHECK( idDictMgr->initializeHelper(*m_sctId) == 0 );
        // Save state:
        m_inDetIDTag      = inDetIDTag;
        m_inDetIDFileName = inDetIDFileName;
        m_inDetIdDictTag  = inDetIdDictTag;
        m_doChecks        = doChecks;
    }

    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(m_sctId);

    return StatusCode::SUCCESS; 

}

//--------------------------------------------------------------------

long
SCT_IDDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID& 
SCT_IDDetDescrCnv::classID() { 
    return ClassID_traits<SCT_ID>::ID(); 
}

//--------------------------------------------------------------------
SCT_IDDetDescrCnv::SCT_IDDetDescrCnv(ISvcLocator* svcloc) 
    :
    DetDescrConverter(ClassID_traits<SCT_ID>::ID(), svcloc, "SCT_IDDetDescrCnv"),
    m_sctId(nullptr),
    m_doChecks(false)

{}



