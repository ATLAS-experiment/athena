/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Calo Det Descr converter package
 -----------------------------------------
 ***************************************************************************/

#include "CaloIdCnv/CaloIdMgrDetDescrCnv.h"
#include "CaloIdentifier/CaloIdManager.h"


// infrastructure includes
#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/errorcheck.h"

// Identifier includes
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "CaloIdentifier/CaloDM_ID.h"
#include "CaloIdentifier/CaloLVL1_ID.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArMiniFCAL_ID.h"
#include "CaloIdentifier/TileID.h"
#include "CaloIdentifier/Tile_SuperCell_ID.h"
#include "CaloIdentifier/TTOnlineID.h"
#include "CaloIdentifier/LArEM_SuperCell_ID.h"
#include "CaloIdentifier/LArHEC_SuperCell_ID.h"
#include "CaloIdentifier/LArFCAL_SuperCell_ID.h"


long int
CaloIdMgrDetDescrCnv::repSvcType() const
{
  return (storageType());
}

StatusCode
CaloIdMgrDetDescrCnv::initialize()
{
    // First call parent init
    ATH_CHECK( DetDescrConverter::initialize() );
    return StatusCode::SUCCESS;
}


//--------------------------------------------------------------------


namespace {


template <class T>
StatusCode set_helper (const ServiceHandle<StoreGateSvc>& detStore,
                       const char* sgkey,
                       CaloIdManager* mgr,
                       MsgStream& log)
{
  const T* id = 0;
  CHECK_WITH_CONTEXT( detStore->retrieve (id, sgkey), "CaloIdMgrDetDescrCnv" );
  mgr->set_helper (id);
  log << MSG::DEBUG << "Found Calo ID helper " << sgkey << endmsg;
  return StatusCode::SUCCESS;
}


} // anonymous namespace

StatusCode
CaloIdMgrDetDescrCnv::createObj(IOpaqueAddress* /*pAddr*/, DataObject*& pObj)
{
    ATH_MSG_INFO("in createObj: creating a CaloDescrManager object in the detector store");

    // Create the manager
    CaloIdManager* caloIdMgr = new CaloIdManager();

    // Pass a pointer to the container to the Persistency service by reference.
    pObj = SG::asStorable(caloIdMgr);

    // Initialize manager with all helpers.
#define SET_HELPER(H) CHECK( set_helper<H> (detStore(), #H, caloIdMgr, msg()) )

    SET_HELPER (CaloCell_ID);

    bool is_test_beam = false;
    std::vector<std::string> file_names =
      caloIdMgr->getCaloCell_ID()->file_names();
    for (size_t i = 0; i < file_names.size(); i++) {
      if (file_names[i].find ("_H8_") != std::string::npos) {
        is_test_beam =true;
        break;
      }
    }
      
    SET_HELPER (CaloDM_ID);
    SET_HELPER (CaloLVL1_ID);
    SET_HELPER (LArEM_ID);
    SET_HELPER (LArHEC_ID);
    SET_HELPER (LArFCAL_ID);
    SET_HELPER (LArMiniFCAL_ID);
    SET_HELPER (TileID);
    SET_HELPER (TTOnlineID);

    if (!is_test_beam) {
      SET_HELPER (CaloCell_SuperCell_ID);
      SET_HELPER (LArEM_SuperCell_ID);
      SET_HELPER (LArHEC_SuperCell_ID);
      SET_HELPER (LArFCAL_SuperCell_ID);
      SET_HELPER (Tile_SuperCell_ID);
    }

#undef SET_HELPER

    // Initialize the calo mgr
    //  We protect here in case this has been initialized elsewhere
    if (!caloIdMgr->isInitialized()) {

      ATH_MSG_DEBUG("Initializing CaloIdMgr from values in CaloIdMgrDetDescrCnv");

      // Initialize the manager ...

      // in this one, only strictly calo work is done
      caloIdMgr->initialize();

    }


    return StatusCode::SUCCESS;

}

//--------------------------------------------------------------------

long
CaloIdMgrDetDescrCnv::storageType()
{
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
CLID
CaloIdMgrDetDescrCnv::classID() {
    return ClassID_traits<CaloIdManager>::ID();
}

//--------------------------------------------------------------------
CaloIdMgrDetDescrCnv::CaloIdMgrDetDescrCnv(ISvcLocator* svcloc)
    :
    DetDescrConverter(ClassID_traits<CaloIdManager>::ID(), svcloc, "CaloIdMgrDetDescrCnv")
{}
