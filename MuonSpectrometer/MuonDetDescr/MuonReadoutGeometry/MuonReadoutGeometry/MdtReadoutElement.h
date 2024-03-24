/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONREADOUTGEOMETRY_MDTREADOUTELEMENT_H
#define MUONREADOUTGEOMETRY_MDTREADOUTELEMENT_H

#include "CxxUtils/CachedUniquePtr.h"
#include "CxxUtils/CachedValue.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "CxxUtils/ArrayHelper.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "TrkDistortedSurfaces/SaggedLineSurface.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
class BLinePar;
class MuonReadoutGeomCnvAlg;

namespace Trk {
    class CylinderBounds;
    class SurfaceBounds;
}  // namespace Trk

namespace MuonGM {

    /**
       An MdtReadoutElement corresponds to a single MDT multilayer; therefore
       typicaly a MDT chamber consists of two MdtReadoutElements, each identified
       by StationName, StationEta, StationPhi, Technology=0, and Multilayer.

       Pointers to all MdtReadoutElements are created in the build() method of
       the MuonChamber class, and are held in arrays by the MuonDetectorManager,
       which is responsible for storing, deleting and providing access to these
       objects.

       An MdtReadoutElement holds properties related to its internal structure
       (i.e. wire pitch) and general geometrical properties (size);  it
       implements tracking interfaces and provides access to typical
       readout-geometry information: i.e. number of tubes, position of the tubes,
       distance of a point to the readout side, etc.

       The globalToLocalCoords and globalToLocalTransform methods (+ their opposite)
       define the link between the ATLAS global reference frame and the internal
       (geo-model defined) local reference frame of any drift tube (which is the
       frame where local coordinates of SimHits, in output from G4, are expressed).
    */

    constexpr int maxnlayers = 4;
    constexpr int maxnsteps = 10;

    class MdtReadoutElement final : public MuonReadoutElement {
        friend class ::MuonReadoutGeomCnvAlg;
        friend class MuonChamber;
        friend class MuonChamberLite;

    public:
        MdtReadoutElement(GeoVFullPhysVol* pv, const std::string& stName,MuonDetectorManager* mgr);

        ~MdtReadoutElement() = default;

        /// How many MDT chambers are in the station
        unsigned int nMDTinStation() const { 
             return m_nMDTinStation;
        }
        void setNMdtInStation(unsigned int numMdt) { m_nMDTinStation = numMdt;}
        
        /// Returns whether the chamber is in the barrel 
        /// (Assement on first later in stationName)
        bool barrel() const;
        /// Returns whether the chamber is in the endcap
        bool endcap() const;
        /// Returns the multilayer represented by the readout element
        int getMultilayer() const;
        /// Returns the number of tube layers inside the multilayer
        int getNLayers() const;
        /// Returns the number of tubes in each tube layer
        int getNtubesperlayer() const;
        /// Returns the number of tubes in the endcap trapezoid sharing the same length
        int getNtubesinastep() const;
        /// Sets the multilayer number
        void setMultilayer(const int ml);
        /// Sets the number of layers
        void setNLayers(const int nl);
        
        bool getWireFirstLocalCoordAlongZ(int tubeLayer, double& coord) const;
        bool getWireFirstLocalCoordAlongR(int tubeLayer, double& coord) const;

        bool containsId(const Identifier& id) const override;

        // detector specific
        double tubeLength(const int tubeLayer, const int tube) const;
        double getActiveTubeLength(const int tubeLayer, const int tube) const;
        double getWireLength(const int tubeLayer, const int tube) const;
        double tubeLength(const Identifier& id) const;
        /// Returns the inner tube radius excluding the aluminium walls
        double innerTubeRadius() const;
        /// Returns the tube radius taking the thickness of the tubes into account
        double outerTubeRadius() const;
        /// Returns the distance between 2 tubes in a tube layer
        double tubePitch() const;

        
        const Amg::Transform3D& localToGlobalTransf(const Identifier& id) const;
        const Amg::Transform3D& localToGlobalTransf(const int tubeLayer, const int tube) const;
        
        Amg::Transform3D nodeform_localToGlobalTransf(const Identifier& id) const;
        Amg::Transform3D nodeform_localToGlobalTransf(const int tubeLayer, const int tube) const;

        // global to local(tube frame) coord.
        Amg::Transform3D globalToLocalTransf(const int tubeLayer, const int tube) const;
        Amg::Transform3D globalToLocalTransf(const Identifier& id) const;
        
        Amg::Transform3D nodeform_globalToLocalTransf(const Identifier& id) const;
        Amg::Transform3D nodeform_globalToLocalTransf(const int tubeLayer, const int tube) const;

        // in the native MDT reference system
        Amg::Vector3D localTubePos(const Identifier& id) const;
        Amg::Vector3D localTubePos(const int tubelayer, const int tube) const;

        Amg::Vector3D nodeform_localTubePos(const Identifier& id) const;
        Amg::Vector3D nodeform_localTubePos(const int tubelayer, const int tube) const;
        
        /// Returns the global position of the given tube
        Amg::Vector3D tubePos(const Identifier& id) const;
        Amg::Vector3D tubePos(const int tubelayer, const int tube) const;
        /// Returns the global position of the tube excluding the B-line & As-built corrections
        Amg::Vector3D nodeform_tubePos(const Identifier& id) const;
        Amg::Vector3D nodeform_tubePos(const int tubelayer, const int tube) const;
       

        // Readout / HV side
        double signedRODistanceFromTubeCentre(const Identifier& id) const;
        double signedRODistanceFromTubeCentre(const int tubeLayer, const int tube) const;

        double RODistanceFromTubeCentre(const Identifier& id) const;
        double RODistanceFromTubeCentre(const int tubeLayer, const int tube) const;
        
        double distanceFromRO(const Amg::Vector3D& GlobalHitPosition, const Identifier& id) const;
        double distanceFromRO(const Amg::Vector3D& GlobalHitPosition, const int tubelayer, const int tube) const;
        
        int isAtReadoutSide(const Amg::Vector3D& GlobalHitPosition, const Identifier& id) const;
        int isAtReadoutSide(const Amg::Vector3D& GlobalHitPosition, const int tubelayer, const int tube) const;
        
        Amg::Vector3D localROPos(const Identifier& id) const;
        Amg::Vector3D localROPos(const int tubelayer, const int tube) const;
        
        Amg::Vector3D ROPos(const int tubelayer, const int tube) const;
        Amg::Vector3D ROPos(const Identifier& id) const;
        
        Amg::Vector3D tubeFrame_localROPos(const int tubelayer, const int tube) const;
        Amg::Vector3D tubeFrame_localROPos(const Identifier& id) const;

        // defining B-line parameters
        void setBLinePar(const BLinePar* bLine);
        void clearBLinePar();
        const BLinePar* getBLinePar() const { return m_BLinePar; }

        ////////////////////////////////////////////////////////////
        //// Tracking interfaces
        ////////////////////////////////////////////////////////////

        void clearCache() override final;
        void fillCache() override final;

        virtual const Trk::Surface& surface() const override final;
        virtual const Trk::SaggedLineSurface& surface(const Identifier& id) const override final;
        virtual const Trk::SaggedLineSurface& surface(const int tubeLayer, const int tube) const;
        virtual const Trk::SurfaceBounds& bounds() const override final;
        virtual const Trk::CylinderBounds& bounds(const Identifier& id) const override final;
        virtual const Trk::CylinderBounds& bounds(const int tubeLayer, const int tube) const;

        virtual const Amg::Transform3D& transform(const Identifier& id) const override final;
        virtual const Amg::Transform3D& transform() const override final;
        const Amg::Transform3D& transform(const int tubeLayer, const int tube) const;
        
        virtual const Amg::Vector3D& center(const Identifier&) const override final;
        virtual const Amg::Vector3D& center() const override final;
        const Amg::Vector3D& center(const int tubeLayer, const int tube) const;
        
        virtual const Amg::Vector3D& normal(const Identifier&) const override final;
        virtual const Amg::Vector3D& normal() const override final;

        /** returns all the surfaces contained in this detector element */
        std::vector<const Trk::Surface*> surfaces() const;

       Amg::Transform3D tubeToMultilayerTransf(const Identifier& id) const;       
       Amg::Transform3D tubeToMultilayerTransf(const int tubeLayer, const int tube) const;
        

    private:
        // Called from MuonChamber
        void geoInitDone();

        
        double getTubeLengthForCaching(const int tubeLayer, const int tube) const;
        double getNominalTubeLengthWoCutouts(const int tubeLayer, const int tube) const;
        Amg::Vector3D localNominalTubePosWoCutouts(const int tubelayer, const int tube) const;
         // methods handling deformations
        const Amg::Transform3D& fromIdealToDeformed(const int tubelayer, const int tube) const;
        

        Amg::Vector3D posOnDefChamWire(const Amg::Vector3D& locAMDBPos,
                                       const double width_narrow,
                                       const double width_wide,
                                       const double height, 
                                       const double thickness,
                                       const Amg::Vector3D& fixedPoint) const;
        void wireEndpointsAsBuilt(Amg::Vector3D& locAMDBWireEndP, Amg::Vector3D& locAMDBWireEndN, const int tubelayer,
                                  const int tube) const;

        const MdtIdHelper& m_idHelper{idHelperSvc()->mdtIdHelper()};
        const int m_stIdx_BIS{m_idHelper.stationNameIndex("BIS")};
        const int m_stIdx_BOL{m_idHelper.stationNameIndex("BOL")};

        unsigned int m_nMDTinStation{0};
        int m_multilayer{0};
        int m_nlayers{-1};
        double m_tubepitch{-9999.};
        double m_tubelayerpitch{-9999.};
        int m_ntubesperlayer{-1};
        int m_nsteps{-1};
        int m_ntubesinastep{-1};
        double m_tubelenStepSize{-9999.};
        double m_cutoutShift{-9999.};
        double m_endpluglength{-9999.};
        double m_deadlength{-9999.};
        bool m_inBarrel{false};
        std::array<double, maxnsteps> m_tubelength{make_array<double, maxnsteps>(-9999.)};
        std::array<double, maxnlayers> m_firstwire_x{make_array<double, maxnlayers>(-9999.)};
        std::array<double, maxnlayers> m_firstwire_y{make_array<double, maxnlayers>(-9999.)};
        double m_innerRadius{-9999.};
        double m_tubeWallThickness{-9999.};
        CxxUtils::CachedValue<int> m_zsignRO_tubeFrame{};  // comes from AMDB CRO location in the station

        Amg::Transform3D globalTransform(const Amg::Vector3D& tubePos, 
                                        const Amg::Transform3D& toDeform) const;
        
        
        static Amg::Transform3D tubeToMultilayerTransf(const Amg::Vector3D& tubePos, 
                                                const Amg::Transform3D& toDeform) ;
       
       
        struct GeoInfo {
            GeoInfo(const Amg::Transform3D& transform) : m_transform(transform), m_center(transform.translation()) {}
            Amg::Transform3D m_transform{Amg::Transform3D::Identity()};
            Amg::Vector3D m_center{Amg::Vector3D::Zero()};
        };
        std::unique_ptr<GeoInfo> makeGeoInfo(const int tubelayer, const int tube) const;
        const GeoInfo& geoInfo(const int tubeLayer, const int tube) const;
        Amg::Transform3D deformedTransform(const int tubelayer, const int tube) const;

        std::vector<CxxUtils::CachedUniquePtr<GeoInfo> > m_tubeGeo{};                      // one per tube
        std::vector<CxxUtils::CachedUniquePtr<GeoInfo> > m_backupTubeGeo{};                // one per tube
        std::vector<CxxUtils::CachedUniquePtr<Amg::Transform3D> > m_deformTransf{};        // one per tube

        const BLinePar* m_BLinePar{nullptr};
        CxxUtils::CachedValue<Amg::Vector3D> m_elemNormal{};                               // one
        std::vector<CxxUtils::CachedUniquePtr<Trk::SaggedLineSurface> > m_tubeSurfaces{};  // one per tube
        std::vector<CxxUtils::CachedUniquePtr<Trk::CylinderBounds> > m_tubeBounds{};       // one per step in tube-length

        /// Flag whether any elements have been inserted
        /// into the corresponding vectors.
        /// Used to speed up the clear-cache operations for the case where
        /// the vectors are empty.
        mutable std::atomic<bool> m_haveTubeSurfaces{false};
        mutable std::atomic<bool> m_haveTubeGeo{false};
        mutable std::atomic<bool> m_haveTubeBounds{false};
        mutable std::atomic<bool> m_haveDeformTransf{false};

        // the single surface information representing the DetElement
        CxxUtils::CachedUniquePtr<Trk::Surface> m_associatedSurface{};
        CxxUtils::CachedUniquePtr<Trk::SurfaceBounds> m_associatedBounds{};
    };

}  // namespace MuonGM
#include <MuonReadoutGeometry/MdtReadoutElement.icc>
#endif  // MUONREADOUTGEOMETRY_MDTREADOUTELEMENT_H
