/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CaloIdCnv/src/CaloIDHelper_IDDetDescrCnv.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2012
 * @brief Common code for creating calo ID helpers in the detector store.
 */


#include "CaloIdCnv/CaloIDHelper_IDDetDescrCnv.h"
#include "IdDictDetDescr/IdDictManager.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/ClassID_traits.h"
#include "Identifier/IdHelper.h"
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/MsgStream.h"


namespace CaloIdCnv {


/**
 * @brief Called by the converter infrastructure to create an object.
 * @param pAddr Address of the object to create.
 * @param pObj[out] Set to a reference to the created helper.
 */
StatusCode CaloIDHelper_IDDetDescrCnv::createObj (IOpaqueAddress* pAddr,
                                                  DataObject*& pObj)
{
  // Get the a name of the class we're converting.
  std::string type_name;
  SmartIF<IClassIDSvc> clidsvc{ service ("ClassIDSvc") };
  CHECK( clidsvc.isValid() );
  CHECK( clidsvc->getTypeNameOfID (objType(), type_name) );

  // Get the SG key.
  DetDescrAddress* ddAddr = dynamic_cast<DetDescrAddress*> (pAddr);
  if (!ddAddr) {
    ATH_MSG_ERROR ("Dynamic cast to DetDescrAddress fails!");
    return StatusCode::FAILURE;
  }
  std::string helperKey  = *( ddAddr->par() );
  if (helperKey.empty()) {
    ATH_MSG_DEBUG("No Helper key ");
  }
  else {
    ATH_MSG_DEBUG("Helper key is " << helperKey);
  }

  // Get the dictionary manager from the detector store
  const IdDictManager* idDictMgr = 0;
  CHECK( detStore()->retrieve(idDictMgr, "IdDict") );

  // Create the helper.
  IdHelper* idhelper = 0;
  CHECK( createHelper (helperKey, idhelper, pObj) );

  // Initialize the helper.
  idhelper->setMessageSvc (msgSvc());
  if (idDictMgr->initializeHelper(*idhelper)) {
    ATH_MSG_ERROR("Unable to initialize " << type_name);
    return StatusCode::FAILURE;
  } 
  else {
    ATH_MSG_DEBUG("Initialized " << type_name);
  }

  return StatusCode::SUCCESS; 
}


/**
 * @brief Return the service type.  Required by the base class.
 */
long int CaloIDHelper_IDDetDescrCnv::repSvcType() const
{
  return storageType();
}


/**
 * @brief Constructor.
 * @param clid The CLID if the class we're constructing.
 * @param svcloc Gaudi service locator.
 */
CaloIDHelper_IDDetDescrCnv::CaloIDHelper_IDDetDescrCnv (const CLID& clid,
                                                        ISvcLocator* svcloc)
  : DetDescrConverter (clid, svcloc, "CaloIDHelper_IDDetDescrCnv")
{
}


} // namespace CaloIdCnv
