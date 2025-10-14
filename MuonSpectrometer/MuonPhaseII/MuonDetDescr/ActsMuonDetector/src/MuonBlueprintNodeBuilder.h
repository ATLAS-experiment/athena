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

  using staticNodePtr = std::shared_ptr<Acts::Experimental::StaticBlueprintNode>;
  using surfacePtr = std::shared_ptr<Acts::Surface>;
  using MuonChamberSet = MuonGMR4::MuonDetectorManager::MuonChamberSet;
  using MuonSectorSet = MuonGMR4::MuonDetectorManager::MuonSectorSet;
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

  Gaudi::Property<bool> m_useSectors{this, "UseSectors", false}; // Flag to control if we want to build the muon node from sectors or chambers

  /** @brief Blend the sector's/chamber's material as plane surface
    * @param element The element for which to blend the material for
    *  This function returns a plane surface with the element's (chamber or sector) material
    *  assigned to be placed at the center of the element. */
  template<typename T>
  std::shared_ptr<Acts::Surface> blendMaterial(const T& element) const;

  /** @brief Get the chamber's sensitive elements
    * @param gctx The geometry context
    * @param element The element for which to get the sensitive elements (chamber or sector)
    * @param chId The geometry identifier of the chamber
    * @param boundsFactory The factory for volume bounds
    *  This function constructs and returns the sensitive elements (volumes and surfaces) of the sector. */
   template<typename T>
   std::pair<std::vector<staticNodePtr>, std::vector<surfacePtr>> getSensitiveElements(const ActsGeometryContext& gctx,
                                                                                  const T& element,
                                                                                  const Acts::GeometryIdentifier& chId,
                                                                                  Acts::VolumeBoundFactory& boundsFactory) const;

  /** @brief Check if the chamber is in this node
    * @param element The element to check (chamber or sector)
    * @param stationNames The names of the stations to check against
    * @param side The side of the endcap (A, C or Both)
    *  This function checks if the chamber is part of the configured chambers in this node.
    *  It is used to filter out chambers that are not part of this muon node. */
  template<typename T>
  bool isElementInTheStation(const T& element, const std::vector<StIdx>& stationNames, const EndcapSide& side) const;

  /** @brief Build subnodes for the muon system node
   *  @param gctx The geometry context
   *  @param elements The name of the stations to include
   *  @param side The side (A, C or Both)
   *  @param id The geometry identifier of this node
   * @param boundsFactory The factory for volume bounds
   */
  template<typename MuonElementsSet>
  std::shared_ptr<Acts::Experimental::StaticBlueprintNode> buildMuonNode(const Acts::GeometryContext& gctx,
    const MuonElementsSet& elements,
    const std::string& name,
    const Acts::GeometryIdentifier& id,
    Acts::VolumeBoundFactory& boundsFactory) const;
};

} //namespace ActsTrk

#endif
