/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_ComponentFactory.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelRead/ReadGeoModel.h"
#include <sstream>
#include <string>
#include <utility>

using InDetDD::SCT_DetectorManager;

const double SCT_ComponentFactory::s_epsilon = 1.0e-6 * Gaudi::Units::mm;

SCT_ComponentFactory::SCT_ComponentFactory(const std::string & name,
                                           InDetDD::SCT_DetectorManager* detectorManager,
                                           SCT_GeometryManager* geometryManager,
                                           SCT_MaterialManager* materials)
  : m_detectorManager(detectorManager), 
    m_geometryManager(geometryManager),
    m_materials(materials),
    m_name(name)
{}

SCT_ComponentFactory::~SCT_ComponentFactory() 
{}

std::string 
SCT_ComponentFactory::intToString(int i) const
{
  std::ostringstream str;
  str << i;
  return str.str();
}

double
SCT_ComponentFactory::epsilon() const
{
  return s_epsilon;
}


SCT_UniqueComponentFactory::SCT_UniqueComponentFactory(const std::string & name,
			    InDetDD::SCT_DetectorManager* detectorManager,
                            SCT_GeometryManager* geometryManager,
                            SCT_MaterialManager* materials, 
                            GeoModelIO::ReadGeoModel* sqliteReader,
                            std::shared_ptr<std::map<std::string, GeoFullPhysVol*>>        mapFPV,
                            std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX) :
  SCT_ComponentFactory(name, detectorManager, geometryManager, materials),
  m_logVolume(nullptr),
  m_sqliteReader(sqliteReader),
  m_mapFPV(std::move(mapFPV)),
  m_mapAX(std::move(mapAX))
{};
