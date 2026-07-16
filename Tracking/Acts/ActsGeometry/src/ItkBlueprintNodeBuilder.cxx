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
#include <Acts/Navigation/TryAllNavigationPolicy.hpp>
#include <Acts/Surfaces/SurfaceArray.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
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
using enum Acts::AxisBoundaryType;
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
    Acts::Experimental::BlueprintNode& parent, std::size_t ilayer,
    const std::vector<std::shared_ptr<Acts::Surface>>& surfaces) {
  using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
  using enum Acts::CylinderVolumeBounds::Face;
  using enum Acts::AxisDirection;
  using enum Acts::AxisBoundaryType;

  auto addLayer = [ilayer, &surfaces](auto& node) {
    node.addLayer("Strip_Brl_" + std::to_string(ilayer), [&](auto& layer) {
      layer.setNavigationPolicyFactory(
          Acts::NavigationPolicyFactory{}
              .add<Acts::SurfaceArrayNavigationPolicy>(
                  Acts::SurfaceArrayNavigationPolicy::Config{
                      .layerType = Cylinder, .bins = {30, 10}})
              .add<Acts::TryAllNavigationPolicy>(
                  Acts::TryAllNavigationPolicy::Config{.sensitives = false})
              .asUniquePtr());

      layer.setSurfaces(surfaces);
      layer.setEnvelope(Acts::ExtentEnvelope{{
          .z = {5_mm, 5_mm},
          .r = {2_mm, 2_mm},
      }});
    });
  };

  // Inner 3 layers: add material on inner and outer cylinders
  // Outermost layer: add material only on inner cylinder
  parent.addMaterial("Strip_Brl_" + std::to_string(ilayer) + "_Material",
                     [&addLayer, &ilayer](auto& lmat) {
                       if (ilayer < 3) {
                         lmat.configureFace(OuterCylinder,
                                            {AxisRPhi, Closed, 20},
                                            {AxisZ, Bound, 20});
                       }
                       lmat.configureFace(InnerCylinder, {AxisRPhi, Closed, 20},
                                          {AxisZ, Bound, 20});
                       addLayer(lmat);
                     });
}

void addStripEndcapLayer(
    Acts::Experimental::BlueprintNode& parent, const std::string& name,
    const std::vector<std::shared_ptr<Acts::Surface>>& surfaces) {
  using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
  using enum Acts::CylinderVolumeBounds::Face;
  using enum Acts::AxisDirection;
  using enum Acts::AxisBoundaryType;

  parent.addMaterial(name + "_Material", [&](auto& mat) {
    mat.configureFace(PositiveDisc, {AxisR, Bound, 20}, {AxisPhi, Closed, 40});
    mat.configureFace(NegativeDisc, {AxisR, Bound, 20}, {AxisPhi, Closed, 40});

    mat.addLayer(name, [&surfaces](auto& layer) {
      layer.setNavigationPolicyFactory(
          Acts::NavigationPolicyFactory{}
              .add<Acts::SurfaceArrayNavigationPolicy>(
                  Acts::SurfaceArrayNavigationPolicy::Config{.layerType = Disc,
                                                             .bins = {30, 30}})
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

std::shared_ptr<Acts::Experimental::BlueprintNode>
ItkBlueprintNodeBuilder::buildBlueprintNode(
    const Acts::GeometryContext& gctx,
    std::shared_ptr<Acts::Experimental::BlueprintNode>&& childNode) {

  auto itkNode =
      std::make_shared<Acts::Experimental::CylinderContainerBlueprintNode>(
          "itkNode", AxisZ);
  itkNode->setAttachmentStrategy(AttachmentStrategy::Gap);
  itkNode->setResizeStrategy(ResizeStrategy::Gap);

  auto& itk = itkNode->addCylinderContainer("ItkNodeMain", AxisR);
  itk.setAttachmentStrategy(AttachmentStrategy::Gap);
  itk.setResizeStrategy(ResizeStrategy::Gap);

  itk.addMaterial("ItkNodeMain_Material", [&](auto& mat) {
    mat.configureFace(NegativeDisc, {AxisR, Bound, 20}, {AxisPhi, Closed, 40});
    mat.configureFace(PositiveDisc, {AxisR, Bound, 20}, {AxisPhi, Closed, 40});

    auto& innerContainer = mat.addCylinderContainer("ITkInnerContainer", AxisR);

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
    Acts::Experimental::BlueprintNode& node) {

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
  innerPixel.setResizeStrategy(ResizeStrategy::Gap);

  innerPixel.addMaterial("InnerPixelMaterial", [&](auto& mat) {
    mat.configureFace(OuterCylinder, {AxisRPhi, Closed, 20},
                      {AxisZ, Bound, 20});
    mat.configureFace(InnerCylinder, {AxisRPhi, Closed, 20},
                      {AxisZ, Bound, 20});

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
        barrelGeoId.addMaterial("InnerPixel_Material", [&](auto& mat) {
          mat.configureFace(NegativeDisc, {AxisR, Bound, 10},
                            {AxisPhi, Closed, 10});
          mat.configureFace(PositiveDisc, {AxisR, Bound, 10},
                            {AxisPhi, Closed, 10});
        });
    auto& barrel = brl_mat.addCylinderContainer("InnerPixel_Brl", AxisR);

    barrel.setAttachmentStrategy(AttachmentStrategy::Gap);
    barrel.setResizeStrategy(ResizeStrategy::Gap);

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

    for (const auto& [ilayer, surfaces] : layers) {
      ATH_MSG_DEBUG("- Layer " << ilayer << " has " << surfaces.size()
                               << " surfaces");

      barrel.addMaterial(
          std::format("InnerPixel_Brl_{}_Material", ilayer), [&](auto& lmat) {
            lmat.configureFace(OuterCylinder, {AxisRPhi, Closed, 40},
                               {AxisZ, Bound, 20});

            auto& layer =
                lmat.addLayer(std::format("InnerPixel_Brl_{}", ilayer));

            layer.setNavigationPolicyFactory(
                Acts::NavigationPolicyFactory{}
                    .add<Acts::SurfaceArrayNavigationPolicy>(
                        Acts::SurfaceArrayNavigationPolicy::Config{
                            .layerType = Cylinder,
                            .bins = {30, 10}})
                    .add<Acts::TryAllNavigationPolicy>(
                        Acts::TryAllNavigationPolicy::Config{.sensitives =
                                                                 false})
                    .asUniquePtr());

            layer.setSurfaces(surfaces);
            layer.setEnvelope(Acts::ExtentEnvelope{{
                .z = {5_mm, 5_mm},
                .r = {2_mm, 2_mm},
            }});
          });
    }

    // Add endcap containers
    for (int bec : {-2, 2}) {
      std::string s = bec > 0 ? "p" : "n";
      auto& ecGeoId = innerPixelContainer.withGeometryIdentifier();
      ecGeoId.setAllVolumeIdsTo(s_innerPixelVolumeId + std::floor(bec / 2))
          .incrementLayerIds(1);
      auto& ec = ecGeoId.addCylinderContainer("InnerPixel_" + s + "EC", AxisZ);
      ec.setAttachmentStrategy(AttachmentStrategy::Gap);
      ec.setResizeStrategy(ResizeStrategy::Gap);

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
                          .bins = {30, 30}})
                  .add<Acts::TryAllNavigationPolicy>(
                      Acts::TryAllNavigationPolicy::Config{.sensitives = false})
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
        ec.addMaterial(layerName + "_Material", [&](auto& lmat) {
          lmat.configureFace(NegativeDisc, {AxisR, Bound, 10},
                             {AxisPhi, Closed, 40});
          lmat.configureFace(PositiveDisc, {AxisR, Bound, 10},
                             {AxisPhi, Closed, 40});
          addLayer(lmat);
        });
      }
    }
  });

  // Outer pixel: 3 outer barrel layers + outer endcaps
  auto& outerPixel = node.addCylinderContainer("OuterPixel", AxisR);
  outerPixel.setAttachmentStrategy(AttachmentStrategy::Gap);
  outerPixel.setResizeStrategy(ResizeStrategy::Gap);

  outerPixel.addMaterial("OuterPixelMaterial", [&](auto& opmat) {
    opmat.configureFace(OuterCylinder, {AxisRPhi, Bound, 20}, {AxisZ, Bound, 20});
    opmat.configureFace(InnerCylinder, {AxisRPhi, Bound, 20}, {AxisZ, Bound, 20});

    auto& outerPixelContainer = opmat.addCylinderContainer("OuterPixel", AxisZ);

    auto& barrelGeoId = outerPixelContainer.withGeometryIdentifier();
    barrelGeoId.setAllVolumeIdsTo(s_outerPixelVolumeId).incrementLayerIds(1);

    auto& brl_mat =
        barrelGeoId.addMaterial("OuterPixel_Material", [&](auto& bmat) {
          bmat.configureFace(NegativeDisc, {AxisR, Bound, 10},
                            {AxisPhi, Closed, 10});
          bmat.configureFace(PositiveDisc, {AxisR, Bound, 10},
                            {AxisPhi, Closed, 10});
        });
    auto& barrel = brl_mat.addCylinderContainer("OuterPixel_Brl", AxisR);

    barrel.setAttachmentStrategy(AttachmentStrategy::Gap);
    barrel.setResizeStrategy(ResizeStrategy::Gap);

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

    for (const auto& [ilayer, surfaces] : layers) {
      ATH_MSG_DEBUG("- Layer " << ilayer << " has " << surfaces.size()
                               << " surfaces");

      barrel.addMaterial(
          std::format("OuterPixel_Brl_{}_Material", ilayer), [&](auto& lmat) {
            lmat.configureFace(OuterCylinder, {AxisRPhi, Closed, 40},
                               {AxisZ, Bound, 20});

            auto& layer =
                lmat.addLayer("OuterPixel_Brl_" + std::to_string(ilayer));

            layer.setNavigationPolicyFactory(
                Acts::NavigationPolicyFactory{}
                    .add<Acts::SurfaceArrayNavigationPolicy>(
                        Acts::SurfaceArrayNavigationPolicy::Config{
                            .layerType = Cylinder,
                            .bins = {30, 10}})
                    .add<Acts::TryAllNavigationPolicy>(
                        Acts::TryAllNavigationPolicy::Config{.sensitives =
                                                                 false})
                    .asUniquePtr());

            layer.setSurfaces(surfaces);
            layer.setEnvelope(Acts::ExtentEnvelope{{
                .z = {5_mm, 5_mm},
                .r = {2_mm, 2_mm},
            }});
          });
    }

    constexpr static auto addEndcapLayer = [](auto& parent, const auto& name,
                                              const auto& surfaces) {
      parent.addLayer(name, [&surfaces](auto& layer) {
        layer.setNavigationPolicyFactory(
            Acts::NavigationPolicyFactory{}
                .add<Acts::SurfaceArrayNavigationPolicy>(
                    Acts::SurfaceArrayNavigationPolicy::Config{
                        .layerType = Disc, .bins = {30, 30}})
                .add<Acts::TryAllNavigationPolicy>(
                    Acts::TryAllNavigationPolicy::Config{.sensitives = false})
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

        Acts::Experimental::MaterialDesignatorBlueprintNode* material = nullptr;
        if (idx < (diskGroups.size() - 1)) {
          material = &ec_outer.addMaterial(
              "OuterPixel_" + s + "EC_" + std::to_string(idx) + "_Material",
              [&](auto& mat) {
                mat.configureFace(OuterCylinder, {AxisRPhi, Closed, 20},
                                  {AxisZ, Bound, 20});
              });
        }

        auto& ec_stack =
            material
                ? material->addCylinderContainer(
                      "OuterPixel_" + s + "EC_" + std::to_string(idx), AxisZ)
                : ec_outer.addCylinderContainer(
                      "OuterPixel_" + s + "EC_" + std::to_string(idx), AxisZ);

        ec_stack.setAttachmentStrategy(AttachmentStrategy::Gap);
        ec_stack.setResizeStrategy(ResizeStrategy::Gap);

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

          ec_stack.addMaterial(layerName + "_Material", [&](auto& mat) {
            mat.configureFace(PositiveDisc, {AxisR, Bound, 10}, {AxisPhi, Closed, 40});
            mat.configureFace(NegativeDisc, {AxisR, Bound, 10}, {AxisPhi, Closed, 40});
            addEndcapLayer(mat, layerName, surfaces);
          });
        }
      }
    }
  });
}

void ItkBlueprintNodeBuilder::buildItkStripBlueprintNode(
    const Acts::GeometryContext& gctx,
    Acts::Experimental::BlueprintNode& node) {

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
  strip.setResizeStrategy(ResizeStrategy::Gap);

  strip.addMaterial("StripMaterial", [&](auto& mat) {
    mat.configureFace(OuterCylinder, {AxisRPhi, Closed, 20},
                      {AxisZ, Bound, 20});
    mat.configureFace(InnerCylinder, {AxisRPhi, Closed, 20},
                      {AxisZ, Bound, 20});

    auto& stripContainer = mat.addCylinderContainer("Strip", AxisZ);

    // Add barrel container
    stripContainer.withGeometryIdentifier([this, &elements](auto& geoId) {
      geoId.setAllVolumeIdsTo(s_stripVolumeId).incrementLayerIds(1);

      auto& brl_mat = geoId.addMaterial("Strip_Brl_Material", [&](auto& thisMat) {
        thisMat.configureFace(NegativeDisc, {AxisR, Bound, 10},
                          {AxisPhi, Closed, 10});
        thisMat.configureFace(PositiveDisc, {AxisR, Bound, 10},
                          {AxisPhi, Closed, 10});
      });
      brl_mat.addCylinderContainer(
          "Strip_Brl", AxisR, [this, &elements](auto& barrel) {
            barrel.setAttachmentStrategy(AttachmentStrategy::Gap);
            barrel.setResizeStrategy(ResizeStrategy::Gap);

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
            "Strip_" + s + "EC", AxisZ, [&sorted_layers, &s](auto& ec) {
              ec.setAttachmentStrategy(AttachmentStrategy::Gap);
              ec.setResizeStrategy(ResizeStrategy::Gap);

              for (size_t i = 0; i < sorted_layers.size(); ++i) {
                const auto& surfaces = sorted_layers[i];
                auto layerName = "Strip_" + s + "EC_" + std::to_string(i);
                addStripEndcapLayer(ec, layerName, surfaces);
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
