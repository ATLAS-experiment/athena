/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDO_NSW_MMTP_RAWDATACONAINTER_H
#define MUONRDO_NSW_MMTP_RAWDATACONAINTER_H

#include "MuonRDO/NSW_MMTP_RawDataCollection.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "EventContainers/IdentifiableContainer.h"

namespace Muon
{
  class NSW_MMTP_RawDataContainer : public IdentifiableContainer<NSW_MMTP_RawDataCollection> {
  public:
    NSW_MMTP_RawDataContainer(unsigned int hashmax);
    NSW_MMTP_RawDataContainer();
    virtual ~NSW_MMTP_RawDataContainer() = default;

    static CLID classID();
    virtual const CLID& clID() const override;

  };
}

CLASS_DEF( Muon::NSW_MMTP_RawDataContainer , 456567098 , 1 )

#endif
