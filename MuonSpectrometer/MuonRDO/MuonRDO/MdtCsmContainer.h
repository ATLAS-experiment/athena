/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDO_MDTCSMCONTAINER_H
#define MUONRDO_MDTCSMCONTAINER_H


#include <vector>
#include <string>
#include "MuonRDO/MdtCsm.h"
#include "MuonRDO/MdtCsmIdHash.h"
#include "MuonRDO/MdtCsm_Cache.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "EventContainers/IdentifiableContainer.h" 


/** This container provides acces to the MDT RDOs
 @author Stefano Rosati, Mar 2003
*/
class MdtCsmContainer
   :public IdentifiableContainer<MdtCsm> {
public:  
   MdtCsmContainer() ; 
   MdtCsmContainer( unsigned int hashmax) ;
   MdtCsmContainer( MdtCsm_Cache* cache );
  ~MdtCsmContainer() ; 

  typedef MdtCsm::size_type size_type ; 
  typedef IdentifiableContainer<MdtCsm> MyBase; 

  size_type digit_size() const ; 

  /// Convert identifier to idhash
  static unsigned int idToHash(unsigned int id) ;

  // IdentifiableContainer is still a DataObject
  // Put CLID here. 
  static CLID classID();
  
  /** return class ID */
  virtual const CLID& clID() const override;
  
private:

    static const MdtCsmIdHash& hashFcn();
  
};

CLASS_DEF(MdtCsmContainer,4187,0) 

// Output stream.
std::ostream& operator<<(std::ostream& lhs, const MdtCsmContainer& rhs);


#endif     // MUONRDO_MDTCSMCONTAINER_H
