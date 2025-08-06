/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_STRIPLAYER_H
#define MUONREADOUTGEOMETRYR4_STRIPLAYER_H

#include <GeoPrimitives/GeoPrimitives.h>
#include <MuonReadoutGeometryR4/StripDesign.h>
#include <GeoModelUtilities/TransientConstSharedPtr.h>

#include <GeoModelKernel/GeoTransform.h>
#include <GaudiKernel/SystemOfUnits.h>

namespace MuonGMR4{
    /** @brief The StripLayer interfaces the 2D description of the strip plane layout with
     *         the 3D description of the strips within the read out elements volume. 
     *         It places the strips inside the readout volume which is further transformed
     *         by the ReadoutElement to a global placement of the strip within ATLAS. */
    class StripLayer {
        public:
          using TransformPtr = GeoIntrusivePtr<const GeoTransform>;
          /** @brief Standard constructor taking the transform to decribe a strip layer, 
           *         a pointer to the eta strip design and the associated hash for internal
           *         identification of the StripLayer by the Readoutelement.
           * @param layerTransform: Pointer to the Transform to position the strip plane within the
           *                        readout element's frame.
           * @param design: Pointer to the StripDesign describing the strip layout in the plane
           * @param hash: IdentifierHash to uniquely Identify the layer within a readout element. */
          StripLayer(TransformPtr layerTransform,
                     StripDesignPtr design,
                     const IdentifierHash hash);
          /** @brief Constructor taking the transform to position the strip layer, but taking
           *         two StripDesign pointers to describe the orthogonal strips within the same
           *         coordinate system. The transform is assumed to be aligned with the eta measurement.
           * @param etaDesign: Pointer to the StripDesign describing the layout in eta direction
           * @param phiDesign: Pointer to the StripDesign describing the layout in phi direction 
           * @param hash: IdentifierHash to uniquely Identify the layer within a readout element. */
          StripLayer(TransformPtr layerTransform,
                     StripDesignPtr etaDesign,
                     StripDesignPtr phiDesign,
                     const IdentifierHash hash);

          /// Returns the transformation to go from the strip layer center 
          /// to the origin of the Strip chamber
          const Amg::Transform3D& toOrigin() const;           
          /** @brief Returns the underlying strip design.
           *  @param phiView: If the strip layer holds two designs, the one mapping the phi 
           *                   oriented strips is returned */
          const StripDesign& design(bool phiView = false) const;
          /// Returns the hash of the strip layer
          const IdentifierHash hash() const;
          /** @brief Returns the position of the strip centre expressed in the frame of the local readout plane
           *  @param stripNum: Number of the strip to fetch [firtStrip - nStrips] cf. StripDesign,
           *  @param phiView: Switch whether the strips in the phi plane should be returned. 
           *                  Only active if the class is instantiated with two strip designs */
          Amg::Vector3D localStripPosition(unsigned int stripum,
                                           bool phiView = false) const;
          /** @brief Returns the position of the strip edge at (positive y in the strip design description)
           *         in the local coordinates of the local readout planes
           *  @param stripNum: Number of the strip to fetch [firtStrip - nStrips] cf. StripDesign 
           *  @param phiView: Switch whether the strips in the phi plane should be returned. 
           *                  Only active if the class is instantiated with two strip designs */
          Amg::Vector3D localStripLeftEdge(unsigned int stripNum,
                                           bool phiView = false) const;
          /** @brief Returns the position of the  right strip edge at (negative y in the strip design description)
           *         in the local coordinates of the local readout planes
           *  @param stripNum: Number of the strip to fetch [firtStrip - nStrips] cf. StripDesign 
           *  @param phiView: Switch whether the strips in the phi plane should be returned. 
           *                  Only active if the class is instantiated with two strip designs */
          Amg::Vector3D localStripRightEdge(unsigned int stripNum,
                                            bool phiView = false) const;
          /** @brief Comparison operator to recycle equivalent StripLayers for multiple readout elements */
          bool operator<(const StripLayer& other) const;
          /** @brief Returns whether the strip layer also describes strips in the phi direction */
          bool hasPhiDesign() const;
          using CheckVector2D = StripDesign::CheckVector2D;
          /** @brief Transforms the 2D vector from the strip design into a 3D vector
            *         If phi view is switched on, the vector is additionally rotated by 90 degrees
            * @param vec: Vector to be turned into a 3D vector
            * @param phiView: Switched whether the strips should be rotated */
          Amg::Vector3D to3D(CheckVector2D&& vec, const bool phiView) const;
          /** @brief Transforms a 3D vector from the strip design into a 2D vector.
           *         If phi view is switched on, the vector is rotated by -90 degrees
           * @param vec: Vector to be turned into a 3D vector
           * @param phiView: Switched whether the strips should be rotated */
          Amg::Vector2D to2D(const Amg::Vector3D& vec, const bool phiView) const;
          /** @brief Flips the phi rotation from 90 -> -90 degrees */
          void flipPhiRotation();
        private:
           /** @brief Pointer to the GeoModelTransform  */
           TransformPtr m_transform{};
           /** @brief Pointer to the eta strip design */
           StripDesignPtr m_etaDesign{};
           /** @brief Pointer to the phi strip design */
           StripDesignPtr m_phiDesign{};
           /** @brief Hash of the strip layer */
           IdentifierHash m_hash{};
           double m_phiRot{90.*Gaudi::Units::deg};
    };
    using StripLayerPtr = GeoModel::TransientConstSharedPtr<StripLayer>;
    /// Helper struct to share strip layer instances across the readout elements
    struct StripLayerSorter {
            bool operator()( const StripLayerPtr&a, const StripLayerPtr& b) const{
                return (*a) < (*b);
            }
    };
    std::ostream& operator<<(std::ostream& ostr, const StripLayer& lay);
}

#include <MuonReadoutGeometryR4/StripLayer.icc>
#endif