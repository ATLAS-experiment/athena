/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LUCID_RawDataByteStreamCnv/LUCID_DigitByteStreamCnv.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "LUCID_RawEvent/LUCID_DigitContainer.h"
#include "AthenaKernel/ClassID_traits.h"
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "StoreGate/StoreGateSvc.h"
#include "LUCID_RawDataByteStreamCnv/LUCID_RodEncoder.h"

#include <cstdint>
#include <string>
#include <map>

typedef std::map<uint32_t, LUCID_RodEncoder> LucidRodEncoder_map;

LUCID_DigitByteStreamCnv::LUCID_DigitByteStreamCnv(ISvcLocator* svcloc) : 
  Converter(storageType(), classID(), svcloc),
  AthMessaging("LUCID_DigitByteStreamCnv"),
  m_ByteStreamEventAccess("ByteStreamCnvSvc", "LUCID_DigitByteStreamCnv"),
  m_RodBlockVersion      (0),
  m_BCs_per_LVL1ID       (1)
{

}

StatusCode LUCID_DigitByteStreamCnv::initialize() {

  ATH_CHECK( Converter::initialize() );
  ATH_CHECK( m_ByteStreamEventAccess.retrieve() );
    
  return StatusCode::SUCCESS;
}

CLID LUCID_DigitByteStreamCnv::classID() {

  return ClassID_traits<LUCID_DigitContainer>::ID();
}

long LUCID_DigitByteStreamCnv::storageType() {

  return ByteStreamAddress::storageType();
}

StatusCode LUCID_DigitByteStreamCnv::createRep(DataObject* pObj, IOpaqueAddress*& pAddr) {
  
   ATH_MSG_DEBUG(" LUCID_DigitByteStreamCnv::createRep");

   RawEventWrite*        re = m_ByteStreamEventAccess->getRawEvent();
   LUCID_DigitContainer* RDO_container = nullptr;

   // dynamic cast of the pObj to RDO_container based in clid of pObj
   if (!SG::fromStorable(pObj, RDO_container)) {
     ATH_MSG_ERROR("Can not cast to LUCID_DigitContainer");
     return StatusCode::RECOVERABLE;
   }

// get name of the persistent object (pObj)
   std::string nm = pObj->registry()->name();

// and create a empty generic BytestreamAddress  for the specific clid of pObj
   ByteStreamAddress* addr = new ByteStreamAddress(classID(), nm, "");

   pAddr = addr;

   StatusCode sc = fillFEA(RDO_container, re);
   
   if (sc.isFailure()){
     ATH_MSG_ERROR(" Could not convert RawData with to ByteStream ");
     return StatusCode::RECOVERABLE;
   }

  return StatusCode::SUCCESS;
}

StatusCode LUCID_DigitByteStreamCnv::fillFEA(LUCID_DigitContainer* RDO_container, RawEventWrite* re){

  m_fea.clear(); 
  // type of RODDATA is std::vector<uint32_t> 
  FullEventAssembler<SrcIdMap>::RODDATA* theROD;

  m_fea.setRodMinorVersion(RodBlockVersion());
  ATH_MSG_DEBUG("Setting ROD Minor Version Number to: " << m_RodBlockVersion);

  LucidRodEncoder_map RDOEncoder_map;

  LUCID_DigitContainer::const_iterator it_cont     = RDO_container->begin(); 
  LUCID_DigitContainer::const_iterator it_cont_end = RDO_container->end();
  LUCID_RodEncoder::Cache cache{};
  for( ; it_cont != it_cont_end; ++it_cont) {
    
    if ((*it_cont) != nullptr) {

      uint32_t rodId = getSourceID();
      
      RDOEncoder_map[rodId].addDigit((*it_cont),cache);
    } 
    else ATH_MSG_WARNING(" Digit is empty, skipping digit.");
  }

  LucidRodEncoder_map::iterator it_map     = RDOEncoder_map.begin();
  LucidRodEncoder_map::iterator it_map_end = RDOEncoder_map.end();
  
  for (; it_map != it_map_end; ++it_map) { 

    // (*it_map) is now a pointer of the type std::pair<const uint32_t, LUCID_RDOEncoder >
    
    theROD = m_fea.getRodData((*it_map).first);

    ((*it_map).second).encode(*theROD, cache, msg());

    (*theROD).push_back(0); // add status word
  
    LUCID_RawData lrd(*theROD);
    
    lrd.encodeLumatMapping();
    
    *theROD = lrd.getDataBlock();
  }
  
  m_fea.fill(re, msg(MSG::INFO)); 
  
  return StatusCode::SUCCESS;
}
