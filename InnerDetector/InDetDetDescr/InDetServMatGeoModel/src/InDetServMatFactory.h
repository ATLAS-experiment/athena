/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSERVMATGEOMODEL_INDETSERVMATFACTORY_H
#define INDETSERVMATGEOMODEL_INDETSERVMATFACTORY_H

#include "InDetServMatManager.h"
#include "InDetGeoModelUtils/InDetDetectorFactoryBase.h"

namespace InDetDD {
  class AthenaComps;
}

class InDetServMatFactory : public InDetDD::DetectorFactoryBase  {

 public:
  
  // Constructor:
  InDetServMatFactory(InDetDD::AthenaComps * athenaComps);

  // Illegal operations:
  const InDetServMatFactory & operator=(const InDetServMatFactory &right) = delete;
  InDetServMatFactory(const InDetServMatFactory &right) = delete;

  // Destructor:
  ~InDetServMatFactory() = default;
  
  // Creation of geometry:
  virtual void create(GeoPhysVol *world) override;
  // manager
  virtual const InDetDD::InDetServMatManager* getDetectorManager () const override;

private:  
  // private data
  InDetDD::InDetServMatManager* m_manager{nullptr};
};

#endif //  INDETSERVMATGEOMODEL_INDETSERVMATFACTORY_H


