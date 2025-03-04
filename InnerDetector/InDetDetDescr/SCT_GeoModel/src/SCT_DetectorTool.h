/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_GEOMODEL_SCT_DETECTORTOOL_H
#define SCT_GEOMODEL_SCT_DETECTORTOOL_H

#include "GeoModelUtilities/GeoModelTool.h"
#include "SCT_GeoModelAthenaComps.h" 

#include "GeometryDBSvc/IGeometryDBSvc.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"

#include "CxxUtils/checker_macros.h"

#include "GaudiKernel/ServiceHandle.h"

#include <string>

namespace InDetDD {
  class SCT_DetectorManager;
}

class SCT_DetectorTool : public GeoModelTool {

public:
  // Standard Constructor
  SCT_DetectorTool(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode create() override final;
  virtual StatusCode clear() override final;

  // Callback function itself
  virtual StatusCode align(IOVSVC_CALLBACK_ARGS) override;

private:
  BooleanProperty m_alignable{this, "Alignable", true};
  BooleanProperty m_useDynamicAlignFolders{this, "useDynamicAlignFolders", false};
  bool m_cosmic{false};

  const InDetDD::SCT_DetectorManager* m_manager{nullptr};
  
  SCT_GeoModelAthenaComps m_athenaComps;

  ServiceHandle< IGeoDbTagSvc > m_geoDbTagSvc{this,"GeoDbTagSvc","GeoDbTagSvc"};
  ServiceHandle< IGeometryDBSvc > m_geometryDBSvc{this,"GeometryDBSvc","InDetGeometryDBSvc"};
};

#endif // SCT_GEOMODEL_SCT_DETECTORTOOL_H
