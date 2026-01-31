/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIALMAPPING_H
#define ACTSMATERIALMAPPING_H

// ATHENA
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"  /*no forward decl: typedef*/
#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkGeometry/MaterialStepCollection.h"

// ACTS
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Material/SurfaceMaterialMapper.hpp"
#include "Acts/Material/VolumeMaterialMapper.hpp"
// PACKAGE
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

// STL
#include <memory>
#include <vector>
#include <fstream>
#include <mutex>

namespace Acts {
  class ISurfaceMaterial;
  class IVolumeMaterial;

  using SurfaceMaterialMap
      = std::map<GeometryIdentifier, std::shared_ptr<const ISurfaceMaterial>>;

  using VolumeMaterialMap
      = std::map<GeometryIdentifier, std::shared_ptr<const IVolumeMaterial>>;

  using DetectorMaterialMaps = std::pair<SurfaceMaterialMap, VolumeMaterialMap>;
}

class IActsMaterialTrackWriterSvc;
class IActsMaterialStepConverterTool;
class IActsSurfaceMappingTool;
class IActsVolumeMappingTool;
class IActsMaterialJsonWriterTool;

// Not reentrant due to the mutable State variables.
class ActsMaterialMapping : public AthAlgorithm {
public:
  using AthAlgorithm::AthAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:
  ServiceHandle<IActsMaterialTrackWriterSvc>      m_materialTrackWriterSvc{this, "MaterialWriterSvc", "ActsMaterialTrackWriterSvc"};
  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
  Gaudi::Property<bool>                           m_mapSurfaces{this, "mapSurfaces", true, "Map the material onto surfaces"};
  Gaudi::Property<bool>                           m_mapVolumes{this, "mapVolumes", true, "Map the material onto volumes"};
  ToolHandle<IActsMaterialStepConverterTool>      m_materialStepConverterTool{this, "MaterialStepConverterTool", "ActsMaterialStepConverterTool"};
  SG::ReadHandleKey<Trk::MaterialStepCollection>  m_inputMaterialStepCollection{this, "MaterialStepRecordsKey", "MaterialStepRecords"};
  ToolHandle<IActsSurfaceMappingTool>             m_surfaceMappingTool{this, "SurfaceMappingTool", "ActsSurfaceMappingTool"};
  ToolHandle<IActsVolumeMappingTool>              m_volumeMappingTool{this, "VolumeMappingTool", "ActsVolumeMappingTool"};
  ToolHandle<IActsMaterialJsonWriterTool>         m_materialJsonWriterTool{this, "MaterialJsonWriterTool", "ActsMaterialJsonWriterTool"};

  std::optional<Acts::SurfaceMaterialMapper::State> m_mappingState;
  std::optional<Acts::VolumeMaterialMapper::State>  m_mappingStateVol;
};

#endif // ActsGeometry_ActsExtrapolation_h
