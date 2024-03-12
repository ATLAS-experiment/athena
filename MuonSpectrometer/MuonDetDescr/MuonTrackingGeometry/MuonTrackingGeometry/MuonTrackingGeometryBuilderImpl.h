/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MuonTrackingGeometryBuilderImpl.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef MUONTRACKINGGEOMETRY_MUONTRACKINGGEOMETRYBUILDERIMPL_H
#define MUONTRACKINGGEOMETRY_MUONTRACKINGGEOMETRYBUILDERIMPL_H
// Amg
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkDetDescrGeoModelCnv/VolumeConverter.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeArrayCreator.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeHelper.h"
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkGeometry/TrackingGeometry.h"
// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonTrackingGeometry/MuonStationBuilder.h"
// EnvelopeDefinitionService
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"
#include "SubDetectorEnvelopes/RZPair.h"

namespace Trk {
class TrackingGeometry;
class Material;
class VolumeBounds;
class ITrackingVolumeBuilder;

using TrackingVolumeOrderPosition = std::pair<std::shared_ptr<TrackingVolume>, 
                                              Amg::Vector3D>;

}  // namespace Trk

namespace Muon {

/** @class MuonTrackingGeometryBuilderImpl

    The Muon::MuonTrackingGeometryBuilderImpl retrieves MuonStationBuilder and
   MuonInertMaterialBuilder for the Muon Detector sub detectors and combines the
   given Volumes to a full Trk::TrackingGeometry.

    Inheriting directly from IGeometryBuilderCond it can use the protected
   member functions of the IGeometryBuilderCond to glue Volumes together and
   exchange BoundarySurfaces

    @author Sarka.Todorova@cern.ch
  */

class MuonTrackingGeometryBuilderImpl : public AthAlgTool {
   public:
    /** Destructor */
    virtual ~MuonTrackingGeometryBuilderImpl() = default;
    /** AlgTool initailize method.*/
    virtual StatusCode initialize() override;
    /** TrackingGeometry Interface method */
    
    using DetachedVolPtr = std::unique_ptr<Trk::DetachedTrackingVolume>;
    using DetachedVolVec = std::vector<DetachedVolPtr>;
    using VolumeSpanPtr = std::shared_ptr<const Trk::VolumeSpan>;
    using DetachedVolSpanPair = std::pair<Trk::DetachedTrackingVolume*, VolumeSpanPtr>;
    using VolumeSpanArray = std::array<std::vector<DetachedVolSpanPair>, 9>;
    using TrackingVolumePtr = std::unique_ptr<Trk::TrackingVolume>;
    using SharedTrackingVolume = std::shared_ptr<Trk::TrackingVolume>;
    using TrackingVolumeVec = std::vector<TrackingVolumePtr>;

    std::unique_ptr<Trk::TrackingGeometry> trackingGeometryImpl(DetachedVolVec && stations,
                                                                DetachedVolVec && inertObjs,
                                                                Trk::TrackingVolume* tvol) const;

    /** The unique signature */
    static Trk::GeometrySignature signature() { return Trk::MS; }

   protected:
    MuonTrackingGeometryBuilderImpl(const std::string&, const std::string&,
                                    const IInterface*);

    /** Private struct to contain local variables we dont want to be global in
     * this class*/
    struct LocalVariablesContainer {
        LocalVariablesContainer() = default;
        double m_innerBarrelRadius = 0;
        double m_outerBarrelRadius = 0;
        double m_innerEndcapZ = 0;
        double m_outerEndcapZ = 0;
        bool m_adjustStatic = false;
        bool m_static3d = false;
        unsigned int m_frameNum = 0;
        unsigned int m_frameStat = 0;
        std::vector<double> m_zPartitions;
        std::vector<int> m_zPartitionsType;
        std::vector<float> m_adjustedPhi;
        std::vector<int> m_adjustedPhiType;
        std::vector<
            std::vector<std::vector<std::vector<std::pair<int, float>>>>>
            m_hPartitions;
        std::vector<double> m_shieldZPart;
        std::vector<std::vector<std::pair<int, float>>> m_shieldHPart;
        std::map<Trk::DetachedTrackingVolume*, 
                 std::vector<Trk::TrackingVolume*>> m_blendMap;
        VolumeSpanArray m_stationSpan{};
        VolumeSpanArray m_inertSpan{};
        RZPairVector m_msCutoutsIn;
        RZPairVector m_msCutoutsOut;
        Trk::Material m_muonMaterial;  //!< the (empty) material
    };

    /** Volume helper to find geometrical span of enclosed volumes */
    Trk::VolumeConverter m_volumeConverter;
    /** Private method to filter detached volumes in z span */
    VolumeSpanArray findVolumesSpan(const DetachedVolVec& objs,
                                    double zTol, double phiTol,
                                    const LocalVariablesContainer& aLVC) const;
    /** Private methods to define subvolumes and fill them with detached volumes
     */
    TrackingVolumePtr processVolume(const Trk::Volume&, int, int,
                                    const std::string&,
                                    LocalVariablesContainer& aLVC,
                                    bool hasStations) const;
    TrackingVolumePtr processVolume(const Trk::Volume&, int,
                                    const std::string&,
                                    LocalVariablesContainer& aLVC,
                                    bool hasStations) const;
    TrackingVolumePtr processShield(const Trk::Volume&, int,
                                    const std::string&,
                                    LocalVariablesContainer& aLVC,
                                    bool hasStations) const;
    /** Private method to find detached volumes */
    
    std::vector<Trk::DetachedTrackingVolume*> 
            getDetachedObjects(const Trk::Volume& trkVol, 
                               std::vector<Trk::DetachedTrackingVolume*>&,
                               LocalVariablesContainer& aLVC, 
                               int mode = 0) const;
    /** Private method to check if constituent enclosed */
    bool enclosed(const Trk::Volume& volume, 
                  const Trk::VolumeSpan& span,
                  LocalVariablesContainer& aLVC) const;
    /** Private method to retrieve z partition */
    void getZParts(LocalVariablesContainer& aLVC) const;
    /** Private method to retrieve phi partition */
    void getPhiParts(int, LocalVariablesContainer& aLVC) const;
    /** Private method to retrieve h partition */
    void getHParts(LocalVariablesContainer& aLVC) const;
    /** Private method to retrieve shield partition */
    void getShieldParts(LocalVariablesContainer& aLVC) const;
    /** Private method to blend the inert material */
    void blendMaterial(LocalVariablesContainer& aLVC) const;

    ToolHandle<Trk::ITrackingVolumeArrayCreator> m_trackingVolumeArrayCreator{
        this, "TrackingVolumeArrayCreator",
        "Trk::TrackingVolumeArrayCreator/"
        "TrackingVolumeArrayCreator"};  //!< Helper
                                        //!< Tool to
                                        //!< create
                                        //!< TrackingVolume
                                        //!< Arrays

    ToolHandle<Trk::ITrackingVolumeHelper> m_trackingVolumeHelper{
        this, "TrackingVolumeHelper",
        "Trk::TrackingVolumeHelper/TrackingVolumeHelper"};  //!< Helper Tool to
                                                            //!< create
                                                            //!< TrackingVolumes

    ServiceHandle<IEnvelopeDefSvc> m_enclosingEnvelopeSvc{
        this, "EnvelopeDefinitionSvc", "AtlasEnvelopeDefSvc",
        "n"};  //!< service to provide input volume size

    Gaudi::Property<bool> m_muonSimple{this, "SimpleMuonGeometry", false};
    Gaudi::Property<bool> m_loadMSentry{this, "LoadMSEntry", false};
    Gaudi::Property<bool> m_muonActive{this, "BuildActiveMaterial", true};
    Gaudi::Property<bool> m_muonInert{this, "BuildInertMaterial", true};

    // Overall Dimensions
    //!< minimal extend in radial dimension of the muon barrel
    Gaudi::Property<double> m_innerBarrelRadius{this, "InnerBarrelRadius",
                                                4255.};
    //!< maximal extend in radial dimension of the muon barrel
    Gaudi::Property<double> m_outerBarrelRadius{this, "OuterBarrelRadius",
                                                13910.};
    //!< maximal extend in z of the muon barrel
    Gaudi::Property<double> m_barrelZ{this, "BarrelZ", 6785.};
    //!< maximal extend in z of the inner part of muon endcap
    Gaudi::Property<double> m_innerEndcapZ{this, "InnerEndcapZ", 12900.};
    //!< maximal extend in z of the outer part of muon endcap
    Gaudi::Property<double> m_outerEndcapZ{this, "OuterEndcapZ", 26046.};
    
    static constexpr double m_bigWheel{15600.};    //!< maximal extend in z of the big wheel
    static constexpr double m_outerWheel{21000.};  //!< minimal extend in z of the outer wheel (EO)
    static constexpr double m_ectZ{7920.};        //!< minimal extent in z of the ECT
    static constexpr double m_beamPipeRadius{70.};
    static constexpr double m_innerShieldRadius{850.};
    static constexpr double m_outerShieldRadius{1500.};
    static constexpr double m_diskShieldZ{6915.};

    Gaudi::Property<int> m_barrelEtaPartition{this, "EtaBarrelPartitions", 9};
    Gaudi::Property<int> m_innerEndcapEtaPartition{
        this, "EtaInnerEndcapPartitions", 3};
    Gaudi::Property<int> m_outerEndcapEtaPartition{
        this, "EtaOuterEndcapPartitions", 3};
    Gaudi::Property<int> m_phiPartition{this, "PhiPartitions", 16};

    Gaudi::Property<bool> m_adjustStatic{this, "AdjustStatic", true};
    Gaudi::Property<bool> m_static3d{this, "StaticPartition3D", true};

    Gaudi::Property<bool> m_blendInertMaterial{this, "BlendInertMaterial",
                                               false};
    Gaudi::Property<bool> m_removeBlended{this, "RemoveBlendedMaterialObjects",
                                          false};
    Gaudi::Property<double> m_alignTolerance{this, "AlignmentPositionTolerance",
                                             0.};
    Gaudi::Property<int> m_colorCode{this, "ColorCode", 0};
    Gaudi::Property<int> m_activeAdjustLevel{this, "ActiveAdjustLevel", 2};
    Gaudi::Property<int> m_inertAdjustLevel{this, "InertAdjustLevel", 1};

    Gaudi::Property<std::string> m_entryVolume{this, "EntryVolumeName",
                                               "MuonSpectrometerEntrance"};
    Gaudi::Property<std::string> m_exitVolume{
        this, "ExitVolumeName", "All::Container::CompleteDetector"};
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONTRACKINGGEOMETRYBUILDER_H
