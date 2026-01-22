/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_MDTREADOUTELEMENT_H
#define MUONREADOUTGEOMETRYR4_MDTREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/MdtTubeLayer.h>

#ifndef SIMULATIONBASE
#   include <MuonAlignmentData/BLinePar.h>
#   include <MuonAlignmentData/MdtAsBuiltPar.h>
#   include "Acts/Utilities/BoundFactory.hpp"
#endif

namespace MuonGMR4 {

/** @brief Readout element to describe the Monitored Drift Tube  (Mdt) chambers
 *         Mdt chambers usually comrpise out of two packs consisting out of 
 *         3 or 4 tube layers which are separated by a space frame 
 *         onto which they are glued.
 *         Each readout element covers the description of one such pack called a multilayer.
 *         The multilayers have an internal numbering of tubes and layers. The first tube 
 *         of the first layer is defined to be closed to the IP. */
class MdtReadoutElement : public MuonReadoutElement {

   public:
        /** @brief Allow the transform cache to call the protecd data members */
        friend ActsTrk::TransformCacheDetEle<MdtReadoutElement>;
        /** @brief Set of parameters to describe a MDT chamber */
        struct parameterBook {
            /** @brief  Vector defining the position of all tubes in each tube layer.
              *         The Size of the vector reflects the number of tube layers in the
              *         multi layer. The number of tubes of the readout element is taken from
              *         he number of tubes of the first layer */
            std::vector<MdtTubeLayerPtr> tubeLayers{};
            /** @brief List of tube slots without tubes (BMG cutouts) */
            std::unordered_set<IdentifierHash> removedTubes{};
            /** @brief Thickness of the tube wall [mm] */
            double tubeWall{0.};
            /** @brief Inner tube radius ofthe actibe gas */
            double tubeInnerRad{0.};
            /** @brief Distance between 2 tubes in a layer */
            double tubePitch{0.};
            /** @brief Tension parameter Used in the SaggedLine surfaces */
            double wireTension{0.};
            /** @brief Depth of the endplug into the active tube volume */
            double endPlugLength{0.};
            /// 
            double deadLength{0.};
            /** @brief  Etimated radiation lenght  */
            double radLengthX0{0.};
            /** @brief The chambers have either a rectangular or a trapezoidal shape to first approximation.
              *        The former is mounted in the barrel while the latter can be found on the middle and outer
              *        big wheels. In Run 1 & Run 2, the inner wheel also consistet of MDT chambers.
              *        The local coordinate system is placed in the center of the chamber and the x-axis
              *        is parallel to the long & short edges of the trapezoid as illustrated in
              *        https://gitlab.cern.ch/atlas/athena/-/blob/master/docs/images/TrapezoidalBounds.gif
              *        For the rectengular barrel chambers, the X length is read from longHalfX */
            double shortHalfX{0.};
            double longHalfX{0.};
            /** @brief Length of the chamber ~ number of tubes */
            double halfY{0.};
            /** @brief Height of the chamber ~ number of layers  */
            double halfHeight{0.};
            /** @brief Is the readout chip at positive or negative Z in 
              * the tube coordinate frame  */
            double readoutSide{1.};
            /// Sets of surface bounds which is shared amongst all readout elements used
            /// to assign the same bound objects if 2 surfaces share the same dimensions.
    #ifndef SIMULATIONBASE
            std::shared_ptr<Acts::SurfaceBoundFactory> boundFactory;
    #endif

        };
        /** @brief Declare the define args as concatination of the 
         *         parameters to describe the chamber and the 
         *         defineArgs from the MuonReadoutElement holding
         *         the Identifier && GeoModel objects */
        struct defineArgs : public MuonReadoutElement::defineArgs,
                            public parameterBook {};
        /** @brief Constructor with the define arguments */
        MdtReadoutElement(defineArgs&& args);
        /** @brief Destructor */
        virtual ~MdtReadoutElement();
        /** @brief Get a const reference to the parameter book */
        const parameterBook& getParameters() const;
        /** @brief Overload the detector type */
        ActsTrk::DetectorType detectorType() const override final {
            return ActsTrk::DetectorType::Mdt;
        }
        /** @brief Overload from the Acts::DetectorElement (2 * halfheight) */
        double thickness() const override final;
        /** @brief Overload from MuonReadoutElement */
        StatusCode initElement() override final;
        /** @brief Returns the multi layer of the readout element [1;\2] */
        unsigned multilayer() const;
        /** @brief Returns how many tube layers are inside the multi layer [1;4] */
        unsigned numLayers() const;
        /** @brief Returns the number of tubes in a layer */
        unsigned numTubesInLay() const;
        /** @brief Constructs a Measurement hash from layer && tube number
         *  @param layerNumber: Number of the tube layer [1;numLayers()]
         *  @param tubenumber: Number of the tube inside the layer [1;numTubesInLay()] */
        static IdentifierHash measurementHash(unsigned layerNumber, unsigned tubeNumber);
        /** @brief Transforms the measurement hash into a tube number ranging
         *         from [0; numTubeInLay() -1] 
         *  @param hash: Measurement hash to transform*/
        static unsigned tubeNumber(const IdentifierHash& hash);
        /** @brief Transforms the measurement hash into a tube-layer number ranging
         *         from [0; numLayers() -1] 
         *  @param hash: Measurement hash to transform*/
        static unsigned layerNumber(const IdentifierHash& hash);
        /** @brief Constructs the measurement hash from the full measurement Identifier. The
          * hash is always defined w.r.t the specific detector element and used to
          * access the information in memory quickly
          * @param measId: Identifier of an associated Mdt measurement  */
        IdentifierHash measurementHash(const Identifier& measId) const override final;
        /** @brief Projects the measurement hash onto the layerHash mainly used to 
          *        access the plane surface in the center of each tube layer
          * @param measId: Identifier of an associated Mdt measurement */
        IdentifierHash layerHash(const Identifier& measId) const override final;
        /** @brief Static method to tranform a measurement hash into a layer hash
         *  @param measHash: Measurement corresponding to this readout element */
        static IdentifierHash layerHash(const IdentifierHash& measHash);
        /** @brief Back conversion of the measurement hash towards a full identifier
         *         Tube & layer number are extracted from the hash and combined with
         *         the mising Identifier fields from the readout element
         *  @param measHash: Measurement hash to transform */
        Identifier measurementId(const IdentifierHash& measHash) const override final;
        /** @brief Checks whether the passed meaurement hash corresponds
         *         to a valid tube described by the readout element
         *  @param measHash: Associated measurement hash encoding tube & layer number */
        bool isValid(const IdentifierHash& measHash) const;
        /** @brief States whether the chamber is built into the barrel or not */
        bool isBarrel() const;
        /** @brief Returns the pitch between 2 tubes in a layer */
        double tubePitch() const;
        /** @brief Returns the inner tube radius */
        double innerTubeRadius() const;
        /** @brief Adds the thickness of the tube wall onto the radius */
        double tubeRadius() const;
        /** @brief Returns the length of the bottom edge of the chamber (short width) */
        double moduleWidthS() const;
        /** @brief Returns the length of the top edge of the chamber (top width) */
        double moduleWidthL() const;
        /** @brief Returns the height of the chamber (Distance bottom - topWidth)  */
        double moduleHeight() const;
        /** @brief Returns the thickness of the chamber */
        double moduleThickness() const;
        /** @brief Returns the position of the tube mid point in the
         *         ATLAS coordinate frame
         *  @param ctx: Geometry context carrying the current alignment
         *  @param measId: Identifier of the tube for which the position
         *                 is to be retrieved */
        Amg::Vector3D globalTubePos(const ActsTrk::GeometryContext& ctx,
                                    const Identifier& measId) const;
        /** @brief Returns the position of the tube mid point in the
         *         ATLAS coordinate frame
         *  @param ctx: Geometry context carrying the current alignment
         *  @param hash: Measurement hash of the tube for which the position
         *               is to be retrieved */
        Amg::Vector3D globalTubePos(const ActsTrk::GeometryContext& ctx,
                                    const IdentifierHash& hash) const;

        /** @brief Returns the endpoint of the tube where the readout card
         *         is mounted in the ATLAS coordinate frame
         *  @param ctx: Geometry context carrying the current alignment
         *  @param measId: Identifier of the tube for which the position
         *                 is to be retrieved */
        Amg::Vector3D readOutPos(const ActsTrk::GeometryContext& ctx,
                                const Identifier& measId) const;
        /** @brief Returns the endpoint of the tube where the readout card
         *         is mounted in the ATLAS coordinate frame
         *  @param ctx: Geometry context carrying the current alignment
         *  @param hash: Measurement hash of the tube for which the position
         *               is to be retrieved */
        Amg::Vector3D readOutPos(const ActsTrk::GeometryContext& ctx,
                                const IdentifierHash& hash) const;

        /** @brief Returns the endpoint of the tube connected to the 
         *         high voltage in the ATLAS coordinate frame
         *  @param ctx: Geometry context carrying the current alignment
         *  @param measId: Identifier of the tube for which the position
         *                 is to be retrieved */
        Amg::Vector3D highVoltPos(const ActsTrk::GeometryContext& ctx,
                                const Identifier& measId) const;
        /** @brief Returns the endpoint of the tube connected to the 
         *         high voltage in the ATLAS coordinate frame
         *  @param ctx: Geometry context carrying the current alignment
         *  @param hash: Measurement hash of the tube for which the position
         *               is to be retrieved */
        Amg::Vector3D highVoltPos(const ActsTrk::GeometryContext& ctx,
                                const IdentifierHash& hash) const;
        /** @brief Returns the distance to the readout card along the wire
         *  @param ctx: Geometry context carrying the current alignment
         *  @param measId: Identifier of the tube for which the distance
         *                 is to be calculated
         *  @param globPoint: External point in the ATLAS coordinate frame */
        double distanceToReadout(const ActsTrk::GeometryContext& ctx,
                                const Identifier& measId,
                                const Amg::Vector3D& globPoint) const;
        /** @brief Returns the distance to the readout card along the wire
         *  @param ctx: Geometry context carrying the current alignment
         *  @param measHash: Measurement hash of the tube for which the distance
         *                   is to be calculated
         *  @param globPoint: External point in the ATLAS coordinate frame */
        double distanceToReadout(const ActsTrk::GeometryContext& ctx,
                                const IdentifierHash& measHash,
                                const Amg::Vector3D& globPoint) const;
        /** @brief Returns the distance to the readout assuming that the parsed point is expressed 
         *         in the local coordinate system of the tube. I.e. the wire points along the z-axis
         * @param measHash: IdentifierHash of the tube
         * @param localPoint: External local point inside the tube (No check is made whether that's the case). */
        double distanceToReadout(const IdentifierHash& measHash,
                                const Amg::Vector3D& localPoint) const;

        double activeTubeLength(const IdentifierHash& hash) const;
  
        double tubeLength(const IdentifierHash& hash) const;
        
        double wireLength(const IdentifierHash& hash) const;
        /** @brief Returns the uncut tube length */
        double uncutTubeLength(const IdentifierHash& tubeHash) const;


        /** @brief Set the link to the second readout element inside the  muon station. 
         *  @param other: pointer to the readoutElement */
        void setComplementaryReadoutEle(const MdtReadoutElement* other);
        /** @brief Returns the fixed point of the B-line & as-bult defromation 
         *         model expressed in the as-built frame. */
        Amg::Vector3D bLineReferencePoint() const;

        /// Returns the tube position in the chamber coordinate frame (Not applying the B-line corrections)
        Amg::Vector3D localTubePos(const IdentifierHash& hash) const;
        /** @brief Returns the transformation to go into the reference frame of the as-buit & b-line model
                   starting from the readout element frame. */
        Amg::Transform3D asBuiltRefFrame() const;
        /** @brief Returns the pointer to the other readout element inside the muon station */
        const MdtReadoutElement* complementaryRE() const;
    private:
        /// Returns the transformation into the rest frame of the tube
        /// x-axis: Pointing towards the next layer
        /// y-axis: Pointing parallel to the wire layer 
        /// z-axis: Pointing along the wire
        Amg::Transform3D toChamberLayer(const IdentifierHash& hash) const;
        /// Returns the transformation into the rest frame of the tube
        /// x-axis: Pointing towards the next layer
        /// y-axis: Pointing parallel to the wire layer 
        /// z-axis: Pointing along the wire
        Amg::Transform3D toTubeFrame(const IdentifierHash& hash) const;
        /// Applies the B & as-built parameters
        Amg::Transform3D fromIdealToDeformed(const IdentifierHash& tubeHash,
                                             const ActsTrk::DetectorAlignStore* store) const;
                                             
 #ifndef SIMULATIONBASE
        /** @brief Moves the wire endpoints according to the as-built model
         *  @param asBuilt: As-built model parameters
         *  @param tubeHash: Measurement hash of the considered tube
         *  @param nominalEnd: Tube end of a nominally uncut tube
         *  @param side: Does the end represent positive or negative z
        */ 
        using tubeSide_t = MdtAsBuiltPar::tubeSide_t;
        Amg::Vector3D wireEndpointAsBuilt(const MdtAsBuiltPar&  asBuilt,
                                           const IdentifierHash& tubeHash,
                                           const Amg::Vector3D& nominalEnd, 
                                           const tubeSide_t side) const;
        
        /** @brief Apply the B-line model correction to a tube endpoint
         *  @param bline: Set of b-line parameters
         *  @param localTubeEndPoint: Endpoint of the tube to correct
         *  @param fixedPoint: Point in the chamber that's invariant in the b-line model
         *  @param thickness: Thickness of the two multilayers */
        Amg::Vector3D applyBlineCorrections(const BLinePar& bline,
                                            const Amg::Vector3D& localTubeEndPoint,
                                            const Amg::Vector3D& fixedPoint,
                                            const double thickness) const;

#endif        
        /** @brief defining parameter set */
        parameterBook m_pars{};
        /** @brief Detector identifier helper to quickly extract the ID fields */
        const MdtIdHelper& m_idHelper{idHelperSvc()->mdtIdHelper()};
        /// Identifier index of the multilayer (1-2)
        int m_stML{m_idHelper.multilayer(identify())};
        /// Flag defining whether the chamber is barrel or not
        bool m_isBarrel{m_idHelper.isBarrel(identify())};
        /** @brief Complementary readout element. */
        const MdtReadoutElement* m_reOtherMl{this};
};

std::ostream& operator<<(std::ostream& ostr, const MdtReadoutElement::parameterBook& pars);
}  // namespace MuonGMR4

namespace ActsTrk{
    template <> Amg::Transform3D 
        TransformCacheDetEle<MuonGMR4::MdtReadoutElement>::fetchTransform(const DetectorAlignStore* store) const;
    template <> Identifier
        TransformCacheDetEle<MuonGMR4::MdtReadoutElement>::identify() const;
}

#include <MuonReadoutGeometryR4/MdtReadoutElement.icc>
#endif
