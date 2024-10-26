/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// CryostatConstructionTBEC
// Return an envelope that contains the TBEC LAr Cryostat.
// Dec-2005 V. Niess
// from CryostatConstructionH62003 and LArG4TBECCryostatConstruction.

#ifndef LARGEOTBEC_CRYOSTATCONSTRUCTIONTBEC_H
#define LARGEOTBEC_CRYOSTATCONSTRUCTIONTBEC_H

#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
namespace LArGeo {

  class CryostatConstructionTBEC 
  {
  public:
    CryostatConstructionTBEC() = default;
    ~CryostatConstructionTBEC() = default;
    
    // Get the envelope containing this detector.
    GeoIntrusivePtr<GeoVFullPhysVol> GetEnvelope();
    
    // Get the LAr physical volume.
    GeoIntrusivePtr<GeoPhysVol> GetLArPhysical();
    
  private:
    GeoFullPhysVol      *m_cryoEnvelopePhysical{nullptr};
    GeoPhysVol		*m_LArPhysical{nullptr};
  };
  
} // namespace LArGeo

#endif
