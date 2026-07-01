/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/NSW_MMTP_RawDataContainer.h"
#include <iostream>
#include "EventContainers/SelectAllObject.h"

Muon::NSW_MMTP_RawDataContainer::NSW_MMTP_RawDataContainer()
: IdentifiableContainer<NSW_MMTP_RawDataCollection>(nullptr)
{ }

Muon::NSW_MMTP_RawDataContainer::NSW_MMTP_RawDataContainer(unsigned int hashmax)
: IdentifiableContainer<NSW_MMTP_RawDataCollection>(hashmax)
{ }

CLID Muon::NSW_MMTP_RawDataContainer::classID()
{
  return ClassID_traits<NSW_MMTP_RawDataContainer>::ID();
}


const CLID& Muon::NSW_MMTP_RawDataContainer::clID() const
{
  static const CLID cid = classID();
  return cid;
}
