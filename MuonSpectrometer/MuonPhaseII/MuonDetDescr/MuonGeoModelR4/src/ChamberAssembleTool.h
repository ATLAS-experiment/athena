/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE
#ifndef MUONGEOMODELR4_MUONCHAMBERASSMBLETOOL_H
#define MUONGEOMODELR4_MUONCHAMBERASSMBLETOOL_H

#include <AthenaBaseComps/AthAlgTool.h>

#include <MuonReadoutGeometryR4/SpectrometerSector.h>

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <MuonGeoModelR4/IMuonReaoutGeomTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <ActsGeoUtils/Defs.h>

#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/Geometry/VolumeBounds.hpp"
#include "Acts/Utilities/PointerTraits.hpp"
#include "Acts/Utilities/BoundFactory.hpp"
#include <tuple>

namespace MuonGMR4 {

class MuonReadoutElement;
/** @brief Assembly tool to produce the Acts envolpe volumes around the muon stations & also to construct the
 *                  spectrometer envelope volumes */
class ChamberAssembleTool : public extends<AthAlgTool, IMuonReadoutGeomTool> {
   public:
      /** @brief Standard constructor of the tool */
      using base_class::base_class;

      virtual StatusCode buildReadOutElements(MuonDetectorManager &mgr) override final;

      /// @brief Abrivation of the volume bounds
      using VolBounds_t = Acts::VolumeBounds;
      /// @brief Abrivation of the Volume bound ptr
      using VolBoundPtr_t = std::shared_ptr<VolBounds_t>;
      /// @brief Abrivation of the surface bounds
      using SurfBoundPtr_t = std::shared_ptr<const Acts::PlanarBounds>;
      /** @brief Abrivation of the volume transform together with a set of volume & surface bounds */
      using TrfWithBounds = std::tuple<Amg::Transform3D, VolBoundPtr_t, SurfBoundPtr_t>;
   private:
      /** @brief builds the bounding box trapezoidal volume bounds from the set of readout elements
       *         Returns a pair of the volume bounds & the transformation to center the volume
       *  @param gctx: Geometry context holding the alignment & global transformations
       *  @param constituents: List of readout elements around which the bounding box shall be built
       *  @param globToLoc: Transformation to go from the global -> local chamber's frame 
       *  @param volBoundSet: Factory to create the volume bounds & share them across equivalent volumes
       *  @param surfBoundSet: Factory to create the surface bounds representing the material surfaces & share them
       *                       across mutliple surfaces
       *  @param margin: Extra margin by which the returned volume bounds are enlarged */
      template <typename ReObjType>
      TrfWithBounds boundingBox(const ActsTrk::GeometryContext& gctx,
                                const std::vector<ReObjType>& constituents,
                                const Amg::Transform3D& globToLoc,
                                Acts::VolumeBoundFactory& volBoundSet,
                                Acts::SurfaceBoundFactory& surfBoundSet,
                                const double margin) const
            requires (Acts::PointerConcept<ReObjType>);

      /** @brief Builds the trapezoidal bounding box enclosing a single readout element
        * @param reEle: Pointer to the readout element to fetch the bounds from
        * @param boundSet: Cache of create bounds to share the same bounds across multiple volumes */
      static VolBoundPtr_t boundingBox(const MuonReadoutElement* reEle,
                                        Acts::VolumeBoundFactory& boundSet);
      using ChamberPtr = SpectrometerSector::ChamberPtr;

      static VolBoundPtr_t boundingBox(const ChamberPtr& chamber,
                                       Acts::VolumeBoundFactory& boundSet);

      /** @brief Returns the 4 corners of the trapezoid in the x-y plane
        * @param localToGlob: Transform from the trapezoid restframe -> chambers frame
        * @param bounds: Reference to the trapezoidal bounds defining the volume */
      static std::array<Amg::Vector3D, 4> cornerPointsPlane(const Amg::Transform3D& localToGlob, 
                                                            const VolBounds_t& bounds);

      /** @brief Returns the 8 corners marking the trapezoid 
        * @param localToGlob: Transform from the trapezoid restframe -> chambers frame
        * @param bounds: Reference to the trapezoidal bounds defining the volume */
      static std::array<Amg::Vector3D, 8> cornerPoints(const Amg::Transform3D& localToGlob, 
                                                       const VolBounds_t& bounds);

      /** @brief Returns the translation transform centering the 8 corner points of the trapezoid.
        *        The centre is defined as the centre point of the surrounding box
        * @param cornerPoints: Array to all 8 corner points of the trapezoid */
      static Amg::Transform3D centerTrapezoid(const std::array<Amg::Vector3D, 8>& cornerPoints);
      /** @brief Returns the signed distances of an external point to the trapezoidal edge. 
       *         Distances > 0 indicate that the point is inside the boundaries and outside otherwise
       *  @param linePos: Arbitrary point on the trapezoidal edge
       *  @param lineDir: Direction of the trapezoidal edge
       *  @param testMe: External point to measure the distance
       *  @param leftEdge: Switch indicating whether the edge is on the left & right side */
      static double trapezoidEdgeDist(const Amg::Vector3D& linePos,
                                      const Amg::Vector3D& lineDir,
                                      const Amg::Vector3D& testMe,
                                      bool leftEdge);
      /** @brief Enlarge the parsed volume bounds by an extra margin attached to all 3 dimensions
       *  @param enlargeMe: Bounds which are intended to be enlarged
       *  @param margin: Amount by which the total length of the bounds should grow
       *  @param volBoundsSet: Bound factory to assign equivalent bounds to multiple volumes*/
      static VolBoundPtr_t enlargeBounds(const VolBounds_t& enlargeMe,
                                         const double margin,
                                         Acts::VolumeBoundFactory& volBoundSet);
      /** @brief Construct surface bounds which measure equal sizes in halfXlow/halfXhigh & halfY as the 
       *         parsed volume bounds
       *  @param volBounds: Bounds which are mapped to surface bounds
       *  @param surfBoundSet: Bound factory to assign equivalent bounds to multiple surfaces */
      static SurfBoundPtr_t surfaceBounds(const VolBounds_t& volBounds,
                                          Acts::SurfaceBoundFactory& surfBoundSet);
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
      /** @brief Toggling whether the layout is a R3 or R4 layout. If true, the BIS eta: -7
       *         chambers are split into the individual readout elements. */
      Gaudi::Property<bool> m_isRun4{this, "run4Layout", false};
};

}

#endif
#endif

