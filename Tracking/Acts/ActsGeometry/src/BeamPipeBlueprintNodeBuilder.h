/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_BEAMPIPEBLUEPRINTNODEBUILDER_H
#define ACTSGEOMETRY_BEAMPIPEBLUEPRINTNODEBUILDER_H

#include "ActsGeometryInterfaces/IBlueprintNodeBuilder.h"

#include <vector>

class BeamPipeDetectorManager;

namespace ActsTrk {

/** Helper class to build the BeamPipe Blueprint node
 *  It adds the beam pipe as a node to the Blueprint.
 */
class BeamPipeBlueprintNodeBuilder
    : public extends<AthAlgTool, IBlueprintNodeBuilder> {

 public:
  StatusCode initialize() override;
  using base_class::base_class;

  /** @brief Build the BeamPipe Blueprint Node
   *  @param gctx Geometry context
   *  @param child Expected to be null; beam pipe has no inner child. */
  std::shared_ptr<Acts::BlueprintNode> buildBlueprintNode(
      const Acts::GeometryContext& gctx,
      std::shared_ptr<Acts::BlueprintNode>&& child) override;

 private:
  const BeamPipeDetectorManager* m_beamPipeMgr{nullptr};

  Gaudi::Property<bool> m_loadfromDatabase{
      this, "loadFromDatabase", true,
      "Whether to load the beam pipe geometry from the geometry database. If "
      "false, a default tube will be created based on some default value."};

  Gaudi::Property<double> m_defaultInnerRadius{
      this, "defaultInnerRadius", 20.,
      "The inner radius of the beam pipe to use if not loading from the "
      "database."};

  /// Material binning: equidistant bins {r*phi, z} of the material designated
  /// on the outer cylinder of the beam pipe. The bins span the cylinder, whose
  /// extent is set when the volumes are built. The material map is binned the
  /// same way, so a map made with other values does not fit the geometry.
  Gaudi::Property<std::vector<unsigned int>> m_beamPipeCylinderMaterialBins{
      this, "BeamPipeCylinderMaterialBins", {1, 35},
      "Material bins {r*phi, z} of the beam pipe outer cylinder"};
};

}  // namespace ActsTrk

#endif
