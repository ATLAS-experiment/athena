/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "StoreGate/StoreGateSvc.h"

#include "MuonRDO/TgcRdoContainer.h"
#include "EventContainers/SelectAllObject.h"

// Default constructor.
TgcRdoContainer::TgcRdoContainer()
  : IdentifiableContainer<TgcRdo>(hashFcn().max()) 
{
}

// Default constructor.
TgcRdoContainer::TgcRdoContainer(unsigned int hashmax)
  : IdentifiableContainer<TgcRdo>(hashmax) 
{
}

TgcRdoContainer::TgcRdoContainer(TgcRdo_Cache* cache)
  : IdentifiableContainer<TgcRdo>(cache) 
{
}


// Destructor.
TgcRdoContainer::~TgcRdoContainer() 
= default;

/// Convert identifier to idhash
unsigned int 
TgcRdoContainer::idToHash(unsigned int id) 
{
    return (hashFcn()(id));
}

// Class ID
CLID TgcRdoContainer::classID()
{
  return ClassID_traits<TgcRdoContainer>::ID();       
}

const CLID& TgcRdoContainer::clID() const
{
  static const CLID cid = classID();
  return cid;
}

const TgcRdoIdHash&
TgcRdoContainer::hashFcn()
{
    static const TgcRdoIdHash hashFcn;
    return(hashFcn);
}


// Insert a RawData
void TgcRdoContainer::push_back(TgcRawData * /*rawData*/)
{
}
