/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRACKINGGEOMETRY_MUONSTATIONBUILDERIMPL_H
#define MUONTRACKINGGEOMETRY_MUONSTATIONBUILDERIMPL_H

// Amg
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "GeoPrimitives/GeoPrimitives.h"
//
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonTrackingGeometry/MuonStationTypeBuilder.h"
#include "TrkDetDescrGeoModelCnv/GMTreeBrowser.h"
#include "TrkDetDescrGeoModelCnv/GeoMaterialConverter.h"
#include "TrkDetDescrGeoModelCnv/GeoShapeConverter.h"
#include "TrkDetDescrGeoModelCnv/VolumeConverter.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeHelper.h"
#include "TrkGeometry/DetachedTrackingVolume.h"
#include "TrkGeometry/TrackingVolume.h"

namespace Trk {
class MaterialProperties;
}

namespace Muon {



/** @class MuonStationBuilderImpl

    The Muon::MuonStationBuilderImpl retrieves muon stations from Muon Geometry
    Tree prototypes built with help of Muon::MuonStationTypeBuilder
    by Sarka.Todorova@cern.ch
  */

class MuonStationBuilderImpl : public AthAlgTool {
   public:
    using GMInfo = std::tuple<Amg::Transform3D, Identifier>;

    virtual ~MuonStationBuilderImpl() = default;
    virtual StatusCode initialize() override;

    using DetachedVolVec = std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>;
    DetachedVolVec buildDetachedTrackingVolumesImpl(const MuonGM::MuonDetectorManager* muonMgr,
                                                    bool blend = false) const;

   protected:
    using AthAlgTool::AthAlgTool;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    std::vector<std::pair<const GeoVPhysVol*, std::vector<GMInfo>>>
    retrieveGMsensitive(const MuonGM::MuonDetectorManager* muonMgr) const;

    std::unique_ptr<Trk::DetachedTrackingVolume>
    buildDetachedTrackingVolumeType(const GeoVPhysVol* gv, const GMInfo& info) const;


    std::vector<const Trk::Surface*> fetchSurfaces(const Identifier& stationId,
                                                   const MuonGM::MuonDetectorManager* detMgr) const;

    void glueComponents(Trk::DetachedTrackingVolume*) const;

    
    void identifyLayers(Trk::DetachedTrackingVolume&, 
                        const Identifier&,
                        const MuonGM::MuonDetectorManager*) const;

    void identifyLayers(Trk::TrackingVolume&, 
                        const Identifier&,
                        const MuonGM::MuonDetectorManager*) const;


    void checkLayerId(std::string_view comment,
                      const MuonGM::MuonDetectorManager* muonMgr, Identifier id,
                      const Trk::Layer* lay) const;

    ToolHandle<Muon::MuonStationTypeBuilder> m_muonStationTypeBuilder{
        this, "StationTypeBuilder",
        "Muon::MuonStationTypeBuilder/"
        "MuonStationTypeBuilder"};  //!< Helper Tool
                                    //!< to create
                                    //!< TrackingVolume
                                    //!< Arrays
    ToolHandle<Trk::ITrackingVolumeHelper> m_trackingVolumeHelper{
        this, "TrackingVolumeHelper",
        "Trk::TrackingVolumeHelper/TrackingVolumeHelper"};  //!< Helper Tool to
                                                            //!< create
                                                            //!< TrackingVolumes

    Trk::Material m_muonMaterial;  //!< the material
    //!< shape converter
    // Trk::GeoShapeConverter m_geoShapeConverter;
    Trk::GMTreeBrowser m_gmBrowser;
    Trk::VolumeConverter m_volumeConverter;
    //!< material converter
    Trk::GeoMaterialConverter m_materialConverter;
    Gaudi::Property<bool> m_buildBarrel{this, "BuildBarrelStations", true};
    Gaudi::Property<bool> m_buildEndcap{this, "BuildEndcapStations", true};
    Gaudi::Property<bool> m_buildCsc{this, "BuildCSCStations", true};
    Gaudi::Property<bool> m_buildTgc{this, "BuildTGCStations", true};
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONSTATIONBUILDERIMPL_H
