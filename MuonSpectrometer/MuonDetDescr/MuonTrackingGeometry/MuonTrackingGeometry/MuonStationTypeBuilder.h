/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// MuonStationTypeBuilder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

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
#include "GaudiKernel/ToolHandle.h"      //member

// stl
#include <memory>  //std::unique_ptr
#include <string>
#include <utility>  //for std::pair
#include <vector>

class GeoVPhysVol;
class GeoShape;
class Identifier;

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

namespace MuonGM {
class MuonDetectorManager;
class MuonStation;
class MMReadoutElement;
class sTgcReadoutElement;
}  // namespace MuonGM

namespace Muon {

typedef std::pair<Trk::SharedObject<const Trk::Layer>, const Amg::Transform3D*>
    LayTr;

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
    Trk::TrackingVolumeArray* processBoxStationComponents(
        const GeoVPhysVol* cv, Trk::CuboidVolumeBounds* envBounds,
        Cache&) const;
    std::vector<Trk::Layer*>* processBoxComponentsArbitrary(
        const GeoVPhysVol* mv, Trk::CuboidVolumeBounds* envBounds,
        Cache& cache) const;

    Trk::TrackingVolumeArray* processTrdStationComponents(
        const GeoVPhysVol* cv, Trk::TrapezoidVolumeBounds* envBounds,
        Cache&) const;

    Trk::TrackingVolume* processCscStation(const GeoVPhysVol* cv,
                                           const std::string& name,
                                           Cache&) const;

    Trk::TrackingVolume* processTgcStation(const GeoVPhysVol* cv, Cache&) const;

    std::unique_ptr<Trk::DetachedTrackingVolume> process_sTGC(
        const MuonGM::MuonDetectorManager* muonMgr,
        const MuonGM::sTgcReadoutElement* stgc, Identifier id,
        const GeoVPhysVol* gv, Amg::Transform3D transf) const;

    std::unique_ptr<Trk::DetachedTrackingVolume> process_MM(
        const MuonGM::MuonDetectorManager* muonMgr,
        const MuonGM::MMReadoutElement* mm, Identifier id,
        const GeoVPhysVol* gv, Amg::Transform3D transf) const;

    /** components */
    Trk::TrackingVolume* processMdtBox(Trk::Volume*, const GeoVPhysVol*&,
                                       Amg::Transform3D*, double, Cache&) const;

    Trk::TrackingVolume* processMdtTrd(Trk::Volume*, const GeoVPhysVol*&,
                                       Amg::Transform3D*, Cache&) const;

    Trk::TrackingVolume* processRpc(Trk::Volume*,
                                    std::vector<const GeoVPhysVol*>,
                                    std::vector<Amg::Transform3D>,
                                    Cache&) const;

    Trk::TrackingVolume* processSpacer(Trk::Volume&,
                                       std::vector<const GeoVPhysVol*>,
                                       std::vector<Amg::Transform3D>) const;

    Trk::TrackingVolume* processNSW(
        const MuonGM::MuonDetectorManager* muonDetMgr,
        const std::vector<Trk::Layer*>&) const;

    Trk::LayerArray* processCSCTrdComponent(const GeoVPhysVol*&,
                                            Trk::TrapezoidVolumeBounds*&,
                                            Amg::Transform3D*&, Cache&) const;

    Trk::LayerArray* processCSCDiamondComponent(
        const GeoVPhysVol*&, Trk::DoubleTrapezoidVolumeBounds*&,
        Amg::Transform3D*&, Cache&) const;

    Trk::LayerArray* processTGCComponent(const GeoVPhysVol*&,
                                         Trk::TrapezoidVolumeBounds*&,
                                         Amg::Transform3D*&, Cache&) const;

    std::pair<Trk::Layer*, const std::vector<Trk::Layer*>*>
    createLayerRepresentation(Trk::TrackingVolume* trVol) const;

    Trk::Layer* createLayer(const MuonGM::MuonDetectorManager* detMgr,
                            Trk::TrackingVolume* trVol,
                            Trk::MaterialProperties*, Amg::Transform3D&) const;

    static Identifier identifyNSW(const MuonGM::MuonDetectorManager* muonDetMgr,
                                  const std::string&, const Amg::Transform3D&);

    void printChildren(const GeoVPhysVol*, int level = 0) const;
    // used to be private ..
    double get_x_size(const GeoVPhysVol*) const;
    double decodeX(const GeoShape*) const;
    double envelopeThickness(const Trk::VolumeBounds& vb) const;
    Trk::MaterialProperties getAveragedLayerMaterial(const GeoVPhysVol*, double,
                                                     double) const;
    Trk::MaterialProperties collectStationMaterial(
        const Trk::TrackingVolume* trVol, double) const;
    void printTransform(std::string comment, Amg::Transform3D transf) const;
    void printVolumeBounds(std::string comment,
                           const Trk::VolumeBounds& vb) const;

   private:
    // derive layer bounds from the station envelope
    Trk::SurfaceBounds* getLayerBoundsFromEnvelope(
        const Trk::Volume* envelope) const;

    // calculate area defined by (planar) surface bounds
    double area(const Trk::SurfaceBounds* sb) const;

    /** Private method to fill default material */
    // void fillDefaultServiceMaterial();

    Gaudi::Property<bool> m_multilayerRepresentation{
        this, "BuildMultilayerRepresentation", true};
    Gaudi::Property<bool> m_resolveSpacer{this, "ResolveSpacerBeams", false};

    ToolHandle<Trk::ITrackingVolumeArrayCreator>
        m_trackingVolumeArrayCreator{this, "TrackingVolumeArrayCreator",
                                     "Trk::TrackingVolumeArrayCreator/"
                                     "TrackingVolumeArrayCreator"};  //!< Helper
                                                                     //!< Tool
                                                                     //!< to
                                                                     //!< create
                                                                     //!< TrackingVolume
                                                                     //!< Arrays

    std::unique_ptr<Trk::Material> m_muonMaterial;  //!< the material
    Trk::GeoMaterialConverter m_materialConverter;
    Trk::GeoShapeConverter m_geoShapeConverter;
    Trk::VolumeConverter m_volumeConverter;
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONSTATIONTYPEBUILDER_H
