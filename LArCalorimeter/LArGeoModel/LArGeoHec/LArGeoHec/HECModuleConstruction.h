/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file HECModuleConstruction.h
 *
 * @brief Declaration of HECModuleConstruction class
 *
 */
#ifndef LARGEOHEC_HECMODULECONSTRUCTION_H
#define LARGEOHEC_HECMODULECONSTRUCTION_H

#include "GeoModelKernel/GeoFullPhysVol.h"


namespace LArGeo 
{
  /** @class LArGeo::HECModuleConstruction
      @brief GeoModel description of LAr HECModule

      The geometry is built and placed within HECModule envelope which is implemented
      by LArGeoEndcap.
   */
  class HECModuleConstruction 
    {
    public:
      HECModuleConstruction(bool threeBoards=false, bool frontWheel=true, bool tb=false, int tbyear=2002);
      ~HECModuleConstruction() = default;
      // Get the envelope containing this detector.
      GeoIntrusivePtr<GeoFullPhysVol> GetEnvelope();
      
    private:
      GeoIntrusivePtr<GeoFullPhysVol> m_physiHECModule;
      bool	      m_threeBoards;
      bool     	      m_frontWheel;
      bool	      m_tb;
      int 	      m_tbyear;
    };
  
}
#endif
