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

#include <ActsGeoUtils/Defs.h>
#include <ActsGeometryInterfaces/IDetectorElement.h>
#include <ActsGeoUtils/TransformCache.h>
#include <GeoModelUtilities/TransientConstSharedPtr.h>

namespace Acts{
    class Surface;
    class LineBounds;
    class PlanarBounds;
}
namespace MuonGMR4 {

class SpectrometerSector;
class Chamber;
/** @brief MuonReadoutElement is an abstract class representing the geometry of a muon detector. 
  *   The segmentation of the detectors varies along the MS subsystems and is documented 
  *   further in the specific sub-detector classes. As rule of thumb, detectors sitting 
  *   at a different MS layer or station Index, stationEta or station phi are represented by at
  *   least one MuonReadoutElement. The MuonReadoutElements are constructed from the RAW geometry 
  *   provided by GeoModelSvc. The class below is pure virtual and implements the minimal set
  *   of methods shared by all detector technolgies. */
class MuonReadoutElement : public GeoVDetectorElement, public AthMessaging, public ActsTrk::IDetectorElement {
   public:
    /** @brief Helper struct to ship the defining arguments of the detector element  */
    struct defineArgs {
        /** @brief Pointer to the underlying physical volume in GeoModel  */
        GeoIntrusivePtr<const GeoVFullPhysVol> physVol{nullptr};
        /** @brief Pointer to the alignable transform node upstream */
        GeoIntrusivePtr<const GeoAlignableTransform> alignTransform{nullptr};
        /** @brief chamber design name as it's occuring in the parameter book tables E.g. BMS5, RPC10, etc. */
        std::string chambDesign{""};
        /** @brief ATLAS detector element identifier (First channel of the first readout layer) */
        Identifier detElId{};        
    };
    
    /** @brief Constructor taking the basic define arguments */
    MuonReadoutElement(const defineArgs& args);
    virtual ~MuonReadoutElement();
    
    /** @brief Delete the copy constructor */
    MuonReadoutElement(const MuonReadoutElement&) = delete;
    /** @brief Delete the copy assignment */
    MuonReadoutElement& operator=(const MuonReadoutElement&) = delete;
    /** @brief Delete the move constructor */
    MuonReadoutElement(MuonReadoutElement&&) = delete;
    /** @brief Delete the move assignment */
    MuonReadoutElement& operator=(MuonReadoutElement&&) = delete;
    
    /** @brief Initialization of the readout elements. Transforms & surfaces
     *         for each readout layer are created */
    virtual StatusCode initElement() = 0;
    /** @brief Return the alignable transform node of the readout element */
    const GeoAlignableTransform* alignableTransform() const;
    /** @brief Return the ATLAS identifier */
    Identifier identify() const override final;
    /** @brief Returns the hash of the readout element which is identical to the 
     *         detector element hash provided by the associated idHelper */
    IdentifierHash identHash() const;
    /** @brief Returns the stationName (BIS, BOS, etc) encoded into the integer */
    int stationName() const;
    /** @brief Returns the stationEta (positive A site, negative C site) */
    int stationEta() const;
    /** @brief Returns the stationPhi (1-8) -> sector (2*phi - (isSmall)) */
    int stationPhi() const;
    /** @brief Returns the chamber index of the Identifier (MMS & STS) have the same
               chamber Index (EIS) */
    Muon::MuonStationIndex::ChIndex chamberIndex() const;
    /** @brief The measurement hash is a continous numbering schema of 
     *         all readout channels described by the specific MuonReadoutElement
     *         instance. It's always defined w.r.t. the MuonReadoutElement and
     *         has no meaning or wrong meaning without it.
     *         This methods convert the identifier to a measurement hash
     * @param measId: Identifier of the measurement to convert */ 
    virtual IdentifierHash measurementHash(const Identifier& measId) const = 0;
    /** @brief The layer hash removes the bits from the IdentifierHash corresponding
     *         to the measurement's channel number and sets them to zero. It's used 
     *         to access the transform associated to the measurement's strip plane
     *         or to the tube layer */
    virtual IdentifierHash layerHash(const Identifier& measId) const = 0;
    /** @brief Back conversion of the measurement hash to a full Athena Identifier
     *         The behaviour is undefined if a layer hash is parsed
     *  @param measHash: Measurement hash to convert */
    virtual Identifier measurementId(const IdentifierHash& measHash) const = 0;
    /** @brief The chamber design refers to the construction parameters of
     *          a readout element. It's used for the retrieval of the chamber meta data
     *          containing the information about the number of sensors, their sepration etc. */
    const std::string& chamberDesign() const;
    /** @brief Returns the pointer to the muonIdHelperSvc */
    const Muon::IMuonIdHelperSvc* idHelperSvc() const;

    /** @brief Returns the geometrical center point of the readout element
     *  @param ctx: Geometry context to take the alignment corrections into account */
    Amg::Vector3D center(const ActsTrk::GeometryContext& ctx) const;
    /** @brief Returns the origin of the readout element's transform
     *  @param ctx: Geometry context to take the alignment corrections into account
     *  @param id: Identifier of the measurement channel to be retrieved */
    Amg::Vector3D center(const ActsTrk::GeometryContext& ctx,
                         const Identifier& id) const;
    /** @brief Returns the origin of the readout element's transform
     *  @param ctx: Geometry context to take the alignment corrections into account
     *  @param hash: Measurement hash of the transform to be retrieved */
    Amg::Vector3D center(const ActsTrk::GeometryContext& ctx,
                         const IdentifierHash& hash) const;
    /** @brief Returns the transformation from the global ATLAS coordinate system
     *         into the local coordinate system of the readout element. The local axes
     *         are oriented such that
     *              x-axis: Is parallel to the sensors measuring the eta coordinate
     *                      (e.g. parallel to the Mdt tube wire)
     *              y-axis: Is parallel to the sensors measuring the phi coordinate
     *                      (e.g. to the next tube wire)
     *              z-axis: Points vertically outwards the chamber
     *                      (e.g. radially outwards for barrel or to the endcap cavern wall)
     *  @param ctx: Geometry context to take the alignment corrections into account */
    Amg::Transform3D globalToLocalTransform(const ActsTrk::GeometryContext& ctx) const;
    /** @brief Returns the transformations from the ATLAS coordinate system into the
     *         local coordinate system of the readout sensor. The orientiation of the
     *         axes depends on whether the sensor is described by a plane or by a
     *         tube wire.
     *         In the former case, the axes are oriented such that
     *             x-axis: Points to the next eta sensor such that local-x always
     *                     constrains the precision coordinate
     *             y-axis: Points to the next phi sensor such that local y always
     *                     constains the coordinate along the precision sensor
     *             z-axis: Is the cross-product of the other two
     *        For Mdt tubes the axis orientiation is such that
     *              x-axis: Points to the next tube in the same tube-layer
     *              y-axis: Points to the next tube-layer plane
     *              z-axis: Points along the tube wire
     *  @param ctx: Geometry context to take the alignment corrections into account
     *  @param id: Identifier of the measurement for which the transform shall be retrieved */
    Amg::Transform3D globalToLocalTransform(const ActsTrk::GeometryContext& ctx,
                                            const Identifier& id) const;
    /** @brief Returns the transformations from the ATLAS coordinate system into the
     *         local coordinate system using the measurement / layer hash mechanism.
     *         For strip-like detectors a layer hash must always be parsed. For the 
     *         Mdts a measurement or layer hash can be parsed depending on whether the
     *         plane transform or the particular tube transform shall be retrieved.
     *  @param ctx: Geometry context to take the alignment corrections into account.
     *  @param hash: Hash of the transform to fetch (Measurement or layer hash). */
    Amg::Transform3D globalToLocalTransform(const ActsTrk::GeometryContext& ctx, 
                                            const IdentifierHash& hash) const;
    /** @brief Returns the transformation from the local coordinate system  of the readout
     *         element into the global ATLAS coordinate system (inverse of globalToLocal).
     *  @param ctx: Geometry context to take the alignment corrections into account. */
    const Amg::Transform3D& localToGlobalTransform(const ActsTrk::GeometryContext& ctx) const;
    /** @brief Returns the transformation from the local coordinate system  of the readout
     *         element into the global ATLAS coordinate system (inverse of globalToLocal).
     *  @param ctx: Geometry context to take the alignment corrections into account
     *  @param id: Identifier of the measurement for which the transform shall be retrieved */
    const Amg::Transform3D& localToGlobalTransform(const ActsTrk::GeometryContext& ctx,
                                                   const Identifier& id) const;
    /** @brief Returns the transformation from the local coordinate system  of the readout
     *         element into the global ATLAS coordinate system (inverse of globalToLocal).
     *  @param ctx: Geometry context to take the alignment corrections into account
     *  @param hash: Hash of the transform to fetch (Measurement or layer hash). */
    const Amg::Transform3D& localToGlobalTransform(const ActsTrk::GeometryContext& ctx,
                                                   const IdentifierHash& id) const;

#ifndef SIMULATIONBASE
    /** @brief Wrapper function of the localToGlobalTransform method to satisfy the 
     *         Acts::IDetectorElementBase interface
     *  @param gctx: Acts representation of the GeometryContext */
    const Amg::Transform3D& transform(const Acts::GeometryContext& gctx) const override final;
    /** @brief Returns the surface associated with the readout element. It is placed in the 
     *         center of the readout element's volume and has the volumes surface bounds */
    const Acts::Surface& surface() const override final;
    /** @brief Returns the mutable surface associated with the readout element. */
    Acts::Surface& surface() override final;
    /** @brief Returns the surface associated with the transform of a given
      *         readout layer. (E.g. tube or  the strip plane)
      * @param hash: Hash of the surface to fetch (Measurement or layer hash). */
    const Acts::Surface& surface(const IdentifierHash& hash) const;
    /** @brief Returns the mutable surface associated with the transform of a given
      *         readout layer. (E.g. tube or  the strip plane)
      * @param hash: Hash of the surface to fetch (Measurement or layer hash). */
    Acts::Surface& surface(const IdentifierHash& hash);
    /** @brief Returns the mutable surface pointer associated with the transform of a given
      *         readout layer. (E.g. tube or  the strip plane)
      * @param hash: Hash of the surface to fetch (Measurement or layer hash). */
    std::shared_ptr<Acts::Surface> surfacePtr(const IdentifierHash& hash) const;
    /** @brief Returns all surfaces that are associated with the active readout planes */
    std::vector<std::shared_ptr<Acts::Surface>> getSurfaces() const;
    /** @brief Sets the link to the enclosing chamber */
    void setChamberLink(const Chamber* chamber);
    /** @brief Set the link to the enclosing sector envelope */
    void setSectorLink(const SpectrometerSector* envelope);
    /** @brief Returns the pointer to the envelope volume enclosing all chambers in the sector */
    const SpectrometerSector* msSector() const;
    /** @brief Returns the pointer to the chamber enclosing this readout element */
    const Chamber* chamber() const;
#else
    /** @brief AthSimulation does not compile Acts and hence there's no interface declared
     *         for the thickness method which is implemented by each detector technology
     *         To keep the override in both cases declare the dummy for AthSimulation only */
    virtual double thickness() const = 0;
#endif
    /** @brief Release all transforms from the memory that are not connected with a geometry context
     *         but cached by the readout element itself */
    void releaseUnAlignedTrfs() const;
    /** @brief Construct the final aligned transformations and store them in the alignment store.
     *         Returns the number of how many transformations have been stored */
    unsigned int storeAlignedTransforms(const ActsTrk::DetectorAlignStore& store) const override final;
    /** @brief Allow the transform cache access to the private / protected data members */
    friend class ActsTrk::TransformCacheDetEle<MuonGMR4::MuonReadoutElement>;
  protected:
    /** @brief Returns the transformation from the GeoModel tree and applies the A-lines if 
     *         a valid alignment store pointer is provided. The local coordinate system in GeoModel
     *         differs from the system used by the localToGlobaTransformations. It is referred to the
     *         AMDB coordinate system used to describe the MS in Run 1-3
     *                 x-axis: Points along the thickness of the readout element
     *                          (e.g. radially outwards for barrel chambers or along global Z 
     *                                for detector mounted in the endcaps)
     *                 y-axis: Points along the edge which is parallel to the eta sensors
     *                 z-axis: Points along towards the next eta sensor */
    const Amg::Transform3D& toStation(const ActsTrk::DetectorAlignStore* alignStore) const;
      
    /** @brief Constructs the TransformDetEleCache associated with the hash of the 
     *         given Mdt tube or strip layer. The method is templated over the specific
    *         implementation of the readout element as the `TransformCacheDetEle` implements
    *         the assembly of the final transforms. The method returns a failure of an instance
    *         for the same hash has already been invoked earlier
    *  @param hash: Measurement / layer hash to identifier the transform of the readout layer */
    template <class MuonDetImpl> 
            StatusCode insertTransform(const IdentifierHash& hash);
    /** @brief Creates the `TransformCacheDetEle` corresponding the generic local -> global transformation
     *         of the readout element. Needs to be called by each technology during initialization */
    StatusCode createGeoTransform();
#ifndef SIMULATIONBASE
    /** @brief Invokes the factory to create straw surfaces && to associate them with the particular transform cache
     *  @param hash: Measurement hash of the tube of interest
     *  @param lBounds: Surface bound object describing the straw radius and the active tube length */
    StatusCode strawSurfaceFactory(const IdentifierHash& hash, std::shared_ptr<const Acts::LineBounds> lBounds);
    /** @brief Invokes the factory to create plane surfaces && to associate them with the particular transform cache
     *  @param hash: Layer hash of the readout plane of interest
     *  @param lBounds: Bounds describing the rectangle or trapezoidal surface's dimensions */
    StatusCode planeSurfaceFactory(const IdentifierHash& hash, std::shared_ptr<const Acts::PlanarBounds> pBounds);
#endif     
     /// Returns the hash that is associated with the surface cache holding the transformation that is
     /// placing the ReadoutElement inside the ATLAS coordinate system.
     static IdentifierHash geoTransformHash();
   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{"Muon::MuonIdHelperSvc/MuonIdHelperSvc", "MuonReadoutElement"};

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
    TransformCacheMap m_localToGlobalCaches;
#ifndef SIMULATIONBASE
    ///Cache of all associated surfaces
    ActsTrk::SurfaceCacheSet m_surfaces;
    /// Pointer to the associated MS-sector & MuonChamber
    const SpectrometerSector* m_msSectorLink{};
    const Chamber* m_chambLink{nullptr};
#endif
};
}  // namespace MuonGMR4

namespace ActsTrk{
    template <> Amg::Transform3D 
        TransformCacheDetEle<MuonGMR4::MuonReadoutElement>::fetchTransform(const DetectorAlignStore* store) const;
}

#include <MuonReadoutGeometryR4/MuonReadoutElement.icc>
#endif
