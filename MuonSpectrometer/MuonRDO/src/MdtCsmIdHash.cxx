/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "MuonRDO/MdtCsmIdHash.h"

#include "MuonIdHelpers/MdtIdHelper.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/StatusCode.h"
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/StoreGateSvc.h"

#include "Identifier/IdentifierHash.h"

#include <cassert> 
#include <iostream> 

// This class converts a Mdt Identifier into a integer, according to its
// fields 


// default contructor 
MdtCsmIdHash::MdtCsmIdHash( )   
{
  const MdtIdHelper* mdtHelper=nullptr;
  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};

  StatusCode sc = detStore->retrieve( mdtHelper, "MDTIDHELPER" );
  if (sc.isFailure()) {
    throw GaudiException("can not get MdtIdHelper",
                         "MdtCsmIdHash::MdtCsmIdHash()", sc);
  }

  unsigned int used = mdtHelper->module_hash_max();
  IdContext context = mdtHelper->module_context();

  if( mdtHelper->stationNameIndex("BME") != -1 ) {
    used = mdtHelper->detectorElement_hash_max();
    context = mdtHelper->detectorElement_context();
  }

  for(unsigned int hash=0; hash < used; ++hash){
    Identifier id;
    if (!mdtHelper->get_id(hash,id,&context)) {
      m_lookup[id]=(int) hash;
      m_int2id.push_back(id); 
    } else {
      REPORT_MESSAGE_WITH_CONTEXT(MSG::ERROR, "MdtCscIdHash") <<
        "MDT hash constructor failed!" << endmsg;
    }
  } 

  m_size = (int) used;
}

MdtCsmIdHash::ID MdtCsmIdHash::identifier(int index) const {

  if(index>=0 && index < m_size) { 
   return m_int2id[index] ; 
  }
  // return blank id for invalid index 
  std::cout <<" MdtCsmIdHash::ERROR : identifier invalid ID "<<std::endl; 
  Identifier id;
  return id; 

}

int MdtCsmIdHash::operator() (const ID& id) const {

 std::map<ID,int>::const_iterator it = m_lookup.find(id); 
 if(it!=m_lookup.end()){
	return (*it).second; 
 }

 // FIXME  -1 for invalid ID?
 return -1; 

}

int MdtCsmIdHash::max() const {

return m_size; 

}

int MdtCsmIdHash::offset() {

return 0;

}
