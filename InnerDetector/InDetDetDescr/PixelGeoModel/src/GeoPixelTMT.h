/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_GEOPIXELTMT_H
#define PIXELGEOMODEL_GEOPIXELTMT_H

#include "GeoPixelStaveSupport.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoModelKernel/GeoDefinitions.h"

class GeoShape;

class GeoPixelTMT : public GeoPixelStaveSupport {

public:  
  GeoPixelTMT(InDetDD::PixelDetectorManager* ddmgr,
              PixelGeometryManager* mgr, 
              GeoModelIO::ReadGeoModel* sqliteReader,
              std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
              std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX);
  virtual ~GeoPixelTMT() = default;
  virtual GeoVPhysVol* Build() override;
  virtual GeoVPhysVol* getPhysVol () override {return m_physVol;}
  virtual const GeoTrf::Transform3D & transform() const override {return m_transform;}
  virtual double thicknessP() const override {return 0;} // Use ladder thickness from database
  virtual double thicknessN() const override {return 0;} // Use ladder thickness from database
  virtual GeoSimplePolygonBrep* computeStaveEnvelopShape(double) override { return 0;}
  virtual GeoVPhysVol* getEndblockEnvelopShape(int) override {return 0;}
  virtual GeoTransform* getEndblockEnvelopShapeTrf(int) override {return 0;}
  virtual double getEndblockZpos() const override { return 0.; }
  virtual double getServiceZpos() const override { return 0; }
  virtual double getEndblockLength() const override { return 0.; }
  virtual void computeStaveEnvelopTransformAndSize(double ,double, double, double, double, double)  override {};

  virtual int PixelNModule() const override {return 0;}
  virtual int PixelNPlanarModule() const override {return 0;}
  virtual int PixelN3DModule() const override {return 0;}

private:
  const GeoShape * addShape(const GeoShape * lastShape, const GeoShape * nextShape, const GeoTrf::Transform3D & trans);

  PVLink m_physVol{nullptr};
  GeoTrf::Transform3D m_transform;
};

#endif



