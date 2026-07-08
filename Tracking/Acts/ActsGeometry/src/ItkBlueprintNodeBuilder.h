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
  std::shared_ptr<Acts::Experimental::BlueprintNode> buildBlueprintNode(
      const Acts::GeometryContext& gctx,
      std::shared_ptr<Acts::Experimental::BlueprintNode>&& child) override;

 private:
  const InDetDD::SiDetectorManager* m_itkPixelMgr{nullptr};
  const InDetDD::SiDetectorManager* m_itkStripMgr{nullptr};
  std::shared_ptr<ActsElementVector> m_elementStore{nullptr};

  Gaudi::Property<bool> m_doEndcapLayerMerging{this, "doEndcapLayerMerging",
                                                true};
  Gaudi::Property<bool> m_buildPixel{this, "buildPixel", true};
  Gaudi::Property<bool> m_buildStrip{this, "buildStrip", true};

  void buildItkStripBlueprintNode(const Acts::GeometryContext& gctx,
                                  Acts::Experimental::BlueprintNode& node);
  void buildItkPixelBlueprintNode(const Acts::GeometryContext& gctx,
                                  Acts::Experimental::BlueprintNode& node);
};

}  // namespace ActsTrk

#endif
