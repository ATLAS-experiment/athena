/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE
#ifndef MUONGEOMODELR4_MUONCHAMBERASSMBLETOOL_H
#define MUONGEOMODELR4_MUONCHAMBERASSMBLETOOL_H

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>

#include <GeoModelInterfaces/IGeoDbTagSvc.h>
#include <MuonGeoModelR4/IMuonReaoutGeomTool.h>
#include <MuonGeoModelR4/IMuonGeoUtilityTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <ActsGeoUtils/Defs.h>


namespace Acts{
    class TrapezoidVolumeBounds;
}

namespace MuonGMR4 {

class MuonReadoutElement;

class ChamberAssembleTool : public extends<AthAlgTool, IMuonReadoutGeomTool> {
   public:
      /** @brief Standard constructor of the tool */
      using base_class::base_class;

      virtual StatusCode buildReadOutElements(MuonDetectorManager &mgr) override final;

   private:
      using BoundType = Acts::TrapezoidVolumeBounds;

      using BoundTrfPair = std::pair<std::shared_ptr<BoundType>,
                                     Amg::Transform3D>;
      /** @brief builds the bounding box trapezoidal volume bounds from the set of readout elements
       *         Returns a pair of the volume bounds & the transformation to center the volume
       *  @param gctx: Geometry context holding the alignment & global transformations
       *  @param readoutEles: List of readout elements around which the bounding box shall be built
       *  @param globToLoc: Transformation to go from the global -> local chamber's frame 
       *  @param boundSet: Cache of create bounds to share the same bounds across multiple volumes */
      BoundTrfPair boundingBox(const ActsGeometryContext& gctx,
                               const std::vector<const MuonReadoutElement*>& readoutEles,
                               const Amg::Transform3D& globToLoc,
                               ActsTrk::SurfaceBoundSet<BoundType>& boundSet,
                               const double margin = 1.*Gaudi::Units::cm) const;

      /** @brief Builds the trapezoidal bounding box enclosing a single readout element
        * @param reEle: Pointer to the readout element to fetch the bounds from
        * @param boundSet: Cache of create bounds to share the same bounds across multiple volumes */
      static std::shared_ptr<BoundType> boundingBox(const MuonReadoutElement* reEle,
                                                    ActsTrk::SurfaceBoundSet<BoundType>& boundSet);

      /** @brief Returns the 4 corners of the trapezoid in the x-y plane
        * @param localToGlob: Transform from the trapezoid restframe -> chambers frame
        * @param bounds: Reference to the trapezoidal bounds defining the volume */
      static std::array<Amg::Vector3D, 4> cornerPointsPlane(const Amg::Transform3D& localToGlob, 
                                                            const BoundType& bounds);

      /** @brief Returns the 8 corners marking the trapezoid 
        * @param localToGlob: Transform from the trapezoid restframe -> chambers frame
        * @param bounds: Reference to the trapezoidal bounds defining the volume */
      static std::array<Amg::Vector3D, 8> cornerPoints(const Amg::Transform3D& localToGlob, 
                                                       const BoundType& bounds);

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
   
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
      PublicToolHandle<IMuonGeoUtilityTool> m_geoUtilTool{this,"GeoUtilTool", "" };
    
};

}

#endif
#endif

