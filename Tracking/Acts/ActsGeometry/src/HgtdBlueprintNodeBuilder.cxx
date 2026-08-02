/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// This absolutely needs to go first to ensure Eigen plugin is loaded
#include "GeoPrimitives/GeoPrimitives.h"
//
#include <GeoModelKernel/GeoTube.h>
#include <GeoModelKernel/GeoVPhysVol.h>

#include <Acts/Definitions/Direction.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/Blueprint.hpp>
#include <Acts/Geometry/BlueprintNode.hpp>
#include <Acts/Geometry/ContainerBlueprintNode.hpp>
#include <Acts/Geometry/CylinderVolumeBounds.hpp>
#include <Acts/Geometry/Extent.hpp>
#include <Acts/Geometry/GeometryIdentifierBlueprintNode.hpp>
#include <Acts/Geometry/LayerBlueprintNode.hpp>
#include <Acts/Geometry/MaterialDesignatorBlueprintNode.hpp>
#include <Acts/Geometry/PadBlueprintNode.hpp>
#include <Acts/Geometry/ProtoLayer.hpp>
#include <Acts/Geometry/TrackingVolume.hpp>
#include <Acts/Geometry/VolumeAttachmentStrategy.hpp>
#include <Acts/Navigation/SurfaceArrayNavigationPolicy.hpp>
#include <Acts/Navigation/TryAllNavigationPolicy.hpp>
#include <Acts/Surfaces/SurfaceArray.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <cstddef>
#include <ranges>
#include <string>

#include "Acts/Geometry/StaticBlueprintNode.hpp"
#include "Acts/Geometry/VolumeResizeStrategy.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ActsElementVector.h"
#include "ActsInterop/IdentityHelper.h"
#include "ActsInterop/Logger.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "HgtdBlueprintNodeBuilder.h"

using namespace Acts;
using namespace Acts::Experimental;
using namespace Acts::UnitLiterals;

namespace {
/// The volume IDs for the HGTD volumes.
using namespace ActsTrk::detail::GeoVolIds;
using enum Acts::CylinderVolumeBounds::Face;
using enum Acts::AxisDirection;
using enum Acts::AxisBoundaryType;
using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
using AttachmentStrategy = Acts::VolumeAttachmentStrategy;
using ResizeStrategy = Acts::VolumeResizeStrategy;
}  // namespace

namespace ActsTrk {
StatusCode HgtdBlueprintNodeBuilder::initialize() {
  ATH_MSG_DEBUG("Initializing HgtdBlueprintNodeBuilder");

  ATH_CHECK(detStore()->retrieve(m_hgtdMgr, "HGTD"));

  m_elementStore = std::make_shared<ActsElementVector>();

  return StatusCode::SUCCESS;
}

std::shared_ptr<Acts::BlueprintNode>
HgtdBlueprintNodeBuilder::buildBlueprintNode(
    const Acts::GeometryContext& gctx,
    std::shared_ptr<Acts::BlueprintNode>&& child) {

  ExtentEnvelope envelope = ExtentEnvelope{{
      .z = {20_mm, 20_mm},
      .r = {0_mm, 20_mm},
  }};
  auto itkHgtdPad = std::make_shared<Acts::PadBlueprintNode>(
      "itkHgtdPad", envelope);

  auto itkHgtdNode =
      std::make_shared<Acts::CylinderContainerBlueprintNode>(
          "itkHgtd", AxisZ);

  if (child) {
    itkHgtdNode->addChild(std::move(child));
  }
  buildHgtdBlueprintNode(gctx, *itkHgtdNode);

  itkHgtdPad->addChild(itkHgtdNode);
  return itkHgtdPad;
}

void HgtdBlueprintNodeBuilder::buildHgtdBlueprintNode(
    const Acts::GeometryContext& /*gctx*/,
    Acts::BlueprintNode& node) {
  if (not m_hgtdMgr) {
    ATH_MSG_ERROR("HGTD manager not available");
    throw std::runtime_error("HGTD manager not available");
  }

  ATH_MSG_DEBUG("Detector manager has "
                << m_hgtdMgr->getDetectorElementCollection()->size()
                << " elements");

  std::vector<std::shared_ptr<ActsDetectorElement>> elements;

  for (const auto* element : *m_hgtdMgr->getDetectorElementCollection()) {
    const InDetDD::HGTD_DetectorElement* hgtdDetElement =
        dynamic_cast<const InDetDD::HGTD_DetectorElement*>(element);
    if (hgtdDetElement == nullptr) {
      ATH_MSG_ERROR("Detector element was nullptr");
      throw std::runtime_error{"Corrupt detector element collection"};
    }
    elements.push_back(std::make_shared<ActsDetectorElement>(
        *hgtdDetElement, hgtdDetElement->identify()));
  }
  ATH_MSG_VERBOSE("Retrieved " << elements.size() << " elements");

  m_elementStore->vector().insert(m_elementStore->vector().end(),
                                  elements.begin(), elements.end());

  for (int bec : {-2, 2}) {
    const std::string s = bec > 0 ? "p" : "n";

    std::map<int, std::vector<std::shared_ptr<Acts::Surface>>> layers{};

    for (auto& element : elements) {
      IdentityHelper id = element->identityHelper();

      ATH_MSG_VERBOSE("Reading element with bec "
                      << id.bec() << ", layer/disk " << id.layer_disk()
                      << ", eta_module " << id.eta_module() << ", phi_module "
                      << id.phi_module());

      if (id.bec() * bec <= 0) {
        continue;
      }

      layers[id.layer_disk()].push_back(element->surface().getSharedPtr());
    }

    ATH_MSG_DEBUG("Found " << layers.size() << " layers in HGTD " << s << "EC");
    for (auto& [key, surfaces] : layers) {
      ATH_MSG_DEBUG("Layer " << key << " has " << surfaces.size()
                             << " surfaces");
    }

    node.withGeometryIdentifier([&layers, bec, s, this](auto& geoId) {
      std::string ecName = "HGTD_" + s + "EC";
      geoId.setAllVolumeIdsTo((bec > 0 ? s_hgtdPosVolumeId : s_hgtdNegVolumeId))
          .incrementLayerIds(1);

      geoId.addCylinderContainer(ecName, AxisR, [&](auto& hgtd) {
        hgtd.setAttachmentStrategy(AttachmentStrategy::Gap);
        hgtd.setResizeStrategy(ResizeStrategy::Gap);

        hgtd.addCylinderContainer(
            ecName + "_Container", AxisZ, [&](auto& hgtdContainer) {
              hgtdContainer.setAttachmentStrategy(AttachmentStrategy::Gap);
              hgtdContainer.setResizeStrategy(ResizeStrategy::Gap);

              for (auto& [key, surfaces] : layers) {
                std::string layerName = ecName + "_" + std::to_string(key);

                ATH_MSG_DEBUG("Adding layer " << layerName << " with "
                                              << surfaces.size()
                                              << " surfaces");

                addHgtdLayers(hgtdContainer, bec, key, layerName, surfaces);
              }
            });
      });
    });
  }
}

void HgtdBlueprintNodeBuilder::addHgtdLayers(
    Acts::BlueprintNode& parent, int bec, int index,
    const std::string& name,
    std::vector<std::shared_ptr<Acts::Surface>>& surfaces) {
  using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
  using enum Acts::CylinderVolumeBounds::Face;
  using enum Acts::AxisDirection;
  using enum Acts::AxisBoundaryType;

  parent.addMaterial(name + "_Material", [&](auto& mat) {
    if (index == 3) {
      mat.configureFace(bec > 0 ? NegativeDisc : PositiveDisc,
                        {AxisR, Bound, 20}, {AxisPhi, Closed, 40});
    } else {
      mat.configureFace(NegativeDisc, {AxisR, Bound, 20},
                        {AxisPhi, Closed, 40});
      mat.configureFace(PositiveDisc, {AxisR, Bound, 20},
                        {AxisPhi, Closed, 40});
    }

    mat.addLayer(name, [&surfaces](auto& layer) {
      layer.setNavigationPolicyFactory(
          Acts::NavigationPolicyFactory{}
              .add<Acts::SurfaceArrayNavigationPolicy>(
                  Acts::SurfaceArrayNavigationPolicy::Config{.layerType = Disc,
                                                             .bins = {20, 20}})
              .add<Acts::TryAllNavigationPolicy>(
                  Acts::TryAllNavigationPolicy::Config{.sensitives = false})
              .asUniquePtr());
      layer.setSurfaces(surfaces);
      layer.setEnvelope(Acts::ExtentEnvelope{{
          .z = {0.1_mm, 0.1_mm},
          .r = {2_mm, 2_mm},
      }});
    });
  });
}

}  // namespace ActsTrk
