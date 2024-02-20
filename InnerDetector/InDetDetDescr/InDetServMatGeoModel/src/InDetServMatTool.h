/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSERVMATGEOMODEL_INDETSERVMATTOOL_H
#define INDETSERVMATGEOMODEL_INDETSERVMATTOOL_H

#include "GeoModelUtilities/GeoModelTool.h"

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include <string>

class InDetServMatAthenaComps;
class IGeoDbTagSvc;
class IGeometryDBSvc;
class IInDetServMatBuilderTool;

namespace InDetDD {
  class InDetServMatManager;
}

class InDetServMatTool final : public GeoModelTool {

 public: 
  // Standard Constructor
  InDetServMatTool( const std::string& type, const std::string& name, const IInterface* parent );
  // Standard Destructor
  virtual ~InDetServMatTool() override;
  
  virtual StatusCode create() override;
  virtual StatusCode clear() override;

 private:
  ServiceHandle< IGeoDbTagSvc > m_geoDbTagSvc;
  ServiceHandle< IGeometryDBSvc > m_geometryDBSvc;
  ToolHandle<IInDetServMatBuilderTool> m_builderTool;

  bool m_devVersion{false};
  std::string m_overrideVersionName{};
  const InDetDD::InDetServMatManager* m_manager{nullptr};
  InDetServMatAthenaComps * m_athenaComps{nullptr};
};

#endif // INDETSERVMATGEOMODEL_INDETSERVMATTOOL_H

