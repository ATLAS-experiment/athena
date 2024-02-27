/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GeoBorderSurface_h
#define GeoBorderSurface_h 1

#include <string>
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelUtilities/GeoOpticalPhysVol.h"
#include "GeoModelUtilities/GeoOpticalSurface.h"


class GeoOpticalPhysVol;
class GeoOpticalSurface;

class GeoBorderSurface 
{
 public:
  GeoBorderSurface(const std::string& name,
		   GeoOpticalPhysVol* pv1,
		   GeoOpticalPhysVol* pv2,
		   GeoOpticalSurface* opticalSurface);
  GeoBorderSurface(const GeoBorderSurface &right) = default;
  ~GeoBorderSurface() = default;

  GeoBorderSurface& operator= (const GeoBorderSurface &right) = delete;

  std::string getName() const {return m_name;}
  const GeoOpticalPhysVol* getPV1() const {return m_pv1;}
  const GeoOpticalPhysVol* getPV2() const {return m_pv2;}
  const GeoOpticalSurface* getOptSurface() const {return m_opticalSurface;}
 
 private:
  std::string m_name;
  GeoIntrusivePtr<GeoOpticalPhysVol> m_pv1{};
  GeoIntrusivePtr<GeoOpticalPhysVol> m_pv2{};
  GeoIntrusivePtr<GeoOpticalSurface> m_opticalSurface{};

};

#endif 
