/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSERVMATGEOMODEL_INDETSERVMATDETECTORFACTORY_LITE_H
#define INDETSERVMATGEOMODEL_INDETSERVMATDETECTORFACTORY_LITE_H

#include "InDetServMatManager.h"
#include "GeoModelKernel/GeoVDetectorFactory.h"
#include <string>

class InDetServMatFactory_Lite final : public GeoVDetectorFactory  
{
 public:
  // Constructor:
  InDetServMatFactory_Lite() = default;
  
  // Illegal operations:
  const InDetServMatFactory_Lite & operator=(const InDetServMatFactory_Lite &right) = delete;
  InDetServMatFactory_Lite(const InDetServMatFactory_Lite &right) = delete;

  // Creation of geometry:
  virtual void create(GeoPhysVol *world) override;
  
  // Access to the results:
  virtual const InDetDD::InDetServMatManager * getDetectorManager() const override;

 private:  
  // The manager:
  InDetDD::InDetServMatManager* m_manager{nullptr};
};

#endif

