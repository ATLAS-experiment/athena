/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelUtilities/GeoBorderSurface.h"

#include <exception>

GeoBorderSurface::GeoBorderSurface(const std::string& name,
				   GeoOpticalPhysVol* pv1,
				   GeoOpticalPhysVol* pv2,
				   GeoOpticalSurface* opticalSurface):
  m_name(name),
  m_pv1(pv1),
  m_pv2(pv2),
  m_opticalSurface(opticalSurface){
  if(pv1->isShared() || pv2->isShared())
    throw std::runtime_error("Attempt to create a surface with shared physical volumes");

 }


