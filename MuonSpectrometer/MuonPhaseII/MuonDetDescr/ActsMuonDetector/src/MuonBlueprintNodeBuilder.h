/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMUONDETECTOR_MUONBLUEPRINTNODEBUILDER_H
#define ACTSMUONDETECTOR_MUONBLUEPRINTNODEBUILDER_H



#include <GaudiKernel/MsgStream.h>
#include <AthenaBaseComps/AthMessaging.h>

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include "MuonStationIndex/MuonStationIndex.h"
#include "ActsGeometryInterfaces/IBlueprintNodeBuilder.h"

#include "Acts/Utilities/BoundFactory.hpp"
#include "Acts/Surfaces/PlanarBounds.hpp"
#include "Acts/Surfaces/Surface.hpp"




namespace MuonGMR4 {
  class Chamber;
}

namespace Acts {
  class GeometryContext;
  namespace Experimental {
    class StaticBlueprintNode;
    
}

}  // namespace Acts


namespace ActsTrk {

  using volumePtr= std::unique_ptr<Acts::TrackingVolume>;
  using surfacePtr = std::shared_ptr<Acts::Surface>;
  using MuonChamberSet = MuonGMR4::MuonDetectorManager::MuonChamberSet;
  using StIdx = Muon::MuonStationIndex::StIndex;

/** Helper class to build a Blueprint node of the muon system. 
 *  It builds the whole muon system for PhaseII adding it to the Blueprint as a node.
 */
class MuonBlueprintNodeBuilder : public extends<AthAlgTool, IBlueprintNodeBuilder> {

public:


  enum class EndcapSide {
      A,
      C,
      Both
  };

  StatusCode initialize() override;
  using base_class::base_class;
  
  /** @brief Build the Muon Blueprint Node
    *  @param gctx Geometry context
    *  @param childNode The blueprint node as child of this node (for Muon System it should be Calo or Itk).*/
  std::shared_ptr<Acts::Experimental::BlueprintNode> buildBlueprintNode(const Acts::GeometryContext& gctx,
                                              std::shared_ptr<Acts::Experimental::BlueprintNode>&& childNode) override;
                                

private:

  const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

  Gaudi::Property<bool> m_dumpVolumes{this, "dumpVolumes", false}; // Flag to control if we want to visualize each chamber volume individually

  /** @brief Blend the chamber's material as plane surface
    *  @param chamber The chamber to blend the material for
    *  @param chamberTransform The transformation of the chamber
    *  @param materialBoundsFactory The factory for surface bounds
    *  This function returns a plane surface with the chamber's material
    *  assigned to be placed at the center of the chamber. */
  std::shared_ptr<Acts::Surface> blendChamberMaterial(const MuonGMR4::Chamber& chamber) const;
                                                              
  /** @brief Get the chamber's sensitive elements
    *  @param gctx The geometry context
    *  @param chamber The chamber to get the elements from
    *  @param chId The geometry identifier of the chamber
    *  @param boundsFactory The factory for volume bounds
    *  This function constructs and returns the sensitive elements (volumes and surfaces) of the chamber. */
  std::pair<std::vector<volumePtr>, std::vector<surfacePtr>> getSensitiveElements( const ActsGeometryContext& gctx,
                                                                                  const MuonGMR4::Chamber& chamber,
                                                                                  const Acts::GeometryIdentifier& chId,
                                                                                  Acts::VolumeBoundFactory& boundsFactory) const;
  
  /** @brief Check if the chamber is in this node 
    * @param chamber The chamber to check   
    * @param stationNames The names of the stations to check against
    * @param side The side of the endcap (A, C or Both)
    *  This function checks if the chamber is part of the configured chambers in this node.
    *  It is used to filter out chambers that are not part of this muon node. */
  bool isChamberInTheStation(const MuonGMR4::Chamber& chamber, const std::vector<StIdx>& stationNames, const EndcapSide& side) const;

  /** @brief Build subnodes for the muon system node
   *  @param gctx The geometry context
   *  @param stations The name of the stations to include
   *  @param side The side (A, C or Both)
   *  @param id The geometry identifier of this node
   * @param boundsFactory The factory for volume bounds
   */
  std::shared_ptr<Acts::Experimental::StaticBlueprintNode> buildMuonNode(const Acts::GeometryContext& gctx,
    const std::vector<StIdx>& stations,
    const EndcapSide& side,
    const Acts::GeometryIdentifier& id,
    Acts::VolumeBoundFactory& boundsFactory) const;
};

} //namespace ActsTrk

#endif
