/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// This absolutely needs to go first to ensure Eigen plugin is loaded
#include "GeoPrimitives/GeoPrimitives.h"
//

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include <GeoModelKernel/GeoTube.h>
#include <GeoModelKernel/GeoVPhysVol.h>


#include "ItkBlueprintNodeBuilder.h"

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
#include "Acts/Geometry/VolumeResizeStrategy.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include <Acts/Navigation/SurfaceArrayNavigationPolicy.hpp>
#include <Acts/Surfaces/SurfaceArray.hpp>
#include <Acts/Navigation/TryAllNavigationPolicy.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>


#include <format>
#include <fstream>
#include <ranges>

#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ActsElementVector.h"
#include "ActsInterop/IdentityHelper.h"
#include "ActsInterop/Logger.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "BeamPipeGeoModel/BeamPipeDetectorManager.h"



using namespace Acts::UnitLiterals;


namespace{

constexpr std::size_t s_stripVolumeId = 20;
constexpr std::size_t s_innerPixelVolumeId = 5;
constexpr std::size_t s_outerPixelVolumeId = 10;
constexpr std::size_t s_beamPipeVolumeId = 1;

using enum Acts::CylinderVolumeBounds::Face;
using enum Acts::AxisDirection;
using enum Acts::AxisBoundaryType;
using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
using AttachmentStrategy = Acts::VolumeAttachmentStrategy;
using ResizeStrategy = Acts::VolumeResizeStrategy;

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

  if (ilayer < 3) {
    // Inner 3 layers: add material on outer cylinder
    parent.addMaterial("Strip_Brl_" + std::to_string(ilayer) + "_Material",
                       [&addLayer](auto& lmat) {
                         lmat.configureFace(OuterCylinder,
                                            {AxisRPhi, Bound, 20},
                                            {AxisZ, Bound, 20});
                         addLayer(lmat);
                       });
  } else {
    addLayer(parent);
  }
}

void addStripEndcapLayer(
    Acts::Experimental::BlueprintNode& parent, int bec, const std::string& name,
    const std::vector<std::shared_ptr<Acts::Surface>>& surfaces) {
  using enum Acts::SurfaceArrayNavigationPolicy::LayerType;
  using enum Acts::CylinderVolumeBounds::Face;
  using enum Acts::AxisDirection;
  using enum Acts::AxisBoundaryType;
 
  parent.addMaterial(name + "_Material", [&](auto& mat) {
    mat.configureFace(bec < 0 ? NegativeDisc : PositiveDisc, {AxisR, Bound, 20},
                      {AxisPhi, Bound, 40});

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

namespace ActsTrk{

StatusCode ItkBlueprintNodeBuilder::initialize(){
    ATH_CHECK(detStore()->retrieve(m_itkStripMgr, "ITkStrip"));
    ATH_CHECK(detStore()->retrieve(m_itkPixelMgr, "ITkPixel"));
    if(m_buildBeamPipe) {
        ATH_CHECK(detStore()->retrieve(m_beamPipeMgr, "BeamPipe"));
    } 
    m_elementStore = std::make_shared<ActsElementVector>();
    return StatusCode::SUCCESS;

}

std::shared_ptr<Acts::Experimental::BlueprintNode> ItkBlueprintNodeBuilder::buildBlueprintNode(
    const Acts::GeometryContext& gctx, std::shared_ptr<Acts::Experimental::BlueprintNode>&& childNode) {

    if(childNode) {
      ATH_MSG_ERROR("Child node for the Itk should be null - no child expected");
      throw std::runtime_error("Child node is not null");
    }
    
    // The itk node is a container node that will hold both the pixel and strip nodes  
    auto itkNode = std::make_shared<Acts::Experimental::CylinderContainerBlueprintNode>("ItkNode", AxisR);

    // Add the itk pixel to the node
    buildItkPixelBlueprintNode(gctx, *itkNode);

    // Add the itk strip to the node
    buildItkStripBlueprintNode(gctx, *itkNode);

    //Add the beam pipe to the node
    if(m_buildBeamPipe) {

    buildBeamPipeBlueprintNode(gctx, *itkNode);

    }

    return itkNode;

}

void ItkBlueprintNodeBuilder::buildItkPixelBlueprintNode(
    const Acts::GeometryContext& gctx, Acts::Experimental::BlueprintNode& node) {

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
  for (const auto* element :
       *m_itkPixelMgr->getDetectorElementCollection()) {
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

  // Create containers for inner and outer pixel parts
  node.addMaterial("InnerPixelMaterial", [&](auto& mat) {
    mat.configureFace(OuterCylinder, {AxisRPhi, Bound, 20}, {AxisZ, Bound, 20});

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

    auto& barrel = barrelGeoId.addCylinderContainer("InnerPixel_Brl", AxisR);
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
            // Innermost layer: outer cylinder, else inner cylinder
            lmat.configureFace(OuterCylinder, {AxisRPhi, Bound, 40},
                               {AxisZ, Bound, 20});

            // Add layer with surfaces
            auto& layer =
                lmat.addLayer(std::format("InnerPixel_Brl_{}", ilayer));

            // Set navigation policy for efficient surface lookup
            layer.setNavigationPolicyFactory(
                Acts::NavigationPolicyFactory{}
                    .add<Acts::SurfaceArrayNavigationPolicy>(
                        Acts::SurfaceArrayNavigationPolicy::Config{
                            .layerType = Cylinder,
                            .bins = {30, 10}})  // @TODO: Improve this logic
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
    for (int bec : {-2, 2}) {  // Negative and positive endcaps
      std::string s = bec > 0 ? "p" : "n";
      auto& ecGeoId = innerPixelContainer.withGeometryIdentifier();
      ecGeoId.setAllVolumeIdsTo(s_innerPixelVolumeId + std::floor(bec / 2))
          .incrementLayerIds(1);
      auto& ec = ecGeoId.addCylinderContainer("InnerPixel_" + s + "EC", AxisZ);
      ec.setAttachmentStrategy(AttachmentStrategy::Gap);
      ec.setResizeStrategy(ResizeStrategy::Expand);

      std::map<std::tuple<int, int, int>,
               std::vector<std::shared_ptr<Acts::Surface>>>
          initialLayers{};

      for (auto& element : elements) {
        IdentityHelper id = element->identityHelper();
        if (id.bec() * bec <= 0) {
          continue;  // Skip if not in the right endcap
        }

        if (id.layer_disk() >= 3) {
          continue;  // Only include first 3 disks for inner pixel
        }

        std::tuple<int, int, int> key{id.bec(), id.layer_disk(),
                                      id.eta_module()};
        initialLayers[key].push_back(element->surface().getSharedPtr());
      }

      ATH_MSG_DEBUG("Found " << initialLayers.size()
                             << " initial layers to InnerPixel " << s << "EC");

      // Create proto layers from surfaces
      std::vector<LayerData> protoLayers;
      protoLayers.reserve(initialLayers.size());

      for (const auto& [key, surfaces] : initialLayers) {
        auto& layer = protoLayers.emplace_back(gctx, surfaces);
        layer.protoLayer.envelope[AxisR] = {2_mm, 2_mm};
        layer.protoLayer.envelope[AxisZ] = {1_mm, 1_mm};
      }

      // Sort by z position
      std::ranges::sort(protoLayers,
                        [](const LayerData& a, const LayerData& b) {
                          return std::abs(a.protoLayer.medium(AxisZ)) <
                                 std::abs(b.protoLayer.medium(AxisZ));
                        });

      ATH_MSG_DEBUG("Found " << protoLayers.size() << " initial layers");

      // Merge overlapping layers
      std::vector<LayerData> mergedLayers;
      if (m_doEndcapLayerMerging) {
        mergedLayers = mergeLayers(gctx, std::move(protoLayers));
      } else {
        mergedLayers = std::move(protoLayers);
      }

      ATH_MSG_DEBUG("After merging: " << mergedLayers.size() << " layers");

      // Create layers from merged proto layers
      for (const auto [key, pl] : Acts::enumerate(mergedLayers)) {
        ATH_MSG_DEBUG("- Layer " << key << " has " << pl.surfaces.size()
                                 << " surfaces");

        pl.protoLayer.medium(AxisZ);
        auto layerName = std::format("InnerPixel_{}EC_{}", key, s);

        auto addLayer = [&layerName, &pl](auto& parent) {
          // Add layer with surfaces
          auto& layer = parent.addLayer(layerName);

          // Set navigation policy for efficient surface lookup
          layer.setNavigationPolicyFactory(
              Acts::NavigationPolicyFactory{}
                  .add<Acts::SurfaceArrayNavigationPolicy>(
                      Acts::SurfaceArrayNavigationPolicy::Config{
                          .layerType = Disc,
                          .bins = {30, 30}})  // @TODO: Improve this logic
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
        if (key < mergedLayers.size() - 1) {
          ATH_MSG_VERBOSE("Adding material for layer "
                          << layerName );
          ec.addMaterial(layerName + "_Material", [&](auto& lmat) {
            // Set binning for endcap layer
            lmat.configureFace(bec < 0 ? NegativeDisc : PositiveDisc,
                               {AxisR, Bound, 40}, {AxisPhi, Bound, 40});
            addLayer(lmat);
          });
        } else {
          addLayer(ec);
        }
      }
    }
  });

  // Add outer pixel part
  node.addMaterial("OuterPixelMaterial", [&](auto& mat) {
    mat.configureFace(OuterCylinder, {AxisRPhi, Bound, 20}, {AxisZ, Bound, 20});

    auto& outerPixelContainer = mat.addCylinderContainer("OuterPixel", AxisZ);

    // Add barrel container
    auto& barrelGeoId = outerPixelContainer.withGeometryIdentifier();
    barrelGeoId.setAllVolumeIdsTo(s_outerPixelVolumeId).incrementLayerIds(1);

    auto& barrel = barrelGeoId.addCylinderContainer("OuterPixel_Brl", AxisR);
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
            // Innermost layer: outer cylinder, else inner cylinder
            lmat.configureFace(OuterCylinder, {AxisRPhi, Bound, 40},
                               {AxisZ, Bound, 20});

            // Add layer with surfaces
            auto& layer =
                lmat.addLayer("OuterPixel_Brl_" + std::to_string(ilayer));

            // Set navigation policy for efficient surface lookup
            layer.setNavigationPolicyFactory(
                Acts::NavigationPolicyFactory{}
                    .add<Acts::SurfaceArrayNavigationPolicy>(
                        Acts::SurfaceArrayNavigationPolicy::Config{
                            .layerType = Cylinder,
                            .bins = {30, 10}})  // @TODO: Improve this logic
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

    // Add outer pixel endcaps
    for (int bec : {-2, 2}) {  // Negative and positive endcaps
      const std::string s = bec > 0 ? "p" : "n";

      // R stacked Z rings

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

        auto& ec_stack = ec_outer.addCylinderContainer(
            "OuterPixel_" + s + "EC_" + std::to_string(idx), AxisZ);

        ec_stack.setAttachmentStrategy(AttachmentStrategy::Gap);
        ec_stack.setResizeStrategy(ResizeStrategy::Expand);

        // Group sensors by eta rings
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

        // Sort rings by absolute z position
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

        // Create layers from sorted rings
        for (size_t i = 0; i < sorted_rings.size(); ++i) {
          const auto& surfaces = sorted_rings[i];
          auto layerName = "OuterPixel_" + s + "EC_" + std::to_string(idx) +
                           "_" + std::to_string(i);

          if (i < sorted_rings.size() - 1) {
            ec_stack.addMaterial(layerName + "_Material", [&](auto& mat) {
              mat.configureFace(bec > 0 ? PositiveDisc : NegativeDisc,
                                {AxisR, Bound, 20}, {AxisPhi, Bound, 40});
              addEndcapLayer(mat, layerName, surfaces);
            });
          } else {
            addEndcapLayer(ec_stack, layerName, surfaces);
          }
        }
      }
    }
  });

}

void ItkBlueprintNodeBuilder::buildItkStripBlueprintNode(const Acts::GeometryContext& gctx, 
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
        for (const auto* element :
            *m_itkStripMgr->getDetectorElementCollection()) {
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

        // Create container for strip part
        node.addMaterial("StripMaterial", [&](auto& mat) {
            mat.configureFace(OuterCylinder, {AxisRPhi, Bound, 20}, {AxisZ, Bound, 20});

            mat.addCylinderContainer("Strip", AxisZ, [&](auto& strips) {
            // Add barrel container
            strips.withGeometryIdentifier([this, &elements](auto& geoId) {
                geoId.setAllVolumeIdsTo(s_stripVolumeId).incrementLayerIds(1);

                geoId.addCylinderContainer(
                    "Strip_Brl", AxisR, [this, &elements](auto& barrel) {
                    barrel.setAttachmentStrategy(AttachmentStrategy::Gap);
                    barrel.setResizeStrategy(ResizeStrategy::Gap);

                    std::map<int, std::vector<std::shared_ptr<Acts::Surface>>>
                        layers{};

                    for (auto& element : elements) {
                        IdentityHelper id = element->identityHelper();
                        if (id.bec() != 0) {
                        continue;
                        }

                        int elementLayer = id.layer_disk();
                        layers[elementLayer].push_back(
                            element->surface().getSharedPtr());
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
            for (int bec : {-2, 2}) {  // Negative and positive endcaps
                const std::string s = bec > 0 ? "p" : "n";

                std::map<int, std::vector<std::shared_ptr<Acts::Surface>>> layers{};

                for (auto& element : elements) {
                IdentityHelper id = element->identityHelper();
                if (id.bec() * bec <= 0) {
                    continue;  // Skip if not in the right endcap
                }

                layers[id.layer_disk()].push_back(element->surface().getSharedPtr());
                }

                ATH_MSG_DEBUG("Found " << layers.size() << " layers in Strip " << s
                                    << "EC");

                // Sort layers by absolute z position
                std::vector<std::vector<std::shared_ptr<Acts::Surface>>> sorted_layers;
                sorted_layers.reserve(layers.size());
                for (const auto& [key, surfaces] : layers) {
                sorted_layers.push_back(surfaces);
                }

                std::sort(sorted_layers.begin(), sorted_layers.end(),
                        [&gctx](const auto& a, const auto& b) {
                            Acts::ProtoLayer pl_a(gctx, makeConstPtrVector(a));
                            Acts::ProtoLayer pl_b(gctx, makeConstPtrVector(b));
                            return std::abs(pl_a.min(AxisZ)) <
                                std::abs(pl_b.min(AxisZ));
                        });

                strips.withGeometryIdentifier([&sorted_layers, bec, &s](auto& geoId) {
                    geoId.setAllVolumeIdsTo(s_stripVolumeId + std::floor(bec / 2))
                        .incrementLayerIds(1);

                    geoId.addCylinderContainer(
                        "Strip_" + s + "EC", AxisZ, [&sorted_layers, bec, &s](auto& ec) {
                            ec.setAttachmentStrategy(AttachmentStrategy::Gap);
                            ec.setResizeStrategy(ResizeStrategy::Gap);

                            // Create layers from sorted layers
                            for (size_t i = 0; i < sorted_layers.size(); ++i) {
                            const auto& surfaces = sorted_layers[i];
                            auto layerName = "Strip_" + s + "EC_" + std::to_string(i);

                            addStripEndcapLayer(ec, bec, layerName, surfaces);
                            }
                    });
                });
            }

            });
        });
    }

  void ItkBlueprintNodeBuilder::buildBeamPipeBlueprintNode(const Acts::GeometryContext& /*gctx*/,
                                                           Acts::Experimental::BlueprintNode& node) {


  // Get beam pipe parameters from existing code
  PVConstLink beamPipeTopVolume = m_beamPipeMgr->getTreeTop(0);
  if (m_beamPipeMgr->getNumTreeTops() == 1) {
    beamPipeTopVolume =
        m_beamPipeMgr->getTreeTop(0)->getChildVol(0)->getChildVol(0);
  }

  const Amg::Transform3D beamPipeTransform{Amg::getTranslate3D(beamPipeTopVolume->getX().translation())};

  // Extract radius similar to makeBeamPipeConfig
  double beamPipeRadius = 20;  // Default value

  const GeoLogVol* beamPipeLogVolume = beamPipeTopVolume->getLogVol();
  // This should always be set, but let's be safe
  if (beamPipeLogVolume == nullptr) {
    ATH_MSG_ERROR("Beam pipe volume has no log volume");
    throw std::runtime_error("Beam pipe volume has no log volume");
  }

  // Get the geoShape and translate
  const GeoTube* beamPipeTube =
      dynamic_cast<const GeoTube*>(beamPipeLogVolume->getShape());
  if (beamPipeTube == nullptr) {
    ATH_MSG_ERROR("BeamPipeLogVolume was not of type GeoTube");
    throw std::runtime_error{"BeamPipeLogVolume was not of type GeoTube"};
  }

  // Look for SectionC03 to get the actual radius
  for (unsigned int i = 0; i < beamPipeTopVolume->getNChildVols(); i++) {
    auto childName = beamPipeTopVolume->getNameOfChildVol(i);
    if (childName != "SectionC03") {
      continue;  // Skip if not SectionC03
    }
    PVConstLink childTopVolume = beamPipeTopVolume->getChildVol(i);
    const GeoLogVol* childLogVolume = childTopVolume->getLogVol();
    const GeoTube* childTube =
            dynamic_cast<const GeoTube*>(childLogVolume->getShape());
    if (childTube) {
        beamPipeRadius = 0.5 * (childTube->getRMax() + childTube->getRMin());
        break;
    }

  }
  

  ATH_MSG_VERBOSE(
      "BeamPipe constructed from Database: translation (yes) - radius "
      << (beamPipeTube ? "(yes)" : "(no)") << " - r = " << beamPipeRadius);

  ATH_MSG_VERBOSE("BeamPipe shift estimated as    : "
                  << beamPipeTransform.translation().transpose());

  // Add to blueprint following pattern from blueprint_itk.py
  node.withGeometryIdentifier([&](auto& geoId) {
    geoId.setAllVolumeIdsTo(s_beamPipeVolumeId);

    geoId.addMaterial("BeamPipe_Material", [&](auto& mat) {
      mat.configureFace(OuterCylinder, {AxisRPhi, Bound, 20},
                        {AxisZ, Bound, 20});

      // Add static volume for beam pipe
      mat.addStaticVolume(beamPipeTransform,
                          std::make_shared<Acts::CylinderVolumeBounds>(
                              0, beamPipeRadius * 1_mm, 3 * 1_m),
                          "BeamPipe");
    });
  });
}

} //namespace ActsTrk
