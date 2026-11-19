/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_HGTDBLUEPRINTNODEBUILDER_H
#define ACTSGEOMETRY_HGTDBLUEPRINTNODEBUILDER_H

#include "ActsGeometry/ActsElementVector.h"
#include "ActsGeometryInterfaces/IBlueprintNodeBuilder.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"

namespace ActsTrk {

/** Helper class to build the HgtdBlueprint node
 *  It adds the system as a node to the Blueprint.
 */
class HgtdBlueprintNodeBuilder
    : public extends<AthAlgTool, IBlueprintNodeBuilder> {

 public:
  StatusCode initialize() override;
  using base_class::base_class;

  /** @brief Build the HGTD Blueprint Node
   *  @param gctx Geometry context
   *  @param child The child node which is added to the HGTD node.*/
  std::shared_ptr<Acts::BlueprintNode> buildBlueprintNode(
      const Acts::GeometryContext& gctx,
      std::shared_ptr<Acts::BlueprintNode>&& child) override;

 private:
  const HGTD_DetectorManager* m_hgtdMgr{nullptr};

  std::shared_ptr<ActsElementVector> m_elementStore{nullptr};

  /// Base surface-array grid of each HGTD layer
  Gaudi::Property<unsigned int> m_baseBinsR{
      this, "BaseBinsR", 1,
      "Bins in r of the sensitive-surface grid of each HGTD layer, before "
      "NumberOfBinsFactor"};
  Gaudi::Property<unsigned int> m_baseBinsPhi{
      this, "BaseBinsPhi", 50,
      "Bins in phi of the sensitive-surface grid of each HGTD layer, before "
      "NumberOfBinsFactor"};

  /// Scale of the base grid above, along both axes
  Gaudi::Property<double> m_numberOfBinsFactor{
      this, "NumberOfBinsFactor", 5.0,
      "Scale of the BaseBinsR x BaseBinsPhi (r x phi) sensitive-surface grid "
      "of each HGTD "
      "layer (Gen1: NumberOfHgtdBinsFactor of the TrackingGeometrySvc)"};

  /// Material binning: equidistant bins {r, phi} of the material designated on
  /// the HGTD layer discs (the outward disc of each layer and the inward disc
  /// of the innermost one). The bins span the disc, whose extent is set when
  /// the volumes are built. The material map is binned the same way, so a map
  /// made with other values does not fit the geometry.
  Gaudi::Property<std::vector<unsigned int>> m_hgtdDiscMaterialBins{
      this, "HgtdDiscMaterialBins", {50, 50},
      "Material bins {r, phi} of the HGTD layer discs"};

  /** @brief Build the HGTD Blueprint Node
   * @param gctx Geometry context
   * @param node The node to add the HGTD node to */
  void buildHgtdBlueprintNode(const Acts::GeometryContext& gctx,
                              Acts::BlueprintNode& node);

  void addHgtdLayers(Acts::BlueprintNode& parent, int bec, int layer,
                     const std::string& name,
                     std::vector<std::shared_ptr<Acts::Surface>>& surfaces,
                     bool isInnermost);
};

}  // namespace ActsTrk

#endif
