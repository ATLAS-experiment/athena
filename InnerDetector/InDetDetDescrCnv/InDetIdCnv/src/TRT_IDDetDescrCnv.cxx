/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_IDDetDescrCnv.h"

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h" 

#include "IdDictDetDescr/IdDictManager.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictMgr.h"
#include "InDetIdentifier/TRT_ID.h"


//--------------------------------------------------------------------

long int   
TRT_IDDetDescrCnv::repSvcType() const
{
  return (storageType());
}

//--------------------------------------------------------------------

StatusCode 
TRT_IDDetDescrCnv::initialize()
{
    ATH_CHECK( DetDescrConverter::initialize() );
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

StatusCode
TRT_IDDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    // Get the dictionary manager from the detector store
    const IdDictManager* idDictMgr;
    ATH_CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

    // Only initialize helper if it is the first pass or if there is a
    // change in the the file or tag
    bool initHelper               = false;

    const IdDictMgr* mgr          = idDictMgr->manager();

    // Internal InDet id tag
    const std::string  & inDetIDTag  = mgr->tag();

    // DoChecks flag
    bool doChecks                 = mgr->do_checks();

    IdDictDictionary* dict = mgr->find_dictionary("InnerDetector");  
    if (!dict) {
        ATH_MSG_ERROR("unable to find idDict for InnerDetector");
        return StatusCode::FAILURE;
    }

    // File to be read for InDet ids
    const std::string &  inDetIDFileName = dict->file_name();

    // Tag of RDB record for InDet ids
    const std::string &  inDetIdDictTag  = dict->dict_tag();


    if (m_trtId) {

	// TRT id helper already exists - second pass. Check for a
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
	m_trtId = new TRT_ID;
	initHelper = true;
        // add in message service for printout
        m_trtId->setMessageSvc(msgSvc());
    }
    
    if (initHelper) {
	if (idDictMgr->initializeHelper(*m_trtId)) {
	    ATH_MSG_WARNING("Unable to initialize TRT_ID");
	}
	// Save state:
	m_inDetIDTag      = inDetIDTag;
	m_inDetIDFileName = inDetIDFileName;
	m_inDetIdDictTag  = inDetIdDictTag;
	m_doChecks        = doChecks;
    }

    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(m_trtId);

    return StatusCode::SUCCESS; 

}

//--------------------------------------------------------------------

long
TRT_IDDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID& 
TRT_IDDetDescrCnv::classID() { 
    return ClassID_traits<TRT_ID>::ID(); 
}

//--------------------------------------------------------------------
TRT_IDDetDescrCnv::TRT_IDDetDescrCnv(ISvcLocator* svcloc) 
    :
    DetDescrConverter(ClassID_traits<TRT_ID>::ID(), svcloc, "TRT_IDDetDescrCnv"),
    m_trtId(nullptr),
    m_doChecks(false)
{}



