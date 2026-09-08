/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// This absolutely needs to go first to ensure Eigen plugin is loaded
#include "GeoPrimitives/GeoPrimitives.h"
//

#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/Blueprint.hpp>
#include <Acts/Geometry/BlueprintNode.hpp>
#include <Acts/Geometry/ContainerBlueprintNode.hpp>
#include <Acts/Geometry/CylinderVolumeBounds.hpp>
#include <Acts/Geometry/Extent.hpp>
#include <Acts/Geometry/GeometryIdentifierBlueprintNode.hpp>
#include <Acts/Geometry/LayerBlueprintNode.hpp>
#include <Acts/Geometry/MaterialDesignatorBlueprintNode.hpp>
#include <Acts/Geometry/ProtoLayer.hpp>
#include <Acts/Geometry/VolumeAttachmentStrategy.hpp>
#include <Acts/Navigation/SurfaceArrayNavigationPolicy.hpp>
#include <Acts/Navigation/CylinderNavigationPolicy.hpp>
#include <Acts/Navigation/TryAllNavigationPolicy.hpp>
#include <Acts/Surfaces/SurfaceArray.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <Acts/Utilities/AxisSpec.hpp>
#include <cstddef>
#include <format>
#include <ranges>

#include "Acts/Geometry/VolumeResizeStrategy.hpp"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ActsElementVector.h"
#include "ActsInterop/IdentityHelper.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "ItkBlueprintNodeBuilder.h"

using namespace Acts;
using namespace Acts::Experimental;
using namespace Acts::UnitLiterals;

namespace {

using enum Acts::CylinderVolumeBounds::Face;
using enum Acts::AxisDirection;
using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
using AttachmentStrategy = Acts::VolumeAttachmentStrategy;
using ResizeStrategy = Acts::VolumeResizeStrategy;
using namespace ActsTrk::detail::GeoVolIds;

// Helper function to convert shared_ptr vector to const ptr vector
std::vector<const Acts::Surface*> makeConstPtrVector(
    const std::vector<std::shared_ptr<Acts::Surface>>& surfs) {
  std::vector<const Acts::Surface*> constPtrs;
  constPtrs.reserve(surfs.size());
  for (const auto& surf : surfs) {
    constPtrs.push_back(surf.get());
  }
  return constPtrs;
}

// Helper struct to keep ProtoLayer and its associated surfaces together
struct LayerData {
  Acts::ProtoLayer protoLayer;
  std::vector<std::shared_ptr<Acts::Surface>> surfaces;

  LayerData(const Acts::GeometryContext& gctx,
            std::vector<std::shared_ptr<Acts::Surface>> surfs)
      : protoLayer(gctx, makeConstPtrVector(surfs)),
        surfaces(std::move(surfs)) {}
};

// Helper function to merge layers that overlap in z
std::vector<LayerData> mergeLayers(const Acts::GeometryContext& gctx,
                                   std::vector<LayerData> layers) {
  using enum Acts::AxisDirection;

  std::vector<LayerData> mergedLayers;
  if (layers.empty()) {
    return mergedLayers;
  }

  mergedLayers.push_back(std::move(layers.front()));

  for (size_t i = 1; i < layers.size(); i++) {
    auto& current = layers[i];
    auto& prev = mergedLayers.back();

    // Check if they overlap in z
    bool overlap =
        (current.protoLayer.min(AxisZ) <= prev.protoLayer.max(AxisZ) &&
         current.protoLayer.max(AxisZ) >= prev.protoLayer.min(AxisZ));

    if (overlap) {
      // Merge surfaces
      std::vector<std::shared_ptr<Acts::Surface>> mergedSurfaces;
      mergedSurfaces.reserve(current.surfaces.size() + prev.surfaces.size());
      mergedSurfaces.insert(mergedSurfaces.end(), current.surfaces.begin(),
                            current.surfaces.end());
      mergedSurfaces.insert(mergedSurfaces.end(), prev.surfaces.begin(),
                            prev.surfaces.end());

      mergedLayers.pop_back();
      mergedLayers.emplace_back(gctx, std::move(mergedSurfaces));
      auto& merged = mergedLayers.back();
      merged.protoLayer.envelope[AxisR] = current.protoLayer.envelope[AxisR];
      merged.protoLayer.envelope[AxisZ] = current.protoLayer.envelope[AxisZ];
    } else {
      mergedLayers.push_back(std::move(current));
    }
  }

  return mergedLayers;
}

void addStripBarrelLayer(
    Acts::BlueprintNode& parent, std::size_t ilayer,
    const std::vector<std::shared_ptr<Acts::Surface>>& surfaces) {
  using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
  using enum Acts::CylinderVolumeBounds::Face;
  using enum Acts::AxisDirection;

  auto addLayer = [ilayer, &surfaces](auto& node) {
    node.addLayer("Strip_Brl_" + std::to_string(ilayer), [&](auto& layer) {
      layer.setNavigationPolicyFactory(
          Acts::NavigationPolicyFactory{}
              .add<Acts::SurfaceArrayNavigationPolicy>(
                  Acts::SurfaceArrayNavigationPolicy::Config{
                      .layerType = Cylinder, .bins = {0, 0}, .numberOfBinsFactor = 5.0})
              .add<Acts::CylinderNavigationPolicy>()
              .asUniquePtr());

      layer.setSurfaces(surfaces);
      layer.setEnvelope(Acts::ExtentEnvelope{{
          .z = {5_mm, 5_mm},
          .r = {2_mm, 2_mm},
      }});
    });
  };

  // Keep material on each layer's OuterCylinder (the lower-r side of the gap
  // above it); drop the InnerCylinder so it can fuse with the inner
  // neighbour's kept OuterCylinder when the inter-layer gap is collapsed. The
  // innermost layer also drops its InnerCylinder: it is expanded inward onto
  // the StripMaterial inner shell (382.5), whose full-z InnerCylinder material
  // (3mm below) is the surviving surface, so keeping the layer's own
  // InnerCylinder would double-fuse. Surviving material sits at the
  // lower-radius side of each collapsed gap.
  // Layer 3 carries no material; skip the MaterialDesignator wrapper to avoid
  // empty-designator warnings and call addLayer directly on the parent.
  if (ilayer < 3) {
    parent.addMaterial("Strip_Brl_" + std::to_string(ilayer) + "_Material",
                       [&addLayer](auto& lmat) {
                         lmat.configureFace(
                             OuterCylinder,
                             AxisSpec::DeferredEquidistant(20, AxisRPhi),
                             AxisSpec::DeferredEquidistant(20, AxisZ));
                         addLayer(lmat);
                       });
  } else {
    addLayer(parent);
  }
}

void addStripEndcapLayer(
    Acts::BlueprintNode& parent, const std::string& name,
    const std::vector<std::shared_ptr<Acts::Surface>>& surfaces, int bec) {
  using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
  using enum Acts::CylinderVolumeBounds::Face;
  using enum Acts::AxisDirection;

  // Keep material only on the disk's outward disc; the inward disc is dropped
  // so it can fuse with the neighbour's outward disc when the inter-disk gap is
  // collapsed. The innermost disk also drops its inward disc: it is expanded
  // toward the barrel and fuses with the barrel end disc (the kept, smaller-|z|
  // surface). Surviving material sits at the smaller-|z| side of each gap.
  const auto outwardDisc = (bec > 0) ? PositiveDisc : NegativeDisc;
  parent.addMaterial(name + "_Material", [&](auto& mat) {
    mat.configureFace(outwardDisc, AxisSpec::DeferredEquidistant(20, AxisR),
                      AxisSpec::DeferredEquidistant(40, AxisPhi));

    mat.addLayer(name, [&surfaces](auto& layer) {
      layer.setNavigationPolicyFactory(
          Acts::NavigationPolicyFactory{}
              .add<Acts::SurfaceArrayNavigationPolicy>(
                  Acts::SurfaceArrayNavigationPolicy::Config{.layerType = Disc,
                                                             .bins = {0, 0}, .numberOfBinsFactor = 5.0})
              .add<Acts::CylinderNavigationPolicy>()
              .asUniquePtr());

      layer.setSurfaces(surfaces);
      layer.setEnvelope(Acts::ExtentEnvelope{{
          .z = {0.1_mm, 0.1_mm},
          .r = {2_mm, 2_mm},
      }});
    });
  });
}
}  // namespace

namespace ActsTrk {

StatusCode ItkBlueprintNodeBuilder::initialize() {
  // Retrieve the detector managers from the detector store for enabled systems
  if (m_buildPixel) {
    ATH_CHECK(detStore()->retrieve(m_itkPixelMgr, "ITkPixel"));
  }
  if (m_buildStrip) {
    ATH_CHECK(detStore()->retrieve(m_itkStripMgr, "ITkStrip"));
  }
  m_elementStore = std::make_shared<ActsElementVector>();
  return StatusCode::SUCCESS;
}

std::shared_ptr<Acts::BlueprintNode>
ItkBlueprintNodeBuilder::buildBlueprintNode(
    const Acts::GeometryContext& gctx,
    std::shared_ptr<Acts::BlueprintNode>&& childNode) {

  auto itkNode =
      std::make_shared<Acts::CylinderContainerBlueprintNode>(
          "itkNode", AxisZ);
  itkNode->setAttachmentStrategy(AttachmentStrategy::Gap);
  itkNode->setResizeStrategy(ResizeStrategy::Gap);

  auto& itk = itkNode->addCylinderContainer("ItkNodeMain", AxisR);
  itk.setAttachmentStrategy(AttachmentStrategy::Gap);
  itk.setResizeStrategy(ResizeStrategy::Gap);

  itk.addMaterial("ItkNodeMain_Material", [&](auto& mat) {
      if (m_buildStrip) {
        mat.configureFace(NegativeDisc,
                          AxisSpec::DeferredEquidistant(20, AxisR),
                          AxisSpec::DeferredEquidistant(40, AxisPhi));
        mat.configureFace(PositiveDisc,
                          AxisSpec::DeferredEquidistant(20, AxisR),
                          AxisSpec::DeferredEquidistant(40, AxisPhi));
      }

    auto& innerContainer = mat.addCylinderContainer("ITkInnerContainer", AxisR);
    // The radial dead bands between the R-stacked sub-detectors (beampipe /
    // InnerPixel / OuterPixel / Strip) are genuine empty space, but the default
    // Midpoint attachment splits each band at its midpoint and lets BOTH
    // neighbours fill their half with a ResizeStrategy::Gap volume -> two thin
    // gap volumes per interface. Second makes a single neighbour own the whole
    // band -> one gap per interface. Material is unaffected: each sub-detector
    // keeps ResizeStrategy::Gap, so its material-bearing shell stays frozen at
    // its true edge and only the empty band is merged.
    innerContainer.setAttachmentStrategy(AttachmentStrategy::Second);
    innerContainer.setResizeStrategy(ResizeStrategy::Gap);
    // Beam pipe is passed in as an optional child from BeamPipeBlueprintNodeBuilder
    if (childNode) {
      innerContainer.addChild(std::move(childNode));
    }

    if (m_buildPixel) {
      buildItkPixelBlueprintNode(gctx, innerContainer);
    }
    if (m_buildStrip) {
      buildItkStripBlueprintNode(gctx, innerContainer);
    }
  });

  return itkNode;
}

void ItkBlueprintNodeBuilder::buildItkPixelBlueprintNode(
    const Acts::GeometryContext& gctx,
    Acts::BlueprintNode& node) {

  // Get ITkPixel parameters from detector manager
  if (!m_itkPixelMgr) {
    ATH_MSG_ERROR("ITkPixel manager not available");
    throw std::runtime_error("ITkPixel manager not available");
  }

  ATH_MSG_DEBUG("Detector manager has "
                << m_itkPixelMgr->getDetectorElementCollection()->size()
                << " elements");

  std::vector<std::shared_ptr<ActsDetectorElement>> elements;

  InDetDD::SiDetectorElementCollection::const_iterator iter;
  for (const auto* element : *m_itkPixelMgr->getDetectorElementCollection()) {
    const InDetDD::SiDetectorElement* siDetElement =
        dynamic_cast<const InDetDD::SiDetectorElement*>(element);
    if (siDetElement == nullptr) {
      ATH_MSG_ERROR("Detector element was nullptr");
      throw std::runtime_error{"Corrupt detector element collection"};
    }
    elements.push_back(std::make_shared<ActsDetectorElement>(*siDetElement));
  }
  ATH_MSG_VERBOSE("Retrieved " << elements.size() << " elements");

  // Copy to service level store to extend lifetime
  m_elementStore->vector().insert(m_elementStore->vector().end(),
                                  elements.begin(), elements.end());

  // Inner pixel: 2 innermost barrel layers + inner endcap disks
  auto& innerPixel = node.addCylinderContainer("InnerPixel", AxisR);
  innerPixel.setAttachmentStrategy(AttachmentStrategy::Gap);
  // Inner edge Expand: extend all InnerPixel cylinders (endcaps + barrel_0)
  // inward onto the beam pipe, removing InnerPixel::Gap1. The InnerCylinder
  // material (30.9) is dropped below so the expanded face fuses cleanly with
  // the beam pipe outer material (23.9, the kept lower-r surface). Outer edge
  // stays Gap.
  innerPixel.setResizeStrategies(ResizeStrategy::Expand, ResizeStrategy::Gap);

  innerPixel.addMaterial("InnerPixelMaterial", [&](auto& mat) {
    mat.configureFace(OuterCylinder,
                      AxisSpec::DeferredEquidistant(20, AxisRPhi),
                      AxisSpec::DeferredEquidistant(20, AxisZ));

    auto& innerPixelContainer = mat.addCylinderContainer("InnerPixel", AxisZ);

    // Add barrel container
    auto& barrelGeoId = innerPixelContainer.withGeometryIdentifier();
    barrelGeoId.setAllVolumeIdsTo(s_innerPixelVolumeId)
        .incrementLayerIds(1)
        .sortBy([](auto& a, auto& b) {
          auto& boundsA =
              dynamic_cast<const Acts::CylinderVolumeBounds&>(a.volumeBounds());
          auto& boundsB =
              dynamic_cast<const Acts::CylinderVolumeBounds&>(b.volumeBounds());

          using enum Acts::CylinderVolumeBounds::BoundValues;
          double aMidR = (boundsA.get(eMinR) + boundsA.get(eMaxR)) / 2.0;
          double bMidR = (boundsB.get(eMinR) + boundsB.get(eMaxR)) / 2.0;

          return aMidR < bMidR;
        });

    auto& brl_mat =
        barrelGeoId.addMaterial("InnerPixel_Material", [&](auto& material) {
          material.configureFace(NegativeDisc,
                                 AxisSpec::DeferredEquidistant(10, AxisR),
                                 AxisSpec::DeferredEquidistant(10, AxisPhi));
          material.configureFace(PositiveDisc,
                                 AxisSpec::DeferredEquidistant(10, AxisR),
                                 AxisSpec::DeferredEquidistant(10, AxisPhi));
        });
    auto& barrel = brl_mat.addCylinderContainer("InnerPixel_Brl", AxisR);

    // Barrel layers carry material only on their OuterCylinder face. Attaching
    // with Second extends the outer (higher-R) neighbour's material-free inner
    // face inward to meet each layer's outer face, collapsing the inter-layer
    // gap volumes while leaving every material surface fixed.
    barrel.setAttachmentStrategy(AttachmentStrategy::Second);
    // Inner edge Expand: Brl_0's inner cylinder is material-free, so it extends
    // inward onto the beam pipe (the InnerPixel container also expands inward,
    // so the endcaps reach the beam pipe too) removing the barrel's share of the
    // InnerPixel beam-pipe gap.
    // Outer edge Expand: the outermost layer grows out onto the InnerPixel
    // OuterCylinder shell (123.7), removing InnerPixel_Brl::Gap1 (the gap after
    // the last barrel layer). Its own OuterCylinder material is dropped below
    // (kept only up to the second-outermost layer) so the expanded face fuses
    // cleanly with the container shell (the kept surface).
    barrel.setResizeStrategies(ResizeStrategy::Expand, ResizeStrategy::Expand);

    std::map<int, std::vector<std::shared_ptr<Acts::Surface>>> layers{};

    for (auto& element : elements) {
      IdentityHelper id = element->identityHelper();
      if (id.bec() != 0) {
        continue;
      }

      if (id.layer_disk() >= 2) {
        continue;
      }

      int elementLayer = id.layer_disk();
      layers[elementLayer].push_back(element->surface().getSharedPtr());
    }

    ATH_MSG_DEBUG("Adding " << layers.size() << " layers to InnerPixel barrel");

    const int outermostBrlLayer = layers.rbegin()->first;
    for (const auto& [ilayer, surfaces] : layers) {
      ATH_MSG_DEBUG("- Layer " << ilayer << " has " << surfaces.size()
                               << " surfaces");

      auto configureLayer = [&](auto& node) {
        auto& layer = node.addLayer(std::format("InnerPixel_Brl_{}", ilayer));
        layer.setNavigationPolicyFactory(
            Acts::NavigationPolicyFactory{}
                .add<Acts::SurfaceArrayNavigationPolicy>(
                    Acts::SurfaceArrayNavigationPolicy::Config{
                        .layerType = Cylinder,
                        .bins = {0, 0}, .numberOfBinsFactor = 5.0})
                .add<Acts::CylinderNavigationPolicy>()
                .asUniquePtr());
        layer.setSurfaces(surfaces);
        layer.setEnvelope(Acts::ExtentEnvelope{{
            .z = {5_mm, 5_mm},
            .r = {2_mm, 2_mm},
        }});
      };

      // Outermost layer expands onto the container OuterCylinder shell, so
      // drop its own OuterCylinder; skip the MaterialDesignator wrapper to
      // avoid empty-designator warnings.
      if (ilayer != outermostBrlLayer) {
        barrel.addMaterial(
            std::format("InnerPixel_Brl_{}_Material", ilayer),
            [&](auto& lmat) {
              lmat.configureFace(OuterCylinder,
                                 AxisSpec::DeferredEquidistant(40, AxisRPhi),
                                 AxisSpec::DeferredEquidistant(20, AxisZ));
              configureLayer(lmat);
            });
      } else {
        configureLayer(barrel);
      }
    }

    // Add endcap containers
    for (int bec : {-2, 2}) {
      std::string s = bec > 0 ? "p" : "n";
      auto& ecGeoId = innerPixelContainer.withGeometryIdentifier();
      ecGeoId.setAllVolumeIdsTo(s_innerPixelVolumeId + std::floor(bec / 2))
          .incrementLayerIds(1);
      auto& ec = ecGeoId.addCylinderContainer("InnerPixel_" + s + "EC", AxisZ);
      // Collapse the inter-disk gaps by extending the outer disk inward onto
      // its neighbour (Second for +z, First for -z), keeping the material
      // surface at the smaller-|z| side of each gap (see per-disk material
      // handling below).
      ec.setAttachmentStrategy(bec > 0 ? AttachmentStrategy::Second
                                       : AttachmentStrategy::First);
      // Barrel-facing edge Expand: extend the innermost disk toward the barrel
      // (smaller |z|), removing the barrel<->endcap transition gap. That edge is
      // the low-z (inner) side for +z endcaps and the high-z (outer) side for -z
      // endcaps. The far (z-end) edge stays Gap. The innermost disk's inward disc
      // material is dropped below so the expanded face fuses with the barrel end
      // disc (the kept, smaller-|z| surface).
      ec.setResizeStrategies(
          bec > 0 ? ResizeStrategy::Expand : ResizeStrategy::Gap,
          bec > 0 ? ResizeStrategy::Gap : ResizeStrategy::Expand);

      std::map<std::tuple<int, int, int>,
               std::vector<std::shared_ptr<Acts::Surface>>>
          initialLayers{};

      for (auto& element : elements) {
        IdentityHelper id = element->identityHelper();
        if (id.bec() * bec <= 0) {
          continue;
        }

        if (id.layer_disk() >= 3) {
          continue;
        }

        std::tuple<int, int, int> key{id.bec(), id.layer_disk(),
                                      id.eta_module()};
        initialLayers[key].push_back(element->surface().getSharedPtr());
      }

      ATH_MSG_DEBUG("Found " << initialLayers.size()
                             << " initial layers to InnerPixel " << s << "EC");

      std::vector<LayerData> protoLayers;
      protoLayers.reserve(initialLayers.size());

      for (const auto& [key, surfaces] : initialLayers) {
        auto& layer = protoLayers.emplace_back(gctx, surfaces);
        layer.protoLayer.envelope[AxisR] = {2_mm, 2_mm};
        layer.protoLayer.envelope[AxisZ] = {1_mm, 1_mm};
      }

      std::ranges::sort(protoLayers,
                        [](const LayerData& a, const LayerData& b) {
                          return std::abs(a.protoLayer.medium(AxisZ)) <
                                 std::abs(b.protoLayer.medium(AxisZ));
                        });

      ATH_MSG_DEBUG("Found " << protoLayers.size() << " initial layers");

      std::vector<LayerData> mergedLayers;
      if (m_doEndcapLayerMerging) {
        mergedLayers = mergeLayers(gctx, std::move(protoLayers));
      } else {
        mergedLayers = std::move(protoLayers);
      }

      ATH_MSG_DEBUG("After merging: " << mergedLayers.size() << " layers");

      for (const auto [key, pl] : Acts::enumerate(mergedLayers)) {
        ATH_MSG_DEBUG("- Layer " << key << " has " << pl.surfaces.size()
                                 << " surfaces");

        pl.protoLayer.medium(AxisZ);
        auto layerName = std::format("InnerPixel_{}EC_{}", key, s);

        auto addLayer = [&layerName, &pl](auto& parent) {
          auto& layer = parent.addLayer(layerName);

          layer.setNavigationPolicyFactory(
              Acts::NavigationPolicyFactory{}
                  .add<Acts::SurfaceArrayNavigationPolicy>(
                      Acts::SurfaceArrayNavigationPolicy::Config{
                          .layerType = Disc,
                          .bins = {0, 0}, .numberOfBinsFactor = 5.0})
                  .add<Acts::CylinderNavigationPolicy>()
                  .asUniquePtr());

          layer.setSurfaces(pl.surfaces);
          layer.setEnvelope(Acts::ExtentEnvelope{{
              .z = {1_mm, 1_mm},
              .r = {2_mm, 2_mm},
          }});
        };

        ATH_MSG_VERBOSE("Add inner pixel layer"
                        << key << " / " << mergedLayers.size()
                        << " at z = " << pl.protoLayer.medium(AxisZ));
        ATH_MSG_VERBOSE("Adding material for layer " << layerName);
        // Keep material only on each disk's outward disc (away from the IP);
        // the inward disc is dropped so it can fuse with the adjacent disk's
        // kept outward disc without the "both portals carry material" fuse
        // error. The innermost disk drops its inward disc too: it is expanded
        // toward the barrel and fuses with the barrel end disc (the kept,
        // smaller-|z| surface).
        const auto outwardDisc = (bec > 0) ? PositiveDisc : NegativeDisc;
        ec.addMaterial(layerName + "_Material", [&](auto& lmat) {
          lmat.configureFace(outwardDisc,
                             AxisSpec::DeferredEquidistant(10, AxisR),
                             AxisSpec::DeferredEquidistant(40, AxisPhi));
          addLayer(lmat);
        });
      }
    }
  });

  // Outer pixel: 3 outer barrel layers + outer endcaps
  auto& outerPixel = node.addCylinderContainer("OuterPixel", AxisR);
  outerPixel.setAttachmentStrategy(AttachmentStrategy::Gap);
  // Inner edge Expand: extend all OuterPixel cylinders inward onto the
  // InnerPixel outer shell (123.7), removing OuterPixel::Gap1. The InnerCylinder
  // material (147.7) is dropped below so the expanded face fuses cleanly with
  // the InnerPixel OuterCylinder (123.7, the kept lower-r surface). Outer edge
  // stays Gap (the OuterPixel/Strip interface is handled from the Strip side).
  outerPixel.setResizeStrategies(ResizeStrategy::Expand, ResizeStrategy::Gap);

  outerPixel.addMaterial("OuterPixelMaterial", [&](auto& opmat) {
    opmat.configureFace(OuterCylinder,
                        AxisSpec::DeferredEquidistant(20, AxisRPhi),
                        AxisSpec::DeferredEquidistant(20, AxisZ));

    auto& outerPixelContainer = opmat.addCylinderContainer("OuterPixel", AxisZ);

    auto& barrelGeoId = outerPixelContainer.withGeometryIdentifier();
    barrelGeoId.setAllVolumeIdsTo(s_outerPixelVolumeId).incrementLayerIds(1);

    auto& brl_mat =
        barrelGeoId.addMaterial("OuterPixel_Material", [&](auto& bmat) {
          bmat.configureFace(NegativeDisc,
                             AxisSpec::DeferredEquidistant(10, AxisR),
                             AxisSpec::DeferredEquidistant(10, AxisPhi));
          bmat.configureFace(PositiveDisc,
                             AxisSpec::DeferredEquidistant(10, AxisR),
                             AxisSpec::DeferredEquidistant(10, AxisPhi));
        });
    auto& barrel = brl_mat.addCylinderContainer("OuterPixel_Brl", AxisR);

    // See InnerPixel_Brl: layers carry material only on OuterCylinder, so
    // Second attachment removes the inter-layer gaps without moving any
    // material surface.
    barrel.setAttachmentStrategy(AttachmentStrategy::Second);
    // Inner edge Expand: the innermost layer's inner cylinder is material-free,
    // so it extends inward onto the container inner shell and fuses with the
    // OuterPixelMaterial InnerCylinder (one material side, nothing moves).
    // Outer edge Expand: the outermost layer grows out onto the OuterPixel
    // OuterCylinder shell (316.7), removing the gap after the last barrel layer.
    // Its own OuterCylinder material is dropped below so the expanded face fuses
    // cleanly with the container shell (the kept surface).
    barrel.setResizeStrategies(ResizeStrategy::Expand, ResizeStrategy::Expand);

    std::map<int, std::vector<std::shared_ptr<Acts::Surface>>> layers{};

    for (auto& element : elements) {
      IdentityHelper id = element->identityHelper();
      if (id.bec() != 0) {
        continue;
      }

      if (id.layer_disk() <= 1) {
        continue;
      }

      int elementLayer = id.layer_disk();
      layers[elementLayer].push_back(element->surface().getSharedPtr());
    }

    ATH_MSG_DEBUG("Adding " << layers.size() << " layers to OuterPixel barrel");

    const int outermostBrlLayer = layers.rbegin()->first;
    for (const auto& [ilayer, surfaces] : layers) {
      ATH_MSG_DEBUG("- Layer " << ilayer << " has " << surfaces.size()
                               << " surfaces");

      auto configureLayer = [&](auto& node) {
        auto& layer = node.addLayer("OuterPixel_Brl_" + std::to_string(ilayer));
        layer.setNavigationPolicyFactory(
            Acts::NavigationPolicyFactory{}
                .add<Acts::SurfaceArrayNavigationPolicy>(
                    Acts::SurfaceArrayNavigationPolicy::Config{
                        .layerType = Cylinder,
                        .bins = {0, 0}, .numberOfBinsFactor = 5.0})
                .add<Acts::CylinderNavigationPolicy>()
                .asUniquePtr());
        layer.setSurfaces(surfaces);
        layer.setEnvelope(Acts::ExtentEnvelope{{
            .z = {5_mm, 5_mm},
            .r = {2_mm, 2_mm},
        }});
      };

      // Outermost layer expands onto the container OuterCylinder shell, so
      // drop its own OuterCylinder; skip the MaterialDesignator wrapper to
      // avoid empty-designator warnings.
      if (ilayer != outermostBrlLayer) {
        barrel.addMaterial(
            std::format("OuterPixel_Brl_{}_Material", ilayer),
            [&](auto& lmat) {
              lmat.configureFace(OuterCylinder,
                                 AxisSpec::DeferredEquidistant(40, AxisRPhi),
                                 AxisSpec::DeferredEquidistant(20, AxisZ));
              configureLayer(lmat);
            });
      } else {
        configureLayer(barrel);
      }
    }

    constexpr static auto addEndcapLayer = [](auto& parent, const auto& name,
                                              const auto& surfaces) {
      parent.addLayer(name, [&surfaces](auto& layer) {
        layer.setNavigationPolicyFactory(
            Acts::NavigationPolicyFactory{}
                .add<Acts::SurfaceArrayNavigationPolicy>(
                    Acts::SurfaceArrayNavigationPolicy::Config{
                        .layerType = Disc, .bins = {0, 0}, .numberOfBinsFactor = 5.0})
                .add<Acts::CylinderNavigationPolicy>()
                .asUniquePtr());

        layer.setSurfaces(surfaces);
      });
    };

    for (int bec : {-2, 2}) {
      const std::string s = bec > 0 ? "p" : "n";

      auto& ec_outer_geoId = outerPixelContainer.withGeometryIdentifier();
      ec_outer_geoId
          .setAllVolumeIdsTo(s_outerPixelVolumeId + std::floor(bec / 2))
          .incrementLayerIds(1);

      auto& ec_outer =
          ec_outer_geoId.addCylinderContainer("OuterPixel_" + s + "EC", AxisR);

      // Three groups of disks stacked in R
      std::array diskGroups{std::pair{3, 4}, std::pair{6, 5}, std::pair{7, 8}};

      for (size_t idx = 0; idx < diskGroups.size(); ++idx) {
        auto [disk1, disk2] = diskGroups[idx];

        Acts::MaterialDesignatorBlueprintNode* material = nullptr;
        if (idx < (diskGroups.size() - 1)) {
          material = &ec_outer.addMaterial(
              "OuterPixel_" + s + "EC_" + std::to_string(idx) + "_Material",
              [&](auto& mat) {
                mat.configureFace(OuterCylinder,
                                  AxisSpec::DeferredEquidistant(20, AxisRPhi),
                                  AxisSpec::DeferredEquidistant(20, AxisZ));
              });
        }

        auto& ec_stack =
            material
                ? material->addCylinderContainer(
                      "OuterPixel_" + s + "EC_" + std::to_string(idx), AxisZ)
                : ec_outer.addCylinderContainer(
                      "OuterPixel_" + s + "EC_" + std::to_string(idx), AxisZ);

        // Collapse inter-disk gaps by extending the outer disk inward (Second
        // for +z, First for -z); material kept only on outward discs (see
        // below), so surviving material sits at the smaller-|z| side of each gap.
        ec_stack.setAttachmentStrategy(bec > 0 ? AttachmentStrategy::Second
                                               : AttachmentStrategy::First);
        // Barrel-facing edge Expand: extend the innermost disk of each R-group
        // toward the barrel (smaller |z|), removing the barrel<->endcap gap; far
        // (z-end) edge stays Gap. Innermost inward disc dropped below so the
        // expanded face fuses with the barrel end disc (kept, smaller |z|).
        ec_stack.setResizeStrategies(
            bec > 0 ? ResizeStrategy::Expand : ResizeStrategy::Gap,
            bec > 0 ? ResizeStrategy::Gap : ResizeStrategy::Expand);

        std::map<std::tuple<int, int>,
                 std::vector<std::shared_ptr<Acts::Surface>>>
            eta_rings;

        for (auto& element : elements) {
          IdentityHelper id = element->identityHelper();
          if (id.bec() != bec ||
              (id.layer_disk() != disk1 && id.layer_disk() != disk2)) {
            continue;
          }
          std::tuple<int, int> key{id.layer_disk(), id.eta_module()};
          eta_rings[key].push_back(element->surface().getSharedPtr());
        }

        ATH_MSG_DEBUG("Found " << eta_rings.size() << " eta rings in group "
                               << idx);

        std::vector<std::vector<std::shared_ptr<Acts::Surface>>> sorted_rings;
        sorted_rings.reserve(eta_rings.size());
        for (const auto& [key, surfaces] : eta_rings) {
          sorted_rings.push_back(surfaces);
        }

        std::ranges::sort(sorted_rings, [&gctx](const auto& a, const auto& b) {
          Acts::ProtoLayer pl_a(gctx, makeConstPtrVector(a));
          Acts::ProtoLayer pl_b(gctx, makeConstPtrVector(b));
          return std::abs(pl_a.min(AxisZ)) < std::abs(pl_b.min(AxisZ));
        });

        for (size_t i = 0; i < sorted_rings.size(); ++i) {
          const auto& surfaces = sorted_rings[i];
          auto layerName = "OuterPixel_" + s + "EC_" + std::to_string(idx) +
                           "_" + std::to_string(i);

          const auto outwardDisc = (bec > 0) ? PositiveDisc : NegativeDisc;
          ec_stack.addMaterial(layerName + "_Material", [&](auto& mat) {
            mat.configureFace(outwardDisc,
                              AxisSpec::DeferredEquidistant(10, AxisR),
                              AxisSpec::DeferredEquidistant(40, AxisPhi));
            addEndcapLayer(mat, layerName, surfaces);
          });
        }
      }
    }
  });
}

void ItkBlueprintNodeBuilder::buildItkStripBlueprintNode(
    const Acts::GeometryContext& gctx,
    Acts::BlueprintNode& node) {

  // Get ITkStrip parameters from detector manager
  if (!m_itkStripMgr) {
    ATH_MSG_ERROR("ITkStrip manager not available");
    throw std::runtime_error("ITkStrip manager not available");
  }

  ATH_MSG_DEBUG("Detector manager has "
                << m_itkStripMgr->getDetectorElementCollection()->size()
                << " elements");

  std::vector<std::shared_ptr<ActsDetectorElement>> elements;

  InDetDD::SiDetectorElementCollection::const_iterator iter;
  for (const auto* element : *m_itkStripMgr->getDetectorElementCollection()) {
    const InDetDD::SiDetectorElement* siDetElement =
        dynamic_cast<const InDetDD::SiDetectorElement*>(element);
    if (siDetElement == nullptr) {
      ATH_MSG_ERROR("Detector element was nullptr");
      throw std::runtime_error{"Corrupt detector element collection"};
    }
    elements.push_back(std::make_shared<ActsDetectorElement>(*siDetElement));
  }
  ATH_MSG_VERBOSE("Retrieved " << elements.size() << " elements");

  // Copy to service level store to extend lifetime
  m_elementStore->vector().insert(m_elementStore->vector().end(),
                                  elements.begin(), elements.end());

  auto& strip = node.addCylinderContainer("Strip", AxisR);
  strip.setAttachmentStrategy(AttachmentStrategy::Gap);
  // Inner edge Expand: extend all Strip cylinders inward onto the OuterPixel
  // outer shell (316.7), removing Strip::Gap1. The InnerCylinder material
  // (382.5) is dropped below so the expanded face fuses cleanly with the
  // OuterPixel OuterCylinder (316.7, the kept lower-r surface). Outer edge stays
  // Gap (Strip::Gap2, the resize gap up to the ItkNodeMain envelope, carries
  // the Strip outer material and must not move).
  strip.setResizeStrategies(ResizeStrategy::Expand, ResizeStrategy::Gap);

  strip.addMaterial("StripMaterial", [&](auto& mat) {
    mat.configureFace(OuterCylinder,
                      AxisSpec::DeferredEquidistant(20, AxisRPhi),
                      AxisSpec::DeferredEquidistant(20, AxisZ));

    auto& stripContainer = mat.addCylinderContainer("Strip", AxisZ);

    // Add barrel container
    stripContainer.withGeometryIdentifier([this, &elements](auto& geoId) {
      geoId.setAllVolumeIdsTo(s_stripVolumeId).incrementLayerIds(1);

      auto& brl_mat =
          geoId.addMaterial("Strip_Brl_Material", [&](auto& material) {
            material.configureFace(NegativeDisc,
                                   AxisSpec::DeferredEquidistant(10, AxisR),
                                   AxisSpec::DeferredEquidistant(10, AxisPhi));
            material.configureFace(PositiveDisc,
                                   AxisSpec::DeferredEquidistant(10, AxisR),
                                   AxisSpec::DeferredEquidistant(10, AxisPhi));
          });
      brl_mat.addCylinderContainer(
          "Strip_Brl", AxisR, [this, &elements](auto& barrel) {
            // Collapse the inter-layer gaps by extending the outer layer inward
            // (Second); material kept only on OuterCylinder faces (see
            // addStripBarrelLayer) so it stays at the lower-radius side.
            barrel.setAttachmentStrategy(AttachmentStrategy::Second);
            // Inner edge: Expand the innermost layer onto the StripMaterial
            // inner shell (382.5), removing Strip_Brl::Gap1. The layer's own
            // InnerCylinder material (385.5, redundant with the full-z shell
            // 3mm below it) is dropped in addStripBarrelLayer so the expanded
            // face fuses cleanly (one material side). Outer edge stays Gap
            // (invariant, see OuterPixel_Brl).
            barrel.setResizeStrategies(ResizeStrategy::Expand,
                                       ResizeStrategy::Gap);

            std::map<int, std::vector<std::shared_ptr<Acts::Surface>>> layers{};

            for (auto& element : elements) {
              IdentityHelper id = element->identityHelper();
              if (id.bec() != 0) {
                continue;
              }

              int elementLayer = id.layer_disk();
              layers[elementLayer].push_back(element->surface().getSharedPtr());
            }

            ATH_MSG_DEBUG("Adding " << layers.size()
                                    << " layers to Strip barrel");

            for (const auto& [ilayer, surfaces] : layers) {
              ATH_MSG_DEBUG("- Layer " << ilayer << " has " << surfaces.size()
                                       << " surfaces");
              addStripBarrelLayer(barrel, ilayer, surfaces);
            }
          });
    });

    // Add endcap containers
    for (int bec : {-2, 2}) {
      const std::string s = bec > 0 ? "p" : "n";

      std::map<int, std::vector<std::shared_ptr<Acts::Surface>>> layers{};

      for (auto& element : elements) {
        IdentityHelper id = element->identityHelper();
        if (id.bec() * bec <= 0) {
          continue;
        }

        layers[id.layer_disk()].push_back(element->surface().getSharedPtr());
      }

      ATH_MSG_DEBUG("Found " << layers.size() << " layers in Strip " << s
                             << "EC");

      std::vector<std::vector<std::shared_ptr<Acts::Surface>>> sorted_layers;
      sorted_layers.reserve(layers.size());
      for (const auto& [key, surfaces] : layers) {
        sorted_layers.push_back(surfaces);
      }

      std::sort(sorted_layers.begin(), sorted_layers.end(),
                [&gctx](const auto& a, const auto& b) {
                  Acts::ProtoLayer pl_a(gctx, makeConstPtrVector(a));
                  Acts::ProtoLayer pl_b(gctx, makeConstPtrVector(b));
                  return std::abs(pl_a.min(AxisZ)) < std::abs(pl_b.min(AxisZ));
                });

      stripContainer.withGeometryIdentifier([&sorted_layers, bec,
                                             &s](auto& geoId) {
        geoId.setAllVolumeIdsTo(s_stripVolumeId + std::floor(bec / 2))
            .incrementLayerIds(1);

        geoId.addCylinderContainer(
            "Strip_" + s + "EC", AxisZ, [&sorted_layers, &s, bec](auto& ec) {
              // Collapse inter-disk gaps; material kept on outward discs only
              // (see addStripEndcapLayer), so it stays at the smaller-|z| side.
              ec.setAttachmentStrategy(bec > 0 ? AttachmentStrategy::Second
                                               : AttachmentStrategy::First);
              // Barrel-facing edge Expand: extend the innermost disk toward the
              // barrel (smaller |z|), removing the barrel<->endcap gap; far edge
              // stays Gap. Innermost inward disc dropped (addStripEndcapLayer) so
              // the expanded face fuses with the barrel end disc.
              ec.setResizeStrategies(
                  bec > 0 ? ResizeStrategy::Expand : ResizeStrategy::Gap,
                  bec > 0 ? ResizeStrategy::Gap : ResizeStrategy::Expand);

              for (size_t i = 0; i < sorted_layers.size(); ++i) {
                const auto& surfaces = sorted_layers[i];
                auto layerName = "Strip_" + s + "EC_" + std::to_string(i);
                addStripEndcapLayer(ec, layerName, surfaces, bec);
              }
            });
      });
    };
  });

  // Explicit spacer volume beyond the strip detector's natural outer radius.
  // Without it, the outer cylinder carrying the strip container material
  // coincides with the ITk node boundary, causing volume merging issues.
  std::vector<std::shared_ptr<Acts::Surface>> allStripSurfaces;
  allStripSurfaces.reserve(elements.size());
  for (auto& element : elements) {
    allStripSurfaces.push_back(element->surface().getSharedPtr());
  }
  Acts::ProtoLayer stripExtent(gctx, makeConstPtrVector(allStripSurfaces));

  constexpr double spacerClearance = 5_mm;
  constexpr double spacerThickness = 5_mm;
  double spacerRmin = stripExtent.max(AxisR) + spacerClearance;
  double spacerHalfZ = std::max(std::abs(stripExtent.min(AxisZ)),
                                std::abs(stripExtent.max(AxisZ))) +
                       spacerClearance;

  strip.addStaticVolume(
      Acts::Transform3::Identity(),
      std::make_shared<Acts::CylinderVolumeBounds>(
          spacerRmin, spacerRmin + spacerThickness, spacerHalfZ),
      "Strip_OuterSpacer");
}

}  // namespace ActsTrk
