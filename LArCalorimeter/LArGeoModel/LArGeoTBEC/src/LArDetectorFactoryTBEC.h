/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARGEOTBEC_LARDETECTORFACTORYTBEC_H
#define LARGEOTBEC_LARDETECTORFACTORYTBEC_H

#include "GeoModelKernel/GeoVDetectorFactory.h"
#include "LArReadoutGeometry/LArDetectorManager.h"

namespace LArGeo {

  class LArDetectorFactoryTBEC : public GeoVDetectorFactory
  {
    
  public:
    
    // Constructor:
    LArDetectorFactoryTBEC() = default;
  
    // Destructor:
    virtual ~LArDetectorFactoryTBEC() = default;
    
    // Illegal operations:
    const LArDetectorFactoryTBEC & operator=(const LArDetectorFactoryTBEC &right) = delete;
    LArDetectorFactoryTBEC(const LArDetectorFactoryTBEC &right) = delete;
  
    // Creation of geometry:
    virtual void create(GeoPhysVol* world) override;
    
    // Access to the results:
    virtual const LArDetectorManager* getDetectorManager() const override;
    
    // Set parameters:
    void setECVisLimit(int maxCell)       {m_ecVisLimit   = maxCell;}
    
  private:  
    
    // The manager:
    LArDetectorManager*       m_detectorManager{nullptr};
    int                       m_ecVisLimit{-1};

  };

} // namespace LArGeo

#endif 

