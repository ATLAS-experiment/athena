/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMUONDETECTOR_MUONBLUEPRINTNODEBUILDER_H
#define ACTSMUONDETECTOR_MUONBLUEPRINTNODEBUILDER_H

#include <AthenaBaseComps/AthAlgTool.h>

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonStationIndex/MuonStationIndex.h>
#include <ActsGeometryInterfaces/IBlueprintNodeBuilder.h>

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
    class MaterialDesignatorBlueprintNode;
    
}

}  // namespace Acts


namespace ActsTrk {


/** Helper class to build a Blueprint node of the muon system. 
 *  It builds the whole muon system for PhaseII adding it to the Blueprint as a node.
 */
class MuonBlueprintNodeBuilder : public extends<AthAlgTool, IBlueprintNodeBuilder> {
public:
  /** @brief Abrivation of the blueprint node ptr base class */
  using blueprintNodePtr = std::shared_ptr<Acts::Experimental::BlueprintNode>;
  /** @brief Abrivation of the blue print node pointer */
  using staticNodePtr = std::shared_ptr<Acts::Experimental::StaticBlueprintNode>;
  /** @brief Abrivation of the material node pointer */
  using materialNodePtr = std::shared_ptr<Acts::Experimental::MaterialDesignatorBlueprintNode>;
  /** @brief Abrivation of the surface pointer*/
  using surfacePtr = std::shared_ptr<Acts::Surface>;
  /** @brief Abrivate the vector pair of blue print nodes and associated active surfaces */
  using BluePrintSurfPairs_t = std::pair<std::vector<blueprintNodePtr>, std::vector<surfacePtr>>;
  /** @brief Abrivation of the container holding all chambers */
  using MuonChamberSet = MuonGMR4::MuonDetectorManager::MuonChamberSet;
  /** @brief Abrivation of the container holding all ms sectors */
  using MuonSectorSet = MuonGMR4::MuonDetectorManager::MuonSectorSet;
  /** @brief Abrivation of the station index */
  using StIdx = Muon::MuonStationIndex::StIndex;
  /** @brief Abrivatin for the detector region index */
  using DetIdx = Muon::MuonStationIndex::DetectorRegionIndex;
  /** @brief Abrivation for the layer index */
  using LayIdx = Muon::MuonStationIndex::LayerIndex;
  /** @brief Abrivation for the chamber index */
  using ChIdx = Muon::MuonStationIndex::ChIndex;
  /** @brief Abrivation for the stations indices */
  using DetLayIdx_t = std::pair<DetIdx, LayIdx>;
  /** @brief Hide the flexibility to build the tracking geometry from sectors or chambers
   *         behind a variant */
  using EnvelopeSet_t = std::variant<MuonChamberSet, MuonSectorSet>;
  /*** @brief Subdivide the envelopes according to their station index  */
  using EnvelopesPerStIdx_t = std::unordered_map<StIdx, EnvelopeSet_t>;

  enum class EndcapSide: std::uint8_t {
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
  /** @brief the Detector manager */
  const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
  /** @brief Flag to control if we want to build the muon node from sectors or chambers  */
  Gaudi::Property<bool> m_useSectors{this, "UseSectors", false}; 
  /** @brief Flag to control if the volumes should be alignable or not */
  Gaudi::Property<bool> m_alignableVolumes{this, "AlignableVolumes", true};
  /** @brief Flag to assign active material on the chambers */
  Gaudi::Property<bool> m_assignActiveMaterial{this, "AssignActiveMaterial", false};
  /** @brief Flag to construct the passive material surfaces */
  Gaudi::Property<bool> m_buildPassiveVolumes{this, "BuildPassiveVolumes", true};
  /** @brief Number of bins in phi direction on the BI cylinder surface */
  Gaudi::Property<std::size_t> m_nPhiBinsBI{this, "nPhiBinsBI", 16};
  /** @brief Number of bins in Z direction on the BI cylinder surface */
  Gaudi::Property<std::size_t> m_nZBinsBI{this, "nZBinsBI", 12};
  /** @brief Number of bins in phi direction on the BM cylinder surface */
  Gaudi::Property<std::size_t> m_nPhiBinsBM{this, "nPhiBinsBM", 16};
  /** @brief Number of bins in Z direction on the BM cylinder surface */
  Gaudi::Property<std::size_t> m_nZBinsBM{this, "nZBinsBM", 12};
  /** @brief Number of bins in phi direction on the BM cylinder surface */
  Gaudi::Property<std::size_t> m_nPhiBinsBO{this, "nPhiBinsBO", 16};
  /** @brief Number of bins in Z direction on the BM cylinder surface */
  Gaudi::Property<std::size_t> m_nZBinsBO{this, "nZBinsBO", 12};
  /** @brief Number of bins in R direction on the disc before the NSW */
  Gaudi::Property<std::size_t> m_nRBinsEI1{this, "nRBinsEIbNSW", 4};
  /** @brief Number of bins in phi direction on the disc before the NSW */
  Gaudi::Property<std::size_t> m_nPhiBinsEI1{this, "nPhiBinsEIbNSW", 16};
  /** @brief Number of bins in R direction on the disc after the NSW */
  Gaudi::Property<std::size_t> m_nRBinsEI2{this, "nRBinsEIaNSW", 4};
  /** @brief Number of bins in phi direction on the disc after the NSW */
  Gaudi::Property<std::size_t> m_nPhiBinsEI2{this, "nPhiBinsEIaNSW", 16};

  /** @brief Number of bins in R direction on the disc before the middle big wheel */
  Gaudi::Property<std::size_t> m_nRBinsEM1{this, "nRBinsEMbBW", 16};
  /** @brief Number of bins in phi direction on the disc before the middle big wheel  */
  Gaudi::Property<std::size_t> m_nPhiBinsEM1{this, "nPhiBinsEMbBW", 16};
  /** @brief Number of bins in R direction on the disc after the middle big wheel  */
  Gaudi::Property<std::size_t> m_nRBinsEM2{this, "nRBinsEMaBW", 5};
  /** @brief Number of bins in phi direction on the disc after the NSW */
  Gaudi::Property<std::size_t> m_nPhiBinsEM2{this, "nPhiBinsEMaBW", 16};

  /** @brief Prepare a binned material which is associated to the surface
   *  @param type: The surface type on which the material is mapped (Plane, Disc, Cylinder)
   *  @param nBins1: Number of bins in the local0 direction
   *  @param nBins2: Number of bins in the complementary direction */
  std::shared_ptr<Acts::ISurfaceMaterial> preparePassiveMaterial(const Acts::SurfaceBounds& bounds,
                                                          const std::size_t nBins1,
                                                          const std::size_t nBins2) const;

  std::pair<std::size_t, std::size_t> getMaterialBins(const Muon::MuonStationIndex::ChIndex chIdx) const;

  
  /** @brief Get the chamber's sensitive elements
    * @param gctx Geometry context
    * @param element The element for which to get the sensitive elements (chamber or sector)
    * @param chId The geometry identifier of the chamber
    * @param boundsFactory The factory for volume bounds
    *  This function constructs and returns the sensitive elements (volumes and surfaces) of the sector. */
    template<typename T>
    BluePrintSurfPairs_t getSensitiveElements(const ActsTrk::GeometryContext& gctx,
                                              const T& element,
                                              const Acts::GeometryIdentifier& chId,
                                              Acts::VolumeBoundFactory& boundsFactory) const
      requires(std::is_same_v<T, MuonGMR4::Chamber> ||
               std::is_same_v<T, MuonGMR4::SpectrometerSector>);

  /** @brief Construct and return the surfaces for the passive material description (e.g cylinders for barrel/ discs for endcaps)
   *  @param gctx The geometry context
   *  @param elementsPerStation The elements (chambers or sectors) grouped per station to which we want to assign passive material
   * This function uses the elements of the station to construct the surfaces and define their bounds */
  template <typename ElementSet_t>
   std::vector<surfacePtr> getPassiveMaterialSurfaces(const Acts::GeometryContext& gctx,
                                                     const std::unordered_map<unsigned int, ElementSet_t>& elementsPerStation) const;
    
  
  /** @brief Get the active material for a given element representing the chamber/sector
   *  @param element The element for which to get the active material
   *  @return The active surface material */
    template<typename T>    
    std::shared_ptr<const Acts::ISurfaceMaterial>
    getActiveMaterial(const T& element) const
      requires (std::is_same_v<T, MuonGMR4::Chamber> ||
              std::is_same_v<T, MuonGMR4::SpectrometerSector>);

  /** @brief Check if the chamber is in this node
    * @param element The element to check (chamber or sector)
    * @param stationNames The names of the stations to check against
    * @param side The side of the endcap (A, C or Both)
    *  This function checks if the chamber is part of the configured chambers in this node.
    *  It is used to filter out chambers that are not part of this muon node. */
  template<typename T>
  bool isElementInTheStation(const T& element, 
                             const std::vector<StIdx>& stationNames, 
                             const EndcapSide side) const
       requires(std::is_same_v<T, MuonGMR4::Chamber> ||
                std::is_same_v<T, MuonGMR4::SpectrometerSector>);

  /** @brief Build subnodes for the muon system node
   *  @param gctx The geometry context
   *  @param elements The name of the stations to include
   *  @param side The side (A, C or Both)
   *  @param id The geometry identifier of this node
   *  @param boundsFactory The factory for volume bounds
   *  @param passiveStationIds The ids with the chamber indices we want to put passive material surfaces on
   */
  staticNodePtr buildMuonNode(const Acts::GeometryContext& gctx,
                              const EnvelopeSet_t& elements,
                              const std::string& name,
                              const Acts::GeometryIdentifier& id,
                              Acts::VolumeBoundFactory& boundsFactory,
                              const std::vector<ChIdx>& passiveStationIds = {}) const;

  /** @brief Build a static or a material node for a chamber that corresponds to a single blueprint node (e.g for a single MDT multilayer)
   *  @param chamberNode The blueprint node out of which the variant node will be built
   *  @return A variant holding either a static node or material node depending on wether the assignment of active material is enabled
   */
  std::variant<staticNodePtr, materialNodePtr> buildChamberNode(const blueprintNodePtr& chamberNode) const;

  /** @brief Build a static or a material node for a chamber that corresponds to a single blueprint node (e.g for a single MDT multilayer)
   *  @param innerStructure The inner structure of the chamber that corresponds to the children nodes
   *  @param element The element representing the chamber/sector for which the node is built
   *  @param vol The tracking volume associated to the chamber/sector
   *  @return A variant holding either a static node or material node depending on wether the assignment of active material is enabled
   */
  template<typename T>
  std::variant<staticNodePtr, materialNodePtr> buildChamberNode(const T& element, 
                                                                std::unique_ptr<Acts::TrackingVolume>& vol,
                                                                const std::vector<blueprintNodePtr>& innerStructure) const;

  /** @brief Helper function determining whether a readout element is BIS78 */
  bool isBIS78(const MuonGMR4::MuonReadoutElement* element) const;
};

} //namespace ActsTrk

#endif
