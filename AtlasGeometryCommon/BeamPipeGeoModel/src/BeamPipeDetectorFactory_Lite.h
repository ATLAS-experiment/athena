/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BEAMPIPEGEOMODEL_BEAMPIPEDETECTORFACTORY_LITE_H
#define BEAMPIPEGEOMODEL_BEAMPIPEDETECTORFACTORY_LITE_H

#include "GeoModelKernel/GeoVDetectorFactory.h"
#include "BeamPipeGeoModel/BeamPipeDetectorManager.h"

class BeamPipeDetectorFactory_Lite final : public GeoVDetectorFactory  
{
 public:
  
  // Constructor:
  BeamPipeDetectorFactory_Lite();
  
  // Creation of geometry:
  virtual void create(GeoPhysVol *world) override;
  
  // Access to the results:
  virtual const BeamPipeDetectorManager * getDetectorManager() const override;
  
  // Illegal operations:
  const BeamPipeDetectorFactory_Lite & operator=(const BeamPipeDetectorFactory_Lite &right) = delete;
  BeamPipeDetectorFactory_Lite(const BeamPipeDetectorFactory_Lite &right) = delete;

 private:  
  // The manager:
  BeamPipeDetectorManager* m_manager{nullptr};
};

// Class BeamPipeDetectorFactory_Lite 
#endif


