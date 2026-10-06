/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELR4_MUONCHAMBERASSMBLETOOL_H
#define MUONGEOMODELR4_MUONCHAMBERASSMBLETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "MuonReadoutGeometryR4/SpectrometerSector.h"

#include "MuonGeoModelR4/IMuonReaoutGeomTool.h"
#include "MuonGeoModelR4/IMuonGeoUtilityTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

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
      using TrfWithBounds = std::tuple<Acts::Transform3, VolBoundPtr_t, SurfBoundPtr_t>;
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
                                const Acts::Transform3& globToLoc,
                                Acts::VolumeBoundFactory& volBoundSet,
                                Acts::SurfaceBoundFactory& surfBoundSet,
                                const double margin) const
            requires (Acts::PointerConcept<ReObjType>);

      using ChamberPtr = SpectrometerSector::ChamberPtr;

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