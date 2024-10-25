/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file HEC2WheelConstruction.h
 *
 * @brief Declaration of HEC2WheelConstruction class
 *
 */

#ifndef LARGEOHEC_HEC2WHEELCONSTRUCTION_H
#define LARGEOHEC_HEC2WHEELCONSTRUCTION_H

#include "GeoModelKernel/GeoFullPhysVol.h"

namespace LArGeo 
{
  /** @class LArGeo::HEC2WheelConstruction
      @brief GeoModel description of LAr HEC

      The geometry is built and placed within HEC envelope which is implemented
      by LArGeoEndcap.
   */
  class HEC2WheelConstruction 
    {
    public:
      HEC2WheelConstruction() = default;
      ~HEC2WheelConstruction() = default;

      // Get the envelope containing this detector.
      GeoIntrusivePtr<GeoFullPhysVol> GetEnvelope(bool fullGeo, bool posZSide=true);
      
    private:
      GeoIntrusivePtr<GeoFullPhysVol> m_physiHEC{};
      bool	      m_posZSide{false};
    };
  
}
#endif
