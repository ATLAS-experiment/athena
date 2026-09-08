/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeometryBuilderCond.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_GEOMETRYBUILDERCOND_H
#define TRKDETDESCRTOOLS_GEOMETRYBUILDERCOND_H

// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkDetDescrInterfaces/IGeometryBuilderCond.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeBuilder.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeArrayCreator.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeHelper.h"
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/TrackingVolumeManipulator.h"
// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#ifdef TRKDETDESCR_MEMUSAGE
#include "TrkDetDescrUtils/MemoryLogger.h"
#endif

namespace Trk {

class TrackingGeometry;
class TrackingVolume;

/** @class GeometryBuilderCond

  The Trk::TrackingGeometry Builder for ATLAS Geometry

  It retrieves Trk::TrackingGeometry builders for the subdetectors and joins them together
  to a single Trk::TrackingGeometry.

  @author Andreas.Salzburger@cern.ch
  @author Christos Anastopoulos MT fixes
 */

class GeometryBuilderCond
  : public AthAlgTool
  , public TrackingVolumeManipulator
  , virtual public IGeometryBuilderCond
{

public:
  /** Constructor */
  GeometryBuilderCond(const std::string&, const std::string&, const IInterface*);

  /** AlgTool initialize method */
  virtual StatusCode initialize() override;

  /**
   * TrackingGeometry Interface method - optionally a pointer to Bounds
   * Interface marked as not thread safe
   */
  virtual std::unique_ptr<Trk::TrackingGeometry> trackingGeometry(
    const EventContext& ctx,
    Trk::TrackingVolume* tVol,
    SG::WriteCondHandle<TrackingGeometry>& whandle) const override;

  /** The unique signature */
  virtual GeometrySignature geometrySignature() const override { return Trk::Global; }

private:
  /** TrackingGeometry for ATLAS setup */
  std::unique_ptr<Trk::TrackingGeometry> atlasTrackingGeometry
  (const EventContext& ctx, SG::WriteCondHandle<TrackingGeometry>& whandle) const;

#ifdef TRKDETDESCR_MEMUSAGE
  MemoryLogger m_memoryLogger{}; //!< in case the memory is logged
#endif

  Gaudi::Property<bool> m_createWorld{this, "CreateWorldManually", true,
    "Boolean Switch to create World manually"};
  Gaudi::Property<int> m_navigationLevel{this, "NavigationLevel", 2};

  Gaudi::Property<std::vector<double>> m_worldDimension
    {this, "WorldDimension", {}, "The dimensions of the manually created world"};
  Gaudi::Property<std::vector<double>> m_worldMaterialProperties
    {this, "WorldMaterialProperties", {}, "The material properties of the created world"};
  Material m_worldMaterial{};                      //!< the world material

  // -------------------------- Tools for geometry building ------------------------------------------------------ //

  ToolHandle<ITrackingVolumeArrayCreator> m_trackingVolumeArrayCreator
    {this, "TrackingVolumeArrayCreator",
     "Trk::TrackingVolumeArrayCreator/TrackingVolumeArrayCreator",
     "Helper Tool to create TrackingVolume Arrays"};

  ToolHandle<ITrackingVolumeHelper> m_trackingVolumeHelper
    {this, "TrackingVolumeHelper",
     "Trk::TrackingVolumeHelper/TrackingVolumeHelper",
     "Helper Tool to create TrackingVolumes"};

  ToolHandle<IGeometryBuilderCond> m_inDetGeometryBuilderCond
    {this, "InDetTrackingGeometryBuilder", "",
     "GeometryBuilderCond for the InnerDetector"};

  ToolHandle<IGeometryBuilderCond> m_caloGeometryBuilderCond
    {this, "CaloTrackingGeometryBuilder", "",
     "GeometryBuilderCond for the Calorimeters"};

  ToolHandle<IGeometryBuilderCond> m_hgtdGeometryBuilderCond
    {this, "HGTD_TrackingGeometryBuilder", "", "GeometryBuilder for the HGTD"};

  ToolHandle<IGeometryBuilderCond> m_muonGeometryBuilderCond
    {this, "MuonTrackingGeometryBuilder", "",
     "GeometryBuilderCond for the Muon System"};

  Gaudi::Property<bool> m_compactify{this, "Compactify", true,
    "optimize event memory usage: register all surfaces with TG"};
  Gaudi::Property<bool> m_synchronizeLayers{this, "SynchronizeLayers", true,
    "synchronize contained layer dimensions to volumes"};
};

} // end of namespace

#endif // TRKDETDESCRTOOLS_GEOMETRYBUILDERCOND_H

