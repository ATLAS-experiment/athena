/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/MM_RawDataContainer.h"
#include <iostream>
#include "EventContainers/SelectAllObject.h"

Muon::MM_RawDataContainer::MM_RawDataContainer()
: IdentifiableContainer<MM_RawDataCollection>(nullptr) 
{
}

Muon::MM_RawDataContainer::MM_RawDataContainer(unsigned int hashmax)
: IdentifiableContainer<MM_RawDataCollection>(hashmax) 
{
}

Muon::MM_RawDataContainer::MM_RawDataContainer(MM_RawDataCollection_Cache* cache)
: IdentifiableContainer<MM_RawDataCollection>(cache) 
{
}

// Destructor.
Muon::MM_RawDataContainer::~MM_RawDataContainer() = default;

CLID Muon::MM_RawDataContainer::classID()
{
  return ClassID_traits<MM_RawDataContainer>::ID();       
}


const CLID& Muon::MM_RawDataContainer::clID() const
{
  static const CLID cid = classID();
  return cid;
}


// Output stream.
std::ostream& operator<<(std::ostream& lhs, const Muon::MM_RawDataContainer& rhs) {
  lhs << "MM_RawDataContainer has " << rhs.size() << " collections:" << std::endl;
  for (const auto *col : rhs ){
    lhs << "Collection with hash ["<<col->identifierHash()<<"] : " << std::endl;
    for (const auto *rdo : *col ){
      lhs << *rdo;
    }
  }
  return lhs;
}

