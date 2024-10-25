/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// HECWheelConstruction.h
//
// Construct an Atlas HEC Wheel (Front or Rear)
// 
// Oct 2007 M. Fincke
//

#ifndef LARGEOHEC_HECWHEELCONSTRUCTION_H
#define LARGEOHEC_HECWHEELCONSTRUCTION_H

#include <string>
#include "GeoModelKernel/GeoFullPhysVol.h"


namespace LArGeo 
{

  class HECWheelConstruction 
    {
    public:
      HECWheelConstruction(bool fullGeo, const std::string& wheelType="front", bool threeBoards=false, bool posZSide=true);
      virtual ~HECWheelConstruction();

      // Get the envelope containing this detector.
      GeoIntrusivePtr<GeoFullPhysVol> GetEnvelope();
      
    private:
      bool	      m_posZSide;
      bool	      m_threeBoards;
      bool	      m_frontWheel;
      std::string     m_wheelType;
      bool            m_fullGeo;  // true->FULL, false->RECO
    };
  
}
#endif // LARGEOHEC_HECWHEELCONSTRUCTION_H
