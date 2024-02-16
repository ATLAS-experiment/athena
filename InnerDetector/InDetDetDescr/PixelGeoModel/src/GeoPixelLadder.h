/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELLADDER_H
#define PIXELGEOMODEL_GEOPIXELLADDER_H

#include "GeoVPixelFactory.h"

#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"

class GeoPixelSiCrystal;
class GeoPixelStaveSupport;

class GeoPixelLadder : public GeoVPixelFactory {
 public:
  GeoPixelLadder(InDetDD::PixelDetectorManager* ddmgr,
		  PixelGeometryManager* mgr,
		  GeoModelIO::ReadGeoModel* sqliteReader,
                  std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                  std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX,
                  GeoPixelSiCrystal& theSensor,
		  GeoPixelStaveSupport * staveSupport);
  GeoPixelLadder (const GeoPixelLadder&) = delete;
  GeoPixelLadder& operator= (const GeoPixelLadder&) = delete;
  virtual ~GeoPixelLadder() = default;
  virtual GeoVPhysVol* Build() override;
  double thickness() const {return m_thickness;}
  double thicknessP() const {return m_thicknessP;}
  double thicknessN() const {return m_thicknessN;}
  double width() const {return m_width;}

 private:

  double calcThickness();
  double calcWidth(); 

  GeoIntrusivePtr<const GeoLogVol> m_theLadder{};
  GeoPixelSiCrystal& m_theSensor;
  GeoPixelStaveSupport * m_staveSupport;
  double m_thickness;
  double m_thicknessP;
  double m_thicknessN;
  double m_width;
};

#endif
