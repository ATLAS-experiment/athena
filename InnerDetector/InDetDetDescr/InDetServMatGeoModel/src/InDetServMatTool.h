/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSERVMATGEOMODEL_INDETSERVMATTOOL_H
#define INDETSERVMATGEOMODEL_INDETSERVMATTOOL_H

#include "InDetServMatAthenaComps.h"
#include "GeoModelUtilities/GeoModelTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "InDetGeoModelUtils/IInDetServMatBuilderTool.h"

#include <string>

namespace InDetDD {
  class InDetServMatManager;
}

class InDetServMatTool final : public GeoModelTool {

 public: 
  // Standard Constructor
  InDetServMatTool( const std::string& type, const std::string& name, const IInterface* parent );
  // Standard Destructor
  virtual ~InDetServMatTool() override = default;
  
  virtual StatusCode create() override;
  virtual StatusCode clear() override;

 private:
  ServiceHandle< IGeoDbTagSvc > m_geoDbTagSvc{this,"GeoDbTagSvc","GeoDbTagSvc"};
  ToolHandle<IInDetServMatBuilderTool> m_builderTool{this,"ServiceBuilderTool",""};
  StringProperty m_overrideVersionName{this,"OverrideVersionName",""};

  const InDetDD::InDetServMatManager* m_manager{nullptr};
  InDetServMatAthenaComps m_athenaComps{};
};

#endif // INDETSERVMATGEOMODEL_INDETSERVMATTOOL_H

