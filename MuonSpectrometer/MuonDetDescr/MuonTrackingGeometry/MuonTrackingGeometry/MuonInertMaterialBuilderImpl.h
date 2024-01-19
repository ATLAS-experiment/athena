/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MuonInertMaterialBuilderImpl.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef MUONTRACKINGGEOMETRY_MUONINERTMATERIALBUILDERIMPL_H
#define MUONTRACKINGGEOMETRY_MUONINERTMATERIALBUILDERIMPL_H

// Amg
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkDetDescrGeoModelCnv/GeoShapeConverter.h"
#include "TrkDetDescrGeoModelCnv/VolumeConverter.h"
#include "TrkDetDescrGeoModelCnv/GMTreeBrowser.h"
#include "TrkGeometry/DetachedTrackingVolume.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/TrackingVolumeManipulator.h"
// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
// GeoModel
#include "GeoModelKernel/GeoVPhysVol.h"

// CondData
#include "MuonReadoutGeometry/MuonDetectorManager.h"

namespace Trk {
class TrackingVolume;

}  // namespace Trk

namespace MuonGM {
class MuonStation;
}

namespace Muon {

/** @class MuonInertMaterialBuilderImpl

    The Muon::MuonInertMaterialBuilderImpl retrieves muon stations from Muon
   Geometry Tree

    by Sarka.Todorova@cern.ch, Marcin.Wolter@cern.ch
  */

class MuonInertMaterialBuilderImpl : public AthAlgTool, public Trk::TrackingVolumeManipulator {
 public:
  /** Destructor */
  virtual ~MuonInertMaterialBuilderImpl() = default;
  /** AlgTool initialize method.*/
  virtual StatusCode initialize() override;
  
  std::unique_ptr< std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>>
    buildDetachedTrackingVolumesImpl(const MuonGM::MuonDetectorManager* muonMgr, bool blend) const;
  
 protected:
  /** Constructor */
  MuonInertMaterialBuilderImpl(const std::string&, const std::string&, const IInterface*);
  
  /** Method creating material object prototypes */
 
  std::vector<std::pair<Trk::DetachedTrackingVolume*, std::vector<Amg::Transform3D> > >     
    buildDetachedTrackingVolumeTypes(const MuonGM::MuonDetectorManager* muonMgr, bool blend) const;

  /** Method extracting material objects from GeoModel tree */
  void getObjsForTranslation(
      const GeoVPhysVol* pv, const Amg::Transform3D&,
      std::vector<std::pair<const GeoVPhysVol*, std::vector<Amg::Transform3D> > >& vols) const;
  /** Dump from GeoModel tree  */
  void printInfo(const GeoVPhysVol* pv) const;
  void printChildren(const GeoVPhysVol* pv) const;

  Gaudi::Property<bool> m_simplify{this, "SimplifyGeometry", false};  // switch geometry simplification on/off
  Gaudi::Property<bool> m_simplifyToLayers{this, "SimplifyGeometryToLayers",
                                           false};  // switch geometry simplification to layers on/off
  Gaudi::Property<bool> m_debugMode{this, "DebugMode", false};  // build layers & dense volumes in parallel -
                                                                // double counting material !!!
  Gaudi::Property<bool> m_buildBT{this, "BuildBarrelToroids", true};   // build barrel toroids
  Gaudi::Property<bool> m_buildECT{this, "BuildEndcapToroids", true};  // build endcap toroids
  Gaudi::Property<bool> m_buildFeets{this, "BuildFeets", true};        // build feets
  Gaudi::Property<int> m_buildRails{this, "BuildRails", 1};            // build rails
  Gaudi::Property<bool> m_buildShields{this, "BuildShields", true};    // build shieldings
  Gaudi::Property<bool> m_buildSupports{this, "BuildSupports", true};  // build other
  Gaudi::Property<bool> m_buildNSWInert{this, "BuildNSWInert", true};  // build NSW inert material
  Gaudi::Property<double> m_blendLimit{this, "BlendLimit", 1e+06};  // mass limit for blending [g] 
  Trk::GeoShapeConverter m_geoShapeConverter;               //!< shape converter
  Trk::GMTreeBrowser m_gmBrowser;                                  //!< gm tree helper
  Trk::VolumeConverter m_volumeConverter;                                  //!< gm->trk volume helper
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONINERTMATERIALBUILDERIMPL_H
