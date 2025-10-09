/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELSICRYSTAL_H
#define PIXELGEOMODEL_GEOPIXELSICRYSTAL_H

#include "Identifier/Identifier.h"
#include "GeoVPixelFactory.h"

#include <memory>

class GeoLogVol;

namespace InDetDD {
  class SiDetectorDesign;
}

class GeoPixelSiCrystal : public GeoVPixelFactory {
 public:
  GeoPixelSiCrystal(InDetDD::PixelDetectorManager* ddmgr,
                    PixelGeometryManager* mgr,
		    GeoModelIO::ReadGeoModel* sqliteReader,
                    std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                    std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX,
                    bool isBLayer, bool isModule3D=false, bool even_odd_phi_design=false);
  virtual GeoVPhysVol* Build() override;
  inline Identifier getID() {return m_id;}

  bool GetModule3DFlag() { return m_isModule3D; };

 private:
  Identifier m_id;
  // Cache for multiple phi designs.
  // The first element will hold the design for modules with either even or event+odd phi indices.
  // If there are more than one designs than the second element will hold the design for modules
  // with  odd phi index.
  unsigned int m_nPhiDesigns=1;
  std::array<const InDetDD::SiDetectorDesign*,2> m_design{nullptr,nullptr};
  bool m_isBLayer = false;
  bool m_isModule3D = false;
};

#endif
