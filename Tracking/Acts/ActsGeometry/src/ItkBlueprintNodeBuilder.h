/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ITKBLUEPRINTNODEBUILDER_H
#define ACTSGEOMETRY_ITKBLUEPRINTNODEBUILDER_H

#include "ActsGeometry/ActsElementVector.h"
#include "ActsGeometryInterfaces/IBlueprintNodeBuilder.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"

#include "Acts/Navigation/SurfaceArrayNavigationPolicy.hpp"

#include <vector>

namespace ActsTrk {

/** Helper class to build the ItkBlueprint node
 *  It adds the system as a node to the Blueprint.
 */
class ItkBlueprintNodeBuilder
    : public extends<AthAlgTool, IBlueprintNodeBuilder> {

 public:
  StatusCode initialize() override;
  using base_class::base_class;

  /** @brief Build the Itk Blueprint Node
   *  @param gctx Geometry context
   *  @param child Optional beam pipe node to nest inside the ITk AxisR
   * container.*/
  std::shared_ptr<Acts::BlueprintNode> buildBlueprintNode(
      const Acts::GeometryContext& gctx,
      std::shared_ptr<Acts::BlueprintNode>&& child) override;

 private:
  const InDetDD::SiDetectorManager* m_itkPixelMgr{nullptr};
  const InDetDD::SiDetectorManager* m_itkStripMgr{nullptr};
  std::shared_ptr<ActsElementVector> m_elementStore{nullptr};

  Gaudi::Property<bool> m_doEndcapLayerMerging{this, "doEndcapLayerMerging",
                                                true};
  Gaudi::Property<bool> m_buildPixel{this, "buildPixel", true};
  Gaudi::Property<bool> m_buildStrip{this, "buildStrip", true};

  /// Most bins the surface array lookup serves on each side of the crossing,
  /// as {phi, z} for the barrel and {r, phi} for the endcap. The lookup sizes
  /// the window from the crossing angle and the layer thickness, so these
  /// values only cap it. The defaults cover the steepest crossing for
  /// |z0| < 200 mm at a bin factor of 5.
  Gaudi::Property<std::vector<unsigned int>> m_itkPixelInnerBarrelNeighborWindow{
      this, "ITkPixelInnerBarrelNeighborWindow", {1, 8},
      "Neighbor window bound {phi, z} of the ITk inner pixel barrel layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkPixelInnerEndcapNeighborWindow{
      this, "ITkPixelInnerEndcapNeighborWindow", {2, 1},
      "Neighbor window bound {r, phi} of the ITk inner pixel endcap layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkPixelOuterBarrelNeighborWindow{
      this, "ITkPixelOuterBarrelNeighborWindow", {1, 3},
      "Neighbor window bound {phi, z} of the ITk outer pixel barrel layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkPixelOuterEndcapNeighborWindow{
      this, "ITkPixelOuterEndcapNeighborWindow", {4, 1},
      "Neighbor window bound {r, phi} of the ITk outer pixel endcap and inclined layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkStripBarrelNeighborWindow{
      this, "ITkStripBarrelNeighborWindow", {1, 4},
      "Neighbor window bound {phi, z} of the ITk strip barrel layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkStripEndcapNeighborWindow{
      this, "ITkStripEndcapNeighborWindow", {2, 1},
      "Neighbor window bound {r, phi} of the ITk strip endcap layers"};

  /// Extra cells per direction and axis that each surface fills around its footprint
  Gaudi::Property<unsigned int> m_surfaceArrayOverfill{this, "SurfaceArrayOverfill", 0,
      "Extra cells per direction and axis that each surface fills in the surface arrays"};

  /// Configuration of the surface array navigation policy of one layer
  /// @param layerType The layer type of the surface array
  /// @param neighborWindow The neighbor window bound property, checked in initialize()
  Acts::SurfaceArrayNavigationPolicy::Config surfaceArrayConfig(
      Acts::SurfaceArrayNavigationPolicy::LayerType layerType,
      const Gaudi::Property<std::vector<unsigned int>>& neighborWindow) const;

  void buildItkStripBlueprintNode(const Acts::GeometryContext& gctx,
                                  Acts::BlueprintNode& node);
  void buildItkPixelBlueprintNode(const Acts::GeometryContext& gctx,
                                  Acts::BlueprintNode& node);
};

}  // namespace ActsTrk

#endif
