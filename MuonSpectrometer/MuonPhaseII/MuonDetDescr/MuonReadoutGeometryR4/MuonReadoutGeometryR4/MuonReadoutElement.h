/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELR4_MUONREADOUTELEMENT_H
#define MUONGEOMODELR4_MUONREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
////
#include <AthenaBaseComps/AthMessaging.h>
#include <GaudiKernel/ServiceHandle.h>
#include <GeoModelKernel/GeoVDetectorElement.h>
#include <GeoModelKernel/GeoAlignableTransform.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

#include <ActsGeoUtils/TransformCache.h>
#include <ActsGeoUtils/SurfaceCache.h>
#include <ActsGeometryInterfaces/IDetectorElement.h>

#include <GeoModelUtilities/TransientConstSharedPtr.h>

#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/LineBounds.hpp"
#   include "Acts/Surfaces/PlanarBounds.hpp"
#endif
namespace MuonGMR4 {

class MuonChamber;

///   The MuonReadoutElement is an abstract class representing the geometry
///   representing the muon detector. The segmentation of the detectors varies
///   along the MS subsystems and is documented further in the specific sub
///   detector classes. As rule of thumb, detectors sitting on at a different MS
///   layer or station Index, stationEta or station phi are represented by at
///   least one MuonReadoutElement. The MuonReadout elements are constructed
///   from the RAW geometry provided by GeoModelSvc. The class below is pure
///   virtual and implements the minimal set of methods shared by all muon
///   detector technolgies.
class MuonReadoutElement : public GeoVDetectorElement, public AthMessaging, public ActsTrk::IDetectorElement {

   public:
    /// Helper struct to ship the defining arguments of the detector element
    /// around
    struct defineArgs {
        /// Pointer to the underlying physical volume in GeoModel
        GeoIntrusivePtr<GeoVFullPhysVol> physVol{nullptr};
        /// Pointer to the alignable transformation 
        const GeoAlignableTransform* alignTransform{nullptr};
        /// chamber design name as it's occuring in the parameter book tables E.g. BMS5, RPC10, etc.
        std::string chambDesign{""};
        /// ATLAS identifier
        Identifier detElId{0};        
    };
    

    MuonReadoutElement(defineArgs&& args);
    MuonReadoutElement()=delete;
    MuonReadoutElement(const MuonReadoutElement&)=delete;
    
    /// Element initialization
    virtual StatusCode initElement() = 0;
    /// Returnsthe alignable transform of the readout element
    const GeoAlignableTransform* alignableTransform() const;
    /// Return the athena identifier.
    ///  The Identifier is identical with the first measurment channel in
    ///  readout element (E.g. Strip 1 in Layer 1 in the NSW)
    Identifier identify() const override final;

    /// Returns the Identifier has of the Element that is Identical to the
    /// detElHash from the id_helper class
    IdentifierHash identHash() const;
    /// Returns the stationName (BIS, BOS, etc) encoded into the integer
    int stationName() const;
    /// Returns the stationEta (positive A site, negative O site)
    int stationEta() const;
    /// Returns the stationPhi (1-8) -> sector (2*phi - (isSmall))
    int stationPhi() const;
    /// Returns the chamber index of the Identifier (MMS & STS) have the same
    /// chamber Index (EIS)
    Muon::MuonStationIndex::ChIndex chamberIndex() const;

    /// Constructs the identifier hash from the full measurement Identifier. The
    /// hash is always defined w.r.t the specific detector element and used to
    /// access the information in memory quickly
    virtual IdentifierHash measurementHash(const Identifier& measId) const = 0;

    virtual IdentifierHash layerHash(const Identifier& measId) const = 0;
    /// Converts the measurement hash back to the full Identifier
    virtual Identifier measurementId(const IdentifierHash& measHash) const = 0;


    /// The chamber design refers to the construction parameters of a readout
    /// element. Used for the retrieval of the chamber parameters
    ///   E.g. the chambers BOL1A8 & BOL2A8 are identical in terms of number of
    ///   tubes, dimensions etc.
    std::string chamberDesign() const;

    /// Returns the pointer to the muonIdHelperSvc
    const Muon::IMuonIdHelperSvc* idHelperSvc() const;

    /// Returns the detector center (Which is the same as the detector center of
    /// the first measurement layer)
    Amg::Vector3D center(const ActsGeometryContext& ctx) const;
    /// Returns the center of a given detector layer using the complete
    /// Identifier of the measurement
    Amg::Vector3D center(const ActsGeometryContext& ctx,
                         const Identifier& id) const;
    /// Returns the center of a given detector layer using the Identifier hash
    /// of the measurement
    Amg::Vector3D center(const ActsGeometryContext& ctx,
                         const IdentifierHash& hash) const;

    ///   Transformations to translate between local <-> global coordinates.
    ///   They follow the common ATLAS conventations that the origin is located
    ///   in the center of the detector layer
    ///         x-axis: Points towards the sky
    ///         y-axis: Points towards the edges of ATLAS
    ///         z-axis: Points along the beamline
    ///   The transformations always include the corrections from the A-Lines of
    ///   the alignment system
    /// Returns the global to local transformation into the rest frame of the
    /// detector (Coincides with the first measurement layer)
    Amg::Transform3D globalToLocalTrans(const ActsGeometryContext& ctx) const;
    /// Returns the global to local transformation into the rest frame of a
    /// given measurement layer
    Amg::Transform3D globalToLocalTrans(const ActsGeometryContext& ctx,
                                        const Identifier& id) const;
    /// Returns the global to local transformation into the rest frame of a
    /// given measurement layer
    Amg::Transform3D globalToLocalTrans(const ActsGeometryContext& ctx, 
                                        const IdentifierHash& hash) const;

    /// Returns the local to global transformation into the ATLAS coordinate
    /// system
    const Amg::Transform3D& localToGlobalTrans(const ActsGeometryContext& ctx) const;
    const Amg::Transform3D& localToGlobalTrans(const ActsGeometryContext& ctx,
                                               const Identifier& id) const;
    const Amg::Transform3D& localToGlobalTrans(const ActsGeometryContext& ctx,
                                               const IdentifierHash& id) const;

#ifndef SIMULATIONBASE
    /// Returns the transformation to the origin of the chamber coordinate system
    const Acts::Transform3& transform(const Acts::GeometryContext& gctx) const override final;    
    /// Returns the surface associated to the readout element plane
    const Acts::Surface& surface() const override final;
    Acts::Surface& surface() override final;

    /// Returns the sufrface associated to a wire / measurement plane in the detector
    const Acts::Surface& surface(const IdentifierHash& hash) const;
    Acts::Surface& surface(const IdentifierHash& hash);

    /// Returns the pointer associated to a certain wire / plane
    std::shared_ptr<Acts::Surface> surfacePtr(const IdentifierHash& hash) const;

    /// Set's the link to the MuonChamber object to which the MuonReadoutElement belongs to
    void setChamberLink(GeoModel::TransientConstSharedPtr<MuonChamber> chamber);

    const MuonChamber* getChamber() const;
#else
    /// In AthSimulation there's no Acts::DetectorElement which is declaring this method
    /// in its interface.
    virtual double thickness() const = 0;
#endif
    /// Releases all cached transforms that are not connected with alignment
    void releaseUnAlignedTrfs() const;

    unsigned int storeAlignedTransforms(const ActsTrk::DetectorAlignStore& store) const override final;
   protected:
     using TransformMaker = ActsTrk::TransformCache::TransformMaker;
      
     /// Inserts a transfomration for caching
     StatusCode insertTransform(const IdentifierHash& hash,
                                 TransformMaker make);

     StatusCode insertTransform(const IdentifierHash& hash,
                                std::shared_ptr<const TransformMaker> make);

     StatusCode createGeoTransform();
     /// Returns the transformation into the center of the readout volume
     Amg::Transform3D toStation(const ActsTrk::DetectorAlignStore* alignStore) const;
#ifndef SIMULATIONBASE
     //Creates a MuonSurfaceCache for straw surfaces using the given Bounds and Identifier Hash
     StatusCode strawSurfaceFactory(const IdentifierHash& hash, std::shared_ptr<Acts::LineBounds> lBounds);

     //Creates a MuonSurfaceCache for plane surface using the given Bounds and Identifier Hash
     StatusCode planeSurfaceFactory(const IdentifierHash& hash, std::shared_ptr<Acts::PlanarBounds> pBounds);
#endif     
     /// Returns the hash that is associated with the surface cache holding the transformation that is
     /// placing the ReadoutElement inside the ATLAS coordinate system.
     static IdentifierHash geoTransformHash();
   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        "Muon::MuonIdHelperSvc/MuonIdHelperSvc", "MuonReadoutElement"};

    const defineArgs m_args{};
    /// Cache of the detector element hash
    IdentifierHash m_detElHash{};
    /// Cache the chamber index of the Identifier
    Muon::MuonStationIndex::ChIndex m_chIdx{Muon::MuonStationIndex::ChIndex::ChUnknown};
    /// Cache the station name of the identifier
    int m_stName{-1};
    /// Cache the station eta of the identifier
    int m_stEta{-1};
    /// Cache the station phi of the identifier
    int m_stPhi{-1};

    /// Cache all local to global transformations
    using TransformCacheMap = std::unordered_map<IdentifierHash, std::unique_ptr<ActsTrk::TransformCache>>;
    TransformCacheMap m_localToGlobalCaches{};
#ifndef SIMULATIONBASE
    ///Cache of all associated surfaces
    ActsTrk::SurfaceCacheSet m_surfaces{};
    /// Pointer to the associated MuonChamber
    GeoModel::TransientConstSharedPtr<MuonChamber> m_chambLink{};
#endif
};
}  // namespace MuonGMR4
#include <MuonReadoutGeometryR4/MuonReadoutElement.icc>
#endif
