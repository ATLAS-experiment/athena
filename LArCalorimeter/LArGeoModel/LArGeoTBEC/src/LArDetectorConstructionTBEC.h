/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// LArDetectorConstructionTBEC
// Return an envelope that contains the LAr End-Cap Test Bench.
// Dec-2005 V. Niess
// from CryostatConstructionH62003 and LArG4TBECCryostatConstruction.

#ifndef LARGEOTBEC_LARDETECTORCONSTRUCTIONTBEC_H
#define LARGEOTBEC_LARDETECTORCONSTRUCTIONTBEC_H

#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"

namespace LArGeo {

  class LArDetectorConstructionTBEC 
  {
  public:
    
    LArDetectorConstructionTBEC() = default;
    ~LArDetectorConstructionTBEC() = default;
    
    // Get the envelope containing this detector.
    PVLink GetEnvelope();
    
  private:
    void                getSimulationParameters();
    GeoIntrusivePtr<GeoFullPhysVol>     createEnvelope();
    
    double              m_eta_pos{0.};
    double              m_eta_cell{0.};
    double              m_phi_pos{0.};
    double              m_phi_cell{0.};
    
    bool                m_hasLeadCompensator{false};
    bool		m_hasPresampler{false};
    double              m_ModuleRotation{0.};
    double              m_YShift{0.};
    
    GeoPhysVol          *m_tbecEnvelopePhysical{nullptr};
  };
  
} // namespace LArGeo

#endif
