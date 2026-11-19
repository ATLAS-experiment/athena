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
  /// |z0| < 200 mm at the bin factor of the layer (5, 2.5 in the strip barrel).
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
  /// The two innermost strip barrel layers have short strips (rows of 24 mm
  /// along z) and are crossed at the steepest angles (|eta| up to 1.9): their
  /// window reaches 6 bins of 9.8 mm in z
  Gaudi::Property<std::vector<unsigned int>> m_itkStripInnerBarrelNeighborWindow{
      this, "ITkStripInnerBarrelNeighborWindow", {1, 6},
      "Neighbor window bound {phi, z} of the two innermost ITk strip barrel layers (short strips)"};
  /// Surface array bins per module along each axis in the strip barrel layers:
  /// half the factor of the other layers, so that the window above stays
  /// small enough for a cheap neighbour cache
  Gaudi::Property<double> m_itkStripBarrelBinFactor{
      this, "ITkStripBarrelBinFactor", 2.5,
      "Surface array bins per module along each axis of the ITk strip barrel layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkStripEndcapNeighborWindow{
      this, "ITkStripEndcapNeighborWindow", {2, 1},
      "Neighbor window bound {r, phi} of the ITk strip endcap layers"};

  /// Extra cells per direction and axis that each surface fills around its footprint
  Gaudi::Property<unsigned int> m_surfaceArrayOverfill{this, "SurfaceArrayOverfill", 0,
      "Extra cells per direction and axis that each surface fills in the surface arrays"};

  /// Material binning: equidistant bins of the material designated on each
  /// surface, as {r*phi, z} on cylinder faces and {r, phi} on disc faces. The
  /// bins span the face, whose extent is set when the volumes are built. The
  /// material map is binned the same way, so a map made with other values
  /// does not fit the geometry.
  Gaudi::Property<std::vector<unsigned int>> m_itkPixelCylinderMaterialBins{
      this, "ITkPixelCylinderMaterialBins", {50, 100},
      "Material bins {r*phi, z} of the ITk pixel cylinders: barrel layers, outer "
      "cylinders of the inner and outer pixel, cylinders between the outer "
      "endcap ring groups"};
  Gaudi::Property<std::vector<unsigned int>> m_itkDiscMaterialBins{
      this, "ITkDiscMaterialBins", {50, 50},
      "Material bins {r, phi} of the two discs closing the whole ITk in z "
      "(built with the strip)"};
  Gaudi::Property<std::vector<unsigned int>> m_itkCylinderMaterialBins{
      this, "ITkCylinderMaterialBins", {50, 100},
      "Material bins {r*phi, z} of the cylinder closing the whole ITk in r "
      "(outer cylinder of the strip)"};
  Gaudi::Property<std::vector<unsigned int>> m_itkPixelDiscMaterialBins{
      this, "ITkPixelDiscMaterialBins", {50, 50},
      "Material bins {r, phi} of the ITk pixel discs: endcap and ring layers "
      "and discs closing the barrels in z"};
  Gaudi::Property<std::vector<unsigned int>> m_itkStripCylinderMaterialBins{
      this, "ITkStripCylinderMaterialBins", {50, 100},
      "Material bins {r*phi, z} of the ITk strip barrel layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkStripDiscMaterialBins{
      this, "ITkStripDiscMaterialBins", {50, 50},
      "Material bins {r, phi} of the ITk strip discs: endcap layers and discs "
      "closing the barrel in z"};

  /// Surface array bins per module along each axis, outside the strip barrel
  static constexpr double s_binFactor = 5.0;

  /// Configuration of the surface array navigation policy of one layer
  /// @param layerType The layer type of the surface array
  /// @param neighborWindow The neighbor window bound property, checked in initialize()
  /// @param binFactor Surface array bins per module along each axis
  Acts::SurfaceArrayNavigationPolicy::Config surfaceArrayConfig(
      Acts::SurfaceArrayNavigationPolicy::LayerType layerType,
      const Gaudi::Property<std::vector<unsigned int>>& neighborWindow,
      double binFactor = s_binFactor) const;

  void buildItkStripBlueprintNode(const Acts::GeometryContext& gctx,
                                  Acts::BlueprintNode& node);
  void buildItkPixelBlueprintNode(const Acts::GeometryContext& gctx,
                                  Acts::BlueprintNode& node);
};

}  // namespace ActsTrk

#endif
