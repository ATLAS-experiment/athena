/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRACKINGGEOMETRY_MUONSTATIONTYPEBUILDER_H
#define MUONTRACKINGGEOMETRY_MUONSTATIONTYPEBUILDER_H
// Amg
#include "GeoPrimitives/GeoPrimitives.h"  //Amg stuff
// Trk
#include "TrkDetDescrGeoModelCnv/GeoShapeConverter.h"
#include "TrkDetDescrGeoModelCnv/VolumeConverter.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeArrayCreator.h"  //in tool handle template
#include "TrkDetDescrUtils/SharedObject.h"  //see the typedef for LayTr
#include "TrkGeometry/TrackingVolume.h"     //also for LayerArray typedef

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"  //base class
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

// stl
#include <memory>  //std::unique_ptr
#include <string>
#include <utility>  //for std::pair
#include <vector>


namespace Trk {
class Volume;
class Layer;
class CuboidVolumeBounds;
class TrapezoidVolumeBounds;
class DoubleTrapezoidVolumeBounds;
class PlaneLayer;
class Material;
class GeoMaterialConverter;
class MaterialProperties;
}  // namespace Trk

namespace MuonGM{
   class MuonDetectorManager;
}
namespace Muon {

/** @class MuonStationTypeBuilder

    The Muon::MuonStationTypeBuilder retrieves components of muon stations from
   Muon Geometry Tree, builds 'prototype' object (TrackingVolume with NameType)

    by Sarka.Todorova@cern.ch
  */

class MuonStationTypeBuilder : public AthAlgTool {
   public:
    struct Cache {
        std::unique_ptr<Trk::MaterialProperties> m_mdtTubeMat{};
        std::unique_ptr<Trk::MaterialProperties> m_rpcLayer{};
        std::vector<std::unique_ptr<Trk::MaterialProperties>> m_mdtFoamMat{};
        std::unique_ptr<Trk::MaterialProperties> m_rpc46{};
        std::vector<std::unique_ptr<Trk::MaterialProperties>> m_rpcDed{};
        std::unique_ptr<Trk::MaterialProperties> m_rpcExtPanel{};
        std::unique_ptr<Trk::MaterialProperties> m_rpcMidPanel{};
        std::unique_ptr<Trk::MaterialProperties> m_matCSC01{};
        std::unique_ptr<Trk::MaterialProperties> m_matCSCspacer1{};
        std::unique_ptr<Trk::MaterialProperties> m_matCSC02{};
        std::unique_ptr<Trk::MaterialProperties> m_matCSCspacer2{};
        std::unique_ptr<Trk::MaterialProperties> m_matTGC01{};
        std::unique_ptr<Trk::MaterialProperties> m_matTGC06{};
    };
    /** Constructor */
    MuonStationTypeBuilder(const std::string&, const std::string&,
                           const IInterface*);
    /** Destructor */
    virtual ~MuonStationTypeBuilder() = default;
    /** AlgTool initailize method.*/
    StatusCode initialize();
    /** AlgTool finalize method */
    StatusCode finalize();
    /** Interface methode */
    static const InterfaceID& interfaceID();
    /** steering routine */
    std::unique_ptr<Trk::TrackingVolumeArray> processBoxStationComponents(const GeoVPhysVol* cv, 
                                                                          const Trk::CuboidVolumeBounds& envBounds,
                                                                          Cache&) const;
    std::vector<std::unique_ptr<Trk::Layer>> processBoxComponentsArbitrary(const GeoVPhysVol* mv, 
                                                                           const Trk::CuboidVolumeBounds& envBounds,
                                                                           Cache& cache) const;

    std::unique_ptr<Trk::TrackingVolumeArray> processTrdStationComponents(const GeoVPhysVol* cv, 
                                                                          const Trk::TrapezoidVolumeBounds& envBounds,
                                                                          Cache&) const;

    std::unique_ptr<Trk::TrackingVolume> processCscStation(const GeoVPhysVol* cv,
                                                           const std::string& name,
                                                           Cache&) const;

    std::unique_ptr<Trk::TrackingVolume> processTgcStation(const GeoVPhysVol* cv, Cache&) const;

    std::unique_ptr<Trk::DetachedTrackingVolume> process_sTGC(const Identifier& id,
                                                              const GeoVPhysVol* gv, 
                                                              const Amg::Transform3D& transf) const;

    std::unique_ptr<Trk::DetachedTrackingVolume> process_MM(const Identifier& id,
                                                            const GeoVPhysVol* gv,
                                                            const Amg::Transform3D& transf) const;

    /** components */
    std::unique_ptr<Trk::TrackingVolume> processMdtBox(const Trk::Volume& trkVol, 
                                                       const GeoVPhysVol*,
                                                       const Amg::Transform3D&, 
                                                       double, Cache&) const;

    std::unique_ptr<Trk::TrackingVolume> processMdtTrd(const Trk::Volume& trkVol, 
                                                       const GeoVPhysVol*,
                                                       const Amg::Transform3D&, Cache&) const;

    std::unique_ptr<Trk::TrackingVolume> processRpc(const Trk::Volume& inVol,
                                                    const std::vector<const GeoVPhysVol*>& childVols,
                                                    const std::vector<Amg::Transform3D>& childVolsTrf,
                                                    Cache&) const;

    std::unique_ptr<Trk::TrackingVolume> processSpacer(const Trk::Volume&,
                                                       std::vector<const GeoVPhysVol*>,
                                                       std::vector<Amg::Transform3D>) const;

    std::unique_ptr<Trk::LayerArray> processCSCTrdComponent(const GeoVPhysVol*,
                                                            const Trk::TrapezoidVolumeBounds&,
                                                            const Amg::Transform3D& , 
                                                            Cache&) const;

    std::unique_ptr<Trk::LayerArray> processCSCDiamondComponent(const GeoVPhysVol*, 
                                                               const Trk::DoubleTrapezoidVolumeBounds&,
                                                               const Amg::Transform3D&, Cache&) const;

    std::unique_ptr<Trk::LayerArray> processTGCComponent(const GeoVPhysVol*,
                                                         const Trk::TrapezoidVolumeBounds&,
                                                         const Amg::Transform3D&, Cache&) const;

    std::pair<std::unique_ptr<Trk::Layer>, 
              std::vector<std::unique_ptr<Trk::Layer>>> createLayerRepresentation(Trk::TrackingVolume& trVol) const;


    Identifier identifyNSW(const std::string&, const Amg::Transform3D&) const;

    // used to be private ..
    double get_x_size(const GeoVPhysVol*) const;
    double decodeX(const GeoShape*) const;
    double envelopeThickness(const Trk::VolumeBounds& vb) const;
    Trk::MaterialProperties getAveragedLayerMaterial(const GeoVPhysVol*, double,
                                                     double) const;
    Trk::MaterialProperties collectStationMaterial(const Trk::TrackingVolume& trVol, double) const;

   private:
    void printVolumeBounds(std::string comment,
                           const Trk::VolumeBounds& vb) const;

    // derive layer bounds from the station envelope
    std::unique_ptr<Trk::SurfaceBounds> getLayerBoundsFromEnvelope(const Trk::Volume& envelope) const;

    // calculate area defined by (planar) surface bounds
    double area(const Trk::SurfaceBounds& sb) const;


    Gaudi::Property<bool> m_multilayerRepresentation{this, "BuildMultilayerRepresentation", true};
    Gaudi::Property<bool> m_resolveSpacer{this, "ResolveSpacerBeams", false};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    // Helper tool to create TrackingVolume Arrays
    ToolHandle<Trk::ITrackingVolumeArrayCreator> m_trackingVolumeArrayCreator{this, "TrackingVolumeArrayCreator",
                                                            "Trk::TrackingVolumeArrayCreator/TrackingVolumeArrayCreator"};  

    std::unique_ptr<const Trk::Material> m_muonMaterial;  //!< the material
    Trk::GeoMaterialConverter m_materialConverter;
    Trk::GeoShapeConverter m_geoShapeConverter;
    Trk::VolumeConverter m_volumeConverter;
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONSTATIONTYPEBUILDER_H
