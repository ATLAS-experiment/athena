/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELCHIP_H
#define GEOPIXELCHIP_H

#include "GeoVPixelFactory.h"
class GeoLogVol;

class GeoPixelChip : public GeoVPixelFactory {
 public:
  GeoPixelChip(InDetDD::PixelDetectorManager* ddmgr,
               PixelGeometryManager* mgr,
	       GeoModelIO::ReadGeoModel* sqliteReader,
               std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
               std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX,
               bool isModule3D)
    : GeoVPixelFactory (ddmgr, mgr, sqliteReader, std::move(mapFPV), std::move(mapAX)),
      m_isModule3D(isModule3D)
  {};
  virtual GeoVPhysVol* Build() override;

 private:
  bool m_isModule3D;
};

#endif
