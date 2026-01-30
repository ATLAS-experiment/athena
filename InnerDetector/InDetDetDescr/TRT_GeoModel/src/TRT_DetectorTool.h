/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_GEOMODEL_TRTDETECTORTOOL_H
#define TRT_GEOMODEL_TRTDETECTORTOOL_H

#include "InDetGeoModelUtils/InDetDDAthenaComps.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h"

#include "GeoModelUtilities/GeoModelTool.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "GeometryDBSvc/IGeometryDBSvc.h"

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include <string>

namespace InDetDD {
  class TRT_DetectorManager;
}

class TRT_DetectorTool final : public GeoModelTool {

public:
  // Standard Constructor
  TRT_DetectorTool( const std::string& type, const std::string& name, const IInterface* parent );

  // Standard Destructor
  virtual ~TRT_DetectorTool() = default;

  virtual StatusCode create() override final;
  virtual StatusCode clear() override final;

  // Callback function itself
  virtual StatusCode align() override final;

private:
  Gaudi::Property<bool> m_useOldActiveGasMixture{this,"UseOldActiveGasMixture",false};
  Gaudi::Property<bool> m_DC2CompatibleBarrelCoordinates{this,"DC2CompatibleBarrelCoordinates",false};
  Gaudi::Property<bool> m_alignable{this,"Alignable",true};
  Gaudi::Property<bool> m_useDynamicAlignFolders{this,"useDynamicAlignFolders",false};

  // Set to true to use argon. DEFAULT VALUE is true. Overridden by DOARGONMIXTURE switch
  Gaudi::Property<bool> m_doArgonMixture{this,"DoXenonArgonMixture",true};
  // Set to true to use krypton. DEFAULT VALUE is true. Overridden by DOKRYPTONMIXTURE switch
  Gaudi::Property<bool> m_doKryptonMixture{this,"DoKryptonMixture",true};

  ServiceHandle< IGeoDbTagSvc > m_geoDbTagSvc{this,"GeoDbTagSvc","GeoDbTagSvc"};
  ServiceHandle< IGeometryDBSvc > m_geometryDBSvc{this,"GeometryDBSvc","InDetGeometryDBSvc"};

  Gaudi::Property<bool> m_dumpStrawStatus{this, "DumpStrawStatus", false};
  Gaudi::Property<std::string> m_strawStatusFile{this, "StrawStatusFile", ""};
  ToolHandle<ITRT_StrawStatusSummaryTool> m_sumTool{this, "SummaryTool", ""}; // added for Argon

  const InDetDD::TRT_DetectorManager* m_manager{nullptr};
  InDetDD::AthenaComps m_athenaComps{"TRT_GeoModel"};
  bool m_initialLayout{true};
};

#endif // TRT_GEOMODEL_TRTDETECTORTOOL_H
