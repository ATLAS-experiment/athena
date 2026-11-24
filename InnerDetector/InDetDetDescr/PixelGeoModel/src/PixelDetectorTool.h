/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELGEOMODEL_PIXELDETECTORTOOL_H
#define PIXELGEOMODEL_PIXELDETECTORTOOL_H

#include "GeoModelUtilities/GeoModelTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "GeoModelInterfaces/IGeoSubDetTool.h"
#include "PixelGeoModel/IIBLParameterSvc.h"
#include "InDetGeoModelUtils/IInDetServMatBuilderTool.h"

#include "CxxUtils/checker_macros.h"

class PixelGeoModelAthenaComps;

namespace InDetDD {
  class PixelDetectorManager;
}

class PixelDetectorTool final : public GeoModelTool {

 public:
  // Standard Constructor
  using GeoModelTool::GeoModelTool;
  // Standard Destructor
  virtual ~PixelDetectorTool() override final;

  virtual StatusCode initialize() override final;

  virtual StatusCode create() override final;
  virtual StatusCode clear() override final;

  // Callback function itself
  virtual StatusCode align() override;

private:
  //
  Gaudi::Property<bool> m_services{this, "Services", true};
  Gaudi::Property<bool> m_servicesOnLadder{this, "ServicesOnLadder", true}; //JBdV
  std::string m_detectorName{"PixelDetector"};
  ServiceHandle<IIBLParameterSvc> m_IBLParameterSvc{this,"IBLParameterSvc","IBLParameterSvc",""};
  Gaudi::Property<bool> m_dc1Geometry{this, "DC1Geometry", false};
  Gaudi::Property<bool> m_alignable{this, "Alignable", true};
  Gaudi::Property<bool> m_tweakIBLDist{this, "TweakIBLDist", true, "whether to tweak L3 transforms with IBLDist"};
  Gaudi::Property<bool> m_initialLayout{this, "InitialLayout", false};
  Gaudi::Property<bool> m_devVersion{this, "DevVersion", false};
  bool m_buildDBM{false};
  Gaudi::Property<bool> m_useDynamicAlignFolders{this, "useDynamicAlignFolders", false};
  ToolHandle< IGeoSubDetTool > m_bcmTool{this,"BCM_Tool","",""};
  ToolHandle< IGeoSubDetTool > m_blmTool{this,"BLM_Tool","",""};
  PublicToolHandle< IInDetServMatBuilderTool > m_serviceBuilderTool{this,"ServiceBuilderTool","",""};
  const InDetDD::PixelDetectorManager * m_manager{nullptr};
  Gaudi::Property<std::string> m_overrideVersionName{this, "OverrideVersionName", ""};

  PixelGeoModelAthenaComps * m_athenaComps{};

};

#endif // PIXELGEOMODEL_PIXELDETECTORTOOL_H
