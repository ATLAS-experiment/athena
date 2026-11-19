/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// This absolutely needs to go first to ensure Eigen plugin is loaded
#include "GeoPrimitives/GeoPrimitives.h"
//

#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/Blueprint.hpp"
#include "Acts/Geometry/BlueprintNode.hpp"
#include "Acts/Geometry/ContainerBlueprintNode.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/Extent.hpp"
#include "Acts/Geometry/GeometryIdentifierBlueprintNode.hpp"
#include "Acts/Geometry/LayerBlueprintNode.hpp"
#include "Acts/Geometry/MaterialDesignatorBlueprintNode.hpp"
#include "Acts/Geometry/ProtoLayer.hpp"
#include "Acts/Geometry/VolumeAttachmentStrategy.hpp"
#include "Acts/Geometry/PadBlueprintNode.hpp"
#include "Acts/Navigation/SurfaceArrayNavigationPolicy.hpp"
#include "Acts/Navigation/CylinderNavigationPolicy.hpp"
#include "Acts/Navigation/TryAllNavigationPolicy.hpp"
#include "Acts/Surfaces/SurfaceArray.hpp"
#include "Acts/Utilities/AxisDefinitions.hpp"
#include "Acts/Utilities/AxisSpec.hpp"
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <optional>
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


/// Convert a {max0, max1} bin count property into a neighbor window without a floor
std::optional<Acts::SurfaceArray::NeighborWindow> toNeighborWindow(
    const std::vector<unsigned int>& maxBins) {
  constexpr unsigned int limit = std::numeric_limits<std::uint8_t>::max();
  if (maxBins.size() != 2 || maxBins[0] > limit || maxBins[1] > limit) {
    return std::nullopt;
  }
  return Acts::SurfaceArray::NeighborWindow{
      {0, 0},
      {static_cast<std::uint8_t>(maxBins[0]), static_cast<std::uint8_t>(maxBins[1])}};
}

/// A material binning property holds two positive bin counts
bool isValidMaterialBins(const std::vector<unsigned int>& bins) {
  return bins.size() == 2 && bins[0] > 0 && bins[1] > 0;
}

/// Designate material on a cylinder face, binned {r*phi, z}
void configureCylinderMaterial(Acts::MaterialDesignatorBlueprintNode& mat,
                               Acts::CylinderVolumeBounds::Face face,
                               const std::vector<unsigned int>& bins,
                               const std::string& materialKey) {
  mat.configureFace(face, AxisSpec::DeferredEquidistant(bins[0], AxisRPhi),
                    AxisSpec::DeferredEquidistant(bins[1], AxisZ), materialKey);
}

/// Designate material on a disc face, binned {r, phi}
void configureDiscMaterial(Acts::MaterialDesignatorBlueprintNode& mat,
                           Acts::CylinderVolumeBounds::Face face,
                           const std::vector<unsigned int>& bins,
                           const std::string& materialKey) {
  mat.configureFace(face, AxisSpec::DeferredEquidistant(bins[0], AxisR),
                    AxisSpec::DeferredEquidistant(bins[1], AxisPhi), materialKey);
}

// Helper struct to keep ProtoLayer and its associated surfaces together
struct LayerData {
  Acts::ProtoLayer protoLayer;
  std::vector<std::shared_ptr<Acts::Surface>> surfaces;

  LayerData(const Acts::GeometryContext& gctx,
            std::vector<std::shared_ptr<Acts::Surface>> surfs)
      : protoLayer(gctx, Acts::unpackConstSmartPointers(surfs)),
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
    const std::vector<std::shared_ptr<Acts::Surface>>& surfaces,
    const Acts::SurfaceArrayNavigationPolicy::Config& surfaceArrayConfig,
    const std::vector<unsigned int>& materialBins) {
  using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
  using enum Acts::CylinderVolumeBounds::Face;
  using enum Acts::AxisDirection;

  // For the strip barrel, the material is designated only on the InnerCylinder
  // (in front of the sensors) of every layer.
  const std::string materialName = std::format("Strip_Brl_{}_Material", ilayer);
  parent.addMaterial(materialName, [&](auto& lmat) {
    configureCylinderMaterial(lmat, InnerCylinder, materialBins, materialName);

    lmat.addLayer(std::format("Strip_Brl_{:}", ilayer), [&](auto& layer) {
      layer.setNavigationPolicyFactory(
          Acts::NavigationPolicyFactory{}
              .add<Acts::SurfaceArrayNavigationPolicy>(surfaceArrayConfig)
              .add<Acts::CylinderNavigationPolicy>()
              .asUniquePtr());

      layer.setSurfaces(surfaces);
      layer.setEnvelope(Acts::ExtentEnvelope{{
          .z = {5_mm, 5_mm},
          .r = {2_mm, 2_mm},
      }});
    });
  });
}

void addStripEndcapLayer(
    Acts::BlueprintNode& parent, const std::string& name,
    const std::vector<std::shared_ptr<Acts::Surface>>& surfaces, int bec,
    const Acts::SurfaceArrayNavigationPolicy::Config& surfaceArrayConfig,
    const std::vector<unsigned int>& materialBins) {
  using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
  using enum Acts::CylinderVolumeBounds::Face;
  using enum Acts::AxisDirection;

  // For the strip endcap, the material is designated only on the
  // inward disc (facing the IP).
  const auto inwardDisc = (bec > 0) ? NegativeDisc : PositiveDisc;
  const std::string materialName = name + "_Material";
  parent.addMaterial(materialName, [&](auto& mat) {
    configureDiscMaterial(mat, inwardDisc, materialBins, materialName);

    mat.addLayer(name, [&surfaces, &surfaceArrayConfig](auto& layer) {
      layer.setNavigationPolicyFactory(
          Acts::NavigationPolicyFactory{}
              .add<Acts::SurfaceArrayNavigationPolicy>(surfaceArrayConfig)
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
  for (const Gaudi::Property<std::vector<unsigned int>>* window :
       {&m_itkPixelInnerBarrelNeighborWindow, &m_itkPixelInnerEndcapNeighborWindow,
        &m_itkPixelOuterBarrelNeighborWindow, &m_itkPixelOuterEndcapNeighborWindow,
        &m_itkStripBarrelNeighborWindow, &m_itkStripInnerBarrelNeighborWindow,
        &m_itkStripEndcapNeighborWindow}) {
    if (!toNeighborWindow(window->value()).has_value()) {
      ATH_MSG_FATAL((*window)
                    << " must hold two bin counts of at most "
                    << static_cast<unsigned int>(std::numeric_limits<std::uint8_t>::max()));
      return StatusCode::FAILURE;
    }
  }
  if (m_surfaceArrayOverfill.value() > std::numeric_limits<std::uint8_t>::max()) {
    ATH_MSG_FATAL(m_surfaceArrayOverfill
                  << " exceeds " << static_cast<unsigned int>(std::numeric_limits<std::uint8_t>::max()));
    return StatusCode::FAILURE;
  }
  if (!(m_itkStripBarrelBinFactor.value() > 0.)) {
    ATH_MSG_FATAL(m_itkStripBarrelBinFactor << " must be positive");
    return StatusCode::FAILURE;
  }
  for (const Gaudi::Property<std::vector<unsigned int>>* bins :
       {&m_itkDiscMaterialBins, &m_itkCylinderMaterialBins,
        &m_itkPixelCylinderMaterialBins, &m_itkPixelDiscMaterialBins,
        &m_itkStripCylinderMaterialBins, &m_itkStripDiscMaterialBins}) {
    if (!isValidMaterialBins(bins->value())) {
      ATH_MSG_FATAL((*bins) << " must hold two positive bin counts");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Material binning " << (*bins));
  }
  // the window sizes were checked above
  ATH_MSG_DEBUG("Strip barrel surface arrays: " << m_itkStripBarrelBinFactor.value()
               << " bins per module, neighbor window bound {phi, z} {"
               << m_itkStripInnerBarrelNeighborWindow.value()[0] << ", "
               << m_itkStripInnerBarrelNeighborWindow.value()[1] << "} in layers 0-1, {"
               << m_itkStripBarrelNeighborWindow.value()[0] << ", "
               << m_itkStripBarrelNeighborWindow.value()[1] << "} in layers 2-3");

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

Acts::SurfaceArrayNavigationPolicy::Config
ItkBlueprintNodeBuilder::surfaceArrayConfig(
    Acts::SurfaceArrayNavigationPolicy::LayerType layerType,
    const Gaudi::Property<std::vector<unsigned int>>& neighborWindow,
    double binFactor) const {
  return Acts::SurfaceArrayNavigationPolicy::Config{
      .layerType = layerType,
      .bins = {0, 0},
      .numberOfBinsFactor = binFactor,
      .neighborWindow = *toNeighborWindow(neighborWindow.value()),
      .overfill = static_cast<std::uint8_t>(m_surfaceArrayOverfill.value())};
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
        configureDiscMaterial(mat, NegativeDisc, m_itkDiscMaterialBins.value(),
                              "ItkNodeMain_Material_NegativeDisc");
        configureDiscMaterial(mat, PositiveDisc, m_itkDiscMaterialBins.value(),
                              "ItkNodeMain_Material_PositiveDisc");
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
  /// Wrap the ITk into a separate volume
  Acts::ExtentEnvelope envelope{};
  envelope[AxisDirection::AxisZ] = {20_mm, 20_mm};
  envelope[AxisDirection::AxisR] = {1_mm, 2_mm};
  auto padNode = std::make_shared<Acts::PadBlueprintNode>("ITkEnvelope", envelope);
  padNode->addChild(itkNode);
  /// Setup a fixed geometry Identifier
  auto idNode = std::make_shared<Acts::GeometryIdentifierBlueprintNode>();
  idNode->setDirectChildVolumeIdTo(s_ITkEnvelopeId);
  idNode->addChild(std::move(padNode));
  return idNode;
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
    configureCylinderMaterial(mat, OuterCylinder,
                              m_itkPixelCylinderMaterialBins.value(),
                              "InnerPixelMaterial");

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
          configureDiscMaterial(material, NegativeDisc,
                                m_itkPixelDiscMaterialBins.value(),
                                "InnerPixel_Material_NegativeDisc");
          configureDiscMaterial(material, PositiveDisc,
                                m_itkPixelDiscMaterialBins.value(),
                                "InnerPixel_Material_PositiveDisc");
        });
    auto& barrel = brl_mat.addCylinderContainer("InnerPixel_Brl", AxisR);

    // Barrel layers carry material only on their InnerCylinder face, in front
    // of the sensors. Attaching with First extends the inner (lower-R)
    // neighbour's material-free outer face outward to meet each layer's inner
    // face, collapsing the inter-layer gap volumes while leaving every material
    // surface fixed.
    barrel.setAttachmentStrategy(AttachmentStrategy::First);
    // Handling of the resize strategies is as follows:
    // Inner edge Gap - Outer edge Expand
    barrel.setResizeStrategies(ResizeStrategy::Gap, ResizeStrategy::Expand);

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

      const std::string materialName =
          std::format("InnerPixel_Brl_{}_Material", ilayer);
      barrel.addMaterial(materialName, [&](auto& lmat) {
        configureCylinderMaterial(lmat, InnerCylinder,
                                  m_itkPixelCylinderMaterialBins.value(),
                                  materialName);

        auto& layer = lmat.addLayer(std::format("InnerPixel_Brl_{}", ilayer));
        layer.setNavigationPolicyFactory(
            Acts::NavigationPolicyFactory{}
                .add<Acts::SurfaceArrayNavigationPolicy>(surfaceArrayConfig(
                    Cylinder, m_itkPixelInnerBarrelNeighborWindow))
                .add<Acts::CylinderNavigationPolicy>()
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
      auto& ec = ecGeoId.addCylinderContainer(std::format("InnerPixel_{:}EC", s), AxisZ);
      // Collapse the inter-disk gaps by extending the inner disk outward onto
      // its neighbour (First for +z, Second for -z), keeping the material
      // surface at the larger-|z| side of each gap (see per-disk material
      // handling below).
      ec.setAttachmentStrategy(bec > 0 ? AttachmentStrategy::First
                                       : AttachmentStrategy::Second);
      // Resize strategies for the endcap disks are as follows:
      // Barrel-facing edge Gap while far (z-end) edge Expand
      ec.setResizeStrategies(
          bec > 0 ? ResizeStrategy::Gap : ResizeStrategy::Expand,
          bec > 0 ? ResizeStrategy::Expand : ResizeStrategy::Gap);

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

        auto addLayer = [this, &layerName, &pl](auto& parent) {
          auto& layer = parent.addLayer(layerName);

          layer.setNavigationPolicyFactory(
              Acts::NavigationPolicyFactory{}
                  .add<Acts::SurfaceArrayNavigationPolicy>(surfaceArrayConfig(
                      Disc, m_itkPixelInnerEndcapNeighborWindow))
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
        // Keep material only on each disk's inward disc (facing the IP, in
        // front of the sensors); the outward disc is dropped so it can fuse
        // with the adjacent disk's kept inward disc without the "both portals
        // carry material" fuse error.
        const auto inwardDisc = (bec > 0) ? NegativeDisc : PositiveDisc;
        const std::string materialName = layerName + "_Material";
        ec.addMaterial(materialName, [&](auto& lmat) {
          configureDiscMaterial(lmat, inwardDisc,
                                m_itkPixelDiscMaterialBins.value(),
                                materialName);
          addLayer(lmat);
        });
      }
    }
  });

  // Outer pixel: 3 outer barrel layers + outer endcaps
  auto& outerPixel = node.addCylinderContainer("OuterPixel", AxisR);
  outerPixel.setAttachmentStrategy(AttachmentStrategy::Gap);
  // Outer pixel resize strategies are as follows:
  // Inner edge Expand - Outer edge stays Gap (the OuterPixel/Strip interface is
  // handled from the Strip side).
  outerPixel.setResizeStrategies(ResizeStrategy::Expand, ResizeStrategy::Gap);

  outerPixel.addMaterial("OuterPixelMaterial", [&](auto& opmat) {
    configureCylinderMaterial(opmat, OuterCylinder,
                              m_itkPixelCylinderMaterialBins.value(),
                              "OuterPixelMaterial");

    auto& outerPixelContainer = opmat.addCylinderContainer("OuterPixel", AxisZ);

    auto& barrelGeoId = outerPixelContainer.withGeometryIdentifier();
    barrelGeoId.setAllVolumeIdsTo(s_outerPixelVolumeId).incrementLayerIds(1);

    auto& brl_mat =
        barrelGeoId.addMaterial("OuterPixel_Material", [&](auto& bmat) {
          configureDiscMaterial(bmat, NegativeDisc,
                                m_itkPixelDiscMaterialBins.value(),
                                "OuterPixel_Material_NegativeDisc");
          configureDiscMaterial(bmat, PositiveDisc,
                                m_itkPixelDiscMaterialBins.value(),
                                "OuterPixel_Material_PositiveDisc");
        });
    auto& barrel = brl_mat.addCylinderContainer("OuterPixel_Brl", AxisR);

    // See InnerPixel_Brl: layers carry material only on InnerCylinder, so
    // First attachment removes the inter-layer gaps without moving any
    // material surface.
    barrel.setAttachmentStrategy(AttachmentStrategy::First);
    // Inner edge Gap - Outer edge Expand
    barrel.setResizeStrategies(ResizeStrategy::Gap, ResizeStrategy::Expand);

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

      const std::string materialName =
          std::format("OuterPixel_Brl_{}_Material", ilayer);
      barrel.addMaterial(materialName, [&](auto& lmat) {
        configureCylinderMaterial(lmat, InnerCylinder,
                                  m_itkPixelCylinderMaterialBins.value(),
                                  materialName);

        auto& layer = lmat.addLayer(std::format("OuterPixel_Brl_{:}", ilayer));
        layer.setNavigationPolicyFactory(
            Acts::NavigationPolicyFactory{}
                .add<Acts::SurfaceArrayNavigationPolicy>(surfaceArrayConfig(
                    Cylinder, m_itkPixelOuterBarrelNeighborWindow))
                .add<Acts::CylinderNavigationPolicy>()
                .asUniquePtr());
        layer.setSurfaces(surfaces);
        layer.setEnvelope(Acts::ExtentEnvelope{{
            .z = {5_mm, 5_mm},
            .r = {2_mm, 2_mm},
        }});
      });
    }

    const auto endcapSurfaceArrayConfig =
        surfaceArrayConfig(Disc, m_itkPixelOuterEndcapNeighborWindow);
    auto addEndcapLayer = [&endcapSurfaceArrayConfig](
                              auto& parent, const auto& name,
                              const auto& surfaces) {
      parent.addLayer(name, [&surfaces, &endcapSurfaceArrayConfig](auto& layer) {
        layer.setNavigationPolicyFactory(
            Acts::NavigationPolicyFactory{}
                .add<Acts::SurfaceArrayNavigationPolicy>(endcapSurfaceArrayConfig)
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
          ec_outer_geoId.addCylinderContainer(std::format("OuterPixel_{:}EC", s), AxisR);

      // Three groups of disks stacked in R
      std::array diskGroups{std::pair{3, 4}, std::pair{6, 5}, std::pair{7, 8}};

      for (size_t idx = 0; idx < diskGroups.size(); ++idx) {
        auto [disk1, disk2] = diskGroups[idx];

        Acts::MaterialDesignatorBlueprintNode* material = nullptr;
        if (idx < (diskGroups.size() - 1)) {
          const std::string materialName =
              "OuterPixel_" + s + "EC_" + std::to_string(idx) + "_Material";
          material = &ec_outer.addMaterial(
              materialName,
              [&](auto& mat) {
                configureCylinderMaterial(
                    mat, OuterCylinder,
                    m_itkPixelCylinderMaterialBins.value(), materialName);
              });
        }

        auto& ec_stack =
            material
                ? material->addCylinderContainer(
                     std::format("OuterPixel_{:}EC_{:}", s, idx), AxisZ)
                : ec_outer.addCylinderContainer(
                     std::format("OuterPixel_{:}EC_{:}", s, idx), AxisZ);

        // Collapse inter-disk gaps by extending the inner disk outward (First
        // for +z, Second for -z); material kept only on inward discs (see
        // below), so surviving material sits at the larger-|z| side of each
        // gap.
        ec_stack.setAttachmentStrategy(bec > 0 ? AttachmentStrategy::First
                                               : AttachmentStrategy::Second);
        // Barrel-facing edge Gap: the innermost disk of each R-group keeps its
        // inward disc material at the disk; far (z-end) edge Expand: the
        // outermost disk's outward disc is material-free, so the disk extends
        // to the end of the container.
        ec_stack.setResizeStrategies(
            bec > 0 ? ResizeStrategy::Gap : ResizeStrategy::Expand,
            bec > 0 ? ResizeStrategy::Expand : ResizeStrategy::Gap);

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
          Acts::ProtoLayer pl_a(gctx, Acts::unpackConstSmartPointers(a));
          Acts::ProtoLayer pl_b(gctx, Acts::unpackConstSmartPointers(b));
          return std::abs(pl_a.min(AxisZ)) < std::abs(pl_b.min(AxisZ));
        });

        for (size_t i = 0; i < sorted_rings.size(); ++i) {
          const auto& surfaces = sorted_rings[i];
          const std::string layerName = std::format("OuterPixel_{:}EC_{:}_{:}", s,idx, i);

          const auto inwardDisc = (bec > 0) ? NegativeDisc : PositiveDisc;
          const std::string materialName = layerName + "_Material";
          ec_stack.addMaterial(materialName, [&](auto& mat) {
            configureDiscMaterial(mat, inwardDisc,
                                  m_itkPixelDiscMaterialBins.value(),
                                  materialName);
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
  // Inner edge Expand - Outer edge stays Gap
  strip.setResizeStrategies(ResizeStrategy::Expand, ResizeStrategy::Gap);

  strip.addMaterial("StripMaterial", [&](auto& mat) {
    configureCylinderMaterial(mat, OuterCylinder,
                              m_itkCylinderMaterialBins.value(),
                              "StripMaterial");

    auto& stripContainer = mat.addCylinderContainer("Strip", AxisZ);

    // Add barrel container
    stripContainer.withGeometryIdentifier([this, &elements](auto& geoId) {
      geoId.setAllVolumeIdsTo(s_stripVolumeId).incrementLayerIds(1);

      auto& brl_mat =
          geoId.addMaterial("Strip_Brl_Material", [&](auto& material) {
            configureDiscMaterial(material, NegativeDisc,
                                  m_itkStripDiscMaterialBins.value(),
                                  "Strip_Brl_Material_NegativeDisc");
            configureDiscMaterial(material, PositiveDisc,
                                  m_itkStripDiscMaterialBins.value(),
                                  "Strip_Brl_Material_PositiveDisc");
          });
      brl_mat.addCylinderContainer(
          "Strip_Brl", AxisR, [this, &elements](auto& barrel) {
            // For the strip barrel and endcap we set the attachment strategy
            // and resize strategies to Gap. This allows to have the sensitive
            // layers to be wrapped between navigation layers.
            barrel.setAttachmentStrategy(AttachmentStrategy::Gap);
            barrel.setResizeStrategies(ResizeStrategy::Gap,
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
              // layers 0 and 1: short strips
              addStripBarrelLayer(
                  barrel, ilayer, surfaces,
                  surfaceArrayConfig(Cylinder,
                                     ilayer < 2 ? m_itkStripInnerBarrelNeighborWindow
                                                : m_itkStripBarrelNeighborWindow,
                                     m_itkStripBarrelBinFactor),
                  m_itkStripCylinderMaterialBins.value());
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
                  Acts::ProtoLayer pl_a(gctx, Acts::unpackConstSmartPointers(a));
                  Acts::ProtoLayer pl_b(gctx, Acts::unpackConstSmartPointers(b));
                  return std::abs(pl_a.min(AxisZ)) < std::abs(pl_b.min(AxisZ));
                });

      const auto endcapSurfaceArrayConfig =
          surfaceArrayConfig(Disc, m_itkStripEndcapNeighborWindow);
      const auto& endcapMaterialBins = m_itkStripDiscMaterialBins.value();
      stripContainer.withGeometryIdentifier([&sorted_layers, bec, &s,
                                             &endcapSurfaceArrayConfig,
                                             &endcapMaterialBins](auto& geoId) {
        geoId.setAllVolumeIdsTo(s_stripVolumeId + std::floor(bec / 2))
            .incrementLayerIds(1);

        geoId.addCylinderContainer(
            std::format("Strip_{:}_EC", s), AxisZ,
            [&sorted_layers, &s, bec, &endcapSurfaceArrayConfig,
             &endcapMaterialBins](auto& ec) {
              // For the strip barrel and endcap we set the attachment strategy
              // and resize strategies to Gap. This allows to have the sensitive
              // layers to be wrapped between navigation layers.
              ec.setAttachmentStrategy(AttachmentStrategy::Gap);
              ec.setResizeStrategies(ResizeStrategy::Gap, ResizeStrategy::Gap);

              for (size_t i = 0; i < sorted_layers.size(); ++i) {
                const auto& surfaces = sorted_layers[i];
                auto layerName = std::format("Strip_{:}EC_{:}", s, i);
                addStripEndcapLayer(ec, layerName, surfaces, bec,
                                    endcapSurfaceArrayConfig, endcapMaterialBins);
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
  Acts::ProtoLayer stripExtent(gctx, Acts::unpackConstSmartPointers(allStripSurfaces));

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
