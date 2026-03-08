/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/CscRawDataContainer.h"
#include "EventContainers/SelectAllObject.h"

// Default constructor.
CscRawDataContainer::CscRawDataContainer()
  : IdentifiableContainer<CscRawDataCollection>(hashFcn().max()) 
{
}

CscRawDataContainer::CscRawDataContainer(unsigned int hashmax)
  : IdentifiableContainer<CscRawDataCollection>(hashmax) 
{
}

CscRawDataContainer::CscRawDataContainer(CscRawDataCollection_Cache* cache)
  : IdentifiableContainer<CscRawDataCollection>(cache) 
{
}

// Destructor.
CscRawDataContainer::~CscRawDataContainer() 
= default;

/// Convert identifier to idhash
unsigned int 
CscRawDataContainer::idToHash(unsigned int id) 
{
    return (hashFcn()(id));
}

/// Convert identifier to idhash
const CscRawDataCollectionIdHash& 
CscRawDataContainer::hashFcn()
{

    static const CscRawDataCollectionIdHash hashFcn;
    return (hashFcn);
}

// Class ID
CLID CscRawDataContainer::classID()
{
  return ClassID_traits<CscRawDataContainer>::ID();       
}


const CLID& CscRawDataContainer::clID() const
{
  static const CLID cid = classID();
  return cid;
}


// Insert a RawData
void CscRawDataContainer::push_back(CscRawData * /*rawData*/)
{
}
