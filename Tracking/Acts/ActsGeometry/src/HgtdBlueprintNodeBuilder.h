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

  /** @brief Build the HGTD Blueprint Node
   * @param gctx Geometry context
   * @param node The node to add the HGTD node to */
  void buildHgtdBlueprintNode(const Acts::GeometryContext& gctx,
                              Acts::BlueprintNode& node);

  void addHgtdLayers(Acts::BlueprintNode& parent, int bec,
                     int layer, const std::string& name,
                     std::vector<std::shared_ptr<Acts::Surface>>& surfaces);
};

}  // namespace ActsTrk

#endif
