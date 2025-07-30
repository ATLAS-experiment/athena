/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_DBMMODULE_H
#define PIXELGEOMODEL_DBMMODULE_H

/**
 * @class DBM_Module
 * Build the DBM Module consisting of diamond sensor, FEI4 and ceramic substrate
 **/

#include "GeoVPixelFactory.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"

#include <memory>

namespace InDetDD {
  class SiDetectorDesign;
  class PixelDiodeMatrix;
}

class DBM_Module : public GeoVPixelFactory {
 public:

  DBM_Module(InDetDD::PixelDetectorManager* ddmgr,
             PixelGeometryManager* mgr,
	     GeoModelIO::ReadGeoModel* sqliteReader,
             std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
             std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX);
  virtual GeoVPhysVol* Build() override;

  private:

  const InDetDD::SiDetectorDesign* m_design;
};

#endif
