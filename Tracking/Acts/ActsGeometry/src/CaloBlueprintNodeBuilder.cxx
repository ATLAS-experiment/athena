/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloBlueprintNodeBuilder.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

#include "Acts/Geometry/Blueprint.hpp"
#include "Acts/Geometry/StaticBlueprintNode.hpp"

#include "Acts/Surfaces/RadialBounds.hpp"
#include "Acts/Surfaces/CylinderBounds.hpp"
#include "Acts/Geometry/ContainerBlueprintNode.hpp"
#include "Acts/Geometry/Extent.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/LayerBlueprintNode.hpp"
#include "Acts/Geometry/VolumeResizeStrategy.hpp"
#include "Acts/Geometry/VolumeAttachmentStrategy.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/PadBlueprintNode.hpp"
#include "Acts/Geometry/GeometryIdentifierBlueprintNode.hpp"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/AxisDefinitions.hpp"

#include "CaloDetDescrUtils/CaloDetDescrBuilder.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloGeoHelpers/CaloSampling.h"


#include <cmath>

using namespace Acts::UnitLiterals;
using namespace ActsTrk::detail::GeoVolIds;

using SampleSet_t = std::unordered_set<CaloCell_ID::CaloSample>;

namespace {
  //create lists from all possible calo samplings that tracks could hit
  //The first list is for disc shaped samples and the second for cylindrical shaped samples
  //Note that TileGap3 is the barrel, but is disk shaped. 

  static const SampleSet_t s_caloDiscSampleList{
          CaloCell_ID::PreSamplerE,
          CaloCell_ID::EME1, CaloCell_ID::EME2, CaloCell_ID::EME3,
          CaloCell_ID::HEC0, CaloCell_ID::HEC1, CaloCell_ID::HEC2, CaloCell_ID::HEC3, 
          CaloCell_ID::TileGap3,
          CaloCell_ID::FCAL0, CaloCell_ID::FCAL1, CaloCell_ID::FCAL2};

  static const SampleSet_t s_caloCylinderSymmetricSampleList{ 
          CaloCell_ID::PreSamplerB, 
          CaloCell_ID::EMB1, CaloCell_ID::EMB2, CaloCell_ID::EMB3,
          CaloCell_ID::TileBar0, CaloCell_ID::TileBar1, CaloCell_ID::TileBar2};

  static const SampleSet_t s_caloCylinderAsymmetricSampleList{ 
          CaloCell_ID::TileGap1, CaloCell_ID::TileGap2, 
          CaloCell_ID::TileExt0, CaloCell_ID::TileExt1, CaloCell_ID::TileExt2};
    
    std::string print(std::span<const CaloDetDescrElement* const> elements) {
        std::stringstream sstr{};
        unsigned counter{};
        for (const CaloDetDescrElement* theDDE : elements) {
            sstr<<"    - "<<(counter++)<<") "
                <<CaloSampling::getSamplingName(theDDE->getSampling())
                <<", radius: ["<< (theDDE->r() - 0.5*theDDE->dr())<<";"
                <<(theDDE->r() + 0.5*theDDE->dr())
                <<"], z: ["<<(theDDE->z() -0.5* theDDE->dz())<<";"
                <<(theDDE->z() +0.5* theDDE->dz())<<"]"<<std::endl;
        }
        return sstr.str();
    }
}

namespace ActsTrk{
using DetElementMap_t = CaloBlueprintNodeBuilder::DetElementMap_t;
using SurfaceMap_t = CaloBlueprintNodeBuilder::SurfaceMap_t;

StatusCode CaloBlueprintNodeBuilder::initialize() {
    if (m_surfaceDuplicates.value().size() != CaloSampling::getNumberOfSamplings()) {
        ATH_MSG_ERROR("Invalid size of "<<m_surfaceDuplicates<<". Expected: "
            <<CaloSampling::getNumberOfSamplings()<<" got "<<m_surfaceDuplicates.value().size());
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

DetElementMap_t CaloBlueprintNodeBuilder::fillDetectorElements(const CaloDetDescrManager& detDecrMgr) const {
    DetElementMap_t sortedElements{};
    for (const CaloDetDescrElement* theDDE : detDecrMgr.element_range()){ 
        const CaloCell_ID::CaloSample sample =theDDE->getSampling();
        sortedElements[Acts::toUnderlying(sample)].push_back(theDDE);
    }
    for (DetElVec_t& detElVec : sortedElements) {
       if (detElVec.empty()) {
           continue;
       }
       const CaloCell_ID::CaloSample sample = detElVec.front()->getSampling();
       std::ranges::sort(detElVec, [&](const CaloDetDescrElement* a, const CaloDetDescrElement* b){
            /** Barrel elements that are cylinders sort them by R */
            if (!s_caloDiscSampleList.count(sample)) {
                if (std::abs(a->r() - b->r()) > Acts::s_epsilon) {
                    return a->r() < b->r();
                }
                return a->z() < b->z();
            }
            /** Endcap elements are discs and sort them first by Z */
            if (std::abs(a->z() - b->z()) > Acts::s_epsilon) {
              return a->z() < b->z();
            }
            return a->r() < b->r();
      });
      /** Remove duplicate elements */
      auto [begin, end] = std::ranges::unique(detElVec, 
                    [&](const CaloDetDescrElement* a,  const CaloDetDescrElement* b) {
                        return std::abs(a->r() - b->r()) < Acts::s_onSurfaceTolerance &&
                               std::abs(a->z() - b->z()) < Acts::s_onSurfaceTolerance;
                    });
      const std::size_t nBefore = detElVec.size();
      detElVec.erase(begin, end);
      ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Removed "<<(nBefore - detElVec.size())
          <<" detector elements for layer "<<CaloSampling::getSamplingName(sample)
          <<", current size: "<<detElVec.size()<<"\n"<<print(detElVec));
    }
    /** The detector elements from TileBar2 and TileGap2 produce each a cylinder
     *  which is exactly at the same R but overlaping with the other. Merge them
     *  into a single vector
     */
    DetElVec_t& gapVec = sortedElements[CaloSampling::TileGap2];
    DetElVec_t& barVec = sortedElements[CaloSampling::TileBar2];
    barVec.insert(barVec.end(), gapVec.begin(), gapVec.end());
    gapVec.clear();
    return sortedElements;
}

std::shared_ptr<Acts::Surface> CaloBlueprintNodeBuilder::createCylinderSurface(std::span<const CaloDetDescrElement* const> detElVec) const {
          
  float zMin{std::numeric_limits<float>::max()}, 
        zMax{-std::numeric_limits<float>::max()},
        rMin{std::numeric_limits<float>::max()}, rMax{0.f};
  for (const CaloDetDescrElement* detEl : detElVec){
      zMin = std::min(zMin, detEl->z() - 0.5f*detEl->dz());
      zMax = std::max(zMax, detEl->z() + 0.5f*detEl->dz());
      rMin = std::min(rMin, detEl->r() - 0.5f*detEl->dr());
      rMax = std::max(rMax, detEl->r() + 0.5f*detEl->dr());
  }
  const float cylinderR = 0.5*(rMin + rMax);
  const float cylinderZ = 0.5*(zMax - zMin);
  const float surfaceZ =  0.5*(zMin + zMax);
  auto cylinder = Acts::Surface::makeShared<Acts::CylinderSurface>(Amg::getTranslateZ3D(surfaceZ), cylinderR, cylinderZ);
  cylinder->assignThickness(0.5*(rMax - rMin));
  cylinder->assignGeometryId(Acts::GeometryIdentifier{}.withLayer(detElVec.front()->getSampling()));
  ATH_MSG_DEBUG(__func__<<"() - Create surface "<<CaloSampling::getSamplingName(detElVec.front()->getSampling())
                  <<" r["<<rMin<<";"<<rMax<<"] z["<<zMin<<";"<<zMax<<"]"
                  <<" -> surface parameters R: "<<cylinder->bounds()<<", Z position: "<<surfaceZ
                  <<", "<<cylinder->geometryId());
  return cylinder;  
}

std::shared_ptr<Acts::Surface> CaloBlueprintNodeBuilder::createDiscSurface(std::span<const CaloDetDescrElement* const> detElVec) const {
    float zMin{std::numeric_limits<float>::max()}, 
          zMax{-std::numeric_limits<float>::max()},
          rMin{std::numeric_limits<float>::max()}, rMax{0.f};
    for (const CaloDetDescrElement* detEl : detElVec){
      zMin = std::min(zMin, detEl->z() - 0.5f*detEl->dz());
      zMax = std::max(zMax, detEl->z() + 0.5f*detEl->dz());
      rMin = std::min(rMin, detEl->r() - 0.5f*detEl->dr());
      rMax = std::max(rMax, detEl->r() + 0.5f*detEl->dr());
    }
    const float surfaceZ = 0.5*(zMin + zMax);
    const auto sampling = detElVec.front()->getSampling();
    ATH_MSG_DEBUG(__func__<<"() - Create surface "<<CaloSampling::getSamplingName(sampling)
                  <<" r["<<rMin<<";"<<rMax<<"] z["<<zMin<<";"<<zMax<<"]"
                  <<" -> surface position: "<<surfaceZ);    
    auto disc = Acts::Surface::makeShared<Acts::DiscSurface>(Amg::getTranslateZ3D(surfaceZ), rMin, rMax);
    disc->assignThickness(0.5*(zMax - zMin));
    disc->assignGeometryId(Acts::GeometryIdentifier{}.withLayer(sampling));
    return disc;
}

SurfaceMap_t CaloBlueprintNodeBuilder::translateToSurfaces(const DetElementMap_t& sortedElements) const {
    SurfaceMap_t surfaces{};
    for (const DetElVec_t& detElVec : sortedElements) {
        if (detElVec.empty()) {
            continue;
        }
        const CaloCell_ID::CaloSample sample = detElVec.front()->getSampling();
        using enum caloRegion;
        if (s_caloCylinderSymmetricSampleList.count(sample)) {
            surfaces[Acts::toUnderlying(BarrelCylinder)].push_back(createCylinderSurface(detElVec));
        } else if (s_caloCylinderAsymmetricSampleList.count(sample)) {
            for (DetElVec_t::const_iterator itr = detElVec.begin(); 
                 itr != detElVec.end(); ) {
                const CaloDetDescrElement* detEl{*itr};
                auto next = std::ranges::find_if(itr, detElVec.end(), [&](const CaloDetDescrElement* testMe){                 
                  return detEl->z() *  testMe->z() < 0.;
                });
                surfaces[Acts::toUnderlying(BarrelCylinder)].push_back(createCylinderSurface({itr, next}));
                itr = next;
            } 
        } else {
           for (DetElVec_t::const_iterator itr = detElVec.begin(); 
                 itr != detElVec.end(); ) {
                const CaloDetDescrElement* detEl{*itr};
                auto next = std::ranges::find_if(itr, detElVec.end(), [&](const CaloDetDescrElement* testMe){                 
                  return detEl->z() * testMe->z() < 0.;
                });
                const auto idx = detEl->z() > 0 ? Acts::toUnderlying(DiscPositiveZ)
                                                : Acts::toUnderlying(DiscNegativeZ);
                surfaces[idx].push_back(createDiscSurface({itr, next}));
                itr = next;
            }  
        }
    }
    return surfaces;
}
std::shared_ptr<Acts::BlueprintNode> 
      CaloBlueprintNodeBuilder::buildBlueprintNode(const Acts::GeometryContext& tgContext,
                                                   std::shared_ptr<Acts::BlueprintNode>&& childNode) {
    std::unique_ptr<CaloDetDescrManager> caloDetDescrMgr = buildCaloDetDescrNoAlign(serviceLocator(), Athena::getMessageSvc());
    
    /** Build the list of detector elements */
    DetElementMap_t detElements = fillDetectorElements(*caloDetDescrMgr);
    /** Convert the detector elements to surfaces */                                      
    SurfaceMap_t sensitives = translateToSurfaces(detElements);
    
    std::shared_ptr<Acts::BlueprintNode> caloNode{};
     /// Bare pointer to put the surfaces that cannot be put into wrapping cylinders
    Acts::TrackingVolume* caloVolume{}; 
    {
        auto caloEnvelope = envelopeVolume(tgContext, sensitives);
        caloVolume = caloEnvelope.get();
        caloNode = std::make_unique<Acts::StaticBlueprintNode>(std::move(caloEnvelope));
    }
    auto barrelNode = std::make_shared<Acts::CylinderContainerBlueprintNode>(
            "CaloBarrelCylinder", Acts::AxisDirection::AxisR);
    barrelNode->setAttachmentStrategy(Acts::VolumeAttachmentStrategy::Second);
    barrelNode->setResizeStrategy(Acts::VolumeResizeStrategy::Expand);
    if (childNode) {
        barrelNode->addChild(std::move(childNode));
    }
    std::unordered_map<CaloSampling::CaloSample, unsigned> barrelExtraCounts{};    
    for (std::shared_ptr<Acts::Surface>& surface : sensitives[Acts::toUnderlying(caloRegion::BarrelCylinder)]) {
        const auto layer = static_cast<CaloSampling::CaloSample>(surface->geometryId().layer());
        if (s_caloCylinderAsymmetricSampleList.count(layer) || 
            layer == CaloSampling::TileBar2) {
            std::ranges::for_each(fillLayer(tgContext, std::move(surface)),
                                  [&](std::shared_ptr<Acts::Surface>& surfToAdd){
                                    surfToAdd->assignGeometryId(Acts::GeometryIdentifier{}.withVolume(s_caloEnvelopeID)
                                                                .withLayer(layer)
                                                                .withSensitive(barrelExtraCounts[layer]++));
                                    caloVolume->addSurface(std::move(surfToAdd));
                                  });
            continue;
        }
        auto layerNode = std::make_shared<Acts::LayerBlueprintNode>(std::format("{:}_Layer", CaloSampling::getSamplingName(layer)));
        layerNode->setSurfaces(fillLayer(tgContext, std::move(surface)));
        layerNode->setEnvelope(Acts::ExtentEnvelope{{
                                     .z = {0.1_mm, 0.1_mm},
                                     .r = {1._cm, 1._cm}}});
        auto idNode = std::make_shared<Acts::GeometryIdentifierBlueprintNode>();
        idNode->addChild(std::move(layerNode));
        idNode->setAllVolumeIdsTo(s_caloBarrelId);
        idNode->setLayerIdTo(layer);
        barrelNode->addChild(std::move(idNode));
    }
    caloNode->addChild(std::move(barrelNode));
    for (const auto& disc: {caloRegion::DiscNegativeZ, caloRegion::DiscPositiveZ}){
        auto layerNode = std::make_shared<Acts::LayerBlueprintNode>(std::format("CaloLayer{:}", disc));
        layerNode->setEnvelope(Acts::ExtentEnvelope{{
                                    .z = {1_mm, 1_mm},
                                    .r = {2_mm, 2_mm}, }});
        using namespace ActsTrk::detail::GeoVolIds;
        const std::size_t volId = disc == caloRegion::DiscNegativeZ ? s_caloEndcapCId : s_caloEndcapAId; 
        std::vector<std::shared_ptr<Acts::Surface>> layerSurfaces{};

        for (auto& surface : sensitives[Acts::toUnderlying(disc)]) {

            const auto layer = static_cast<CaloSampling::CaloSample>(surface->geometryId().layer());
            if (layer != CaloSampling::TileGap3) {
                std::ranges::for_each(fillLayer(tgContext, std::move(surface)),
                                      [&](std::shared_ptr<Acts::Surface>& surf) {
                                        surf->assignGeometryId(Acts::GeometryIdentifier{}.withVolume(volId).
                                                            withLayer(layer).withSensitive(layerSurfaces.size()+1));
                                        layerSurfaces.push_back(std::move(surf));
                                      });
            } else {
                surface->assignGeometryId(surface->geometryId().withVolume(s_caloEnvelopeID)
                                .withSensitive(barrelExtraCounts[layer]++));
                caloVolume->addSurface(std::move(surface));
            }
        }
        layerNode->setSurfaces(std::move(layerSurfaces));
        auto idNode = std::make_shared<Acts::GeometryIdentifierBlueprintNode>();
        idNode->addChild(std::move(layerNode));
        idNode->setAllVolumeIdsTo(volId);
        caloNode->addChild(std::move(idNode));
    }
    return caloNode;
}
std::unique_ptr<Acts::TrackingVolume> CaloBlueprintNodeBuilder::envelopeVolume(const Acts::GeometryContext& tgContext, 
                                                                               const SurfaceMap_t& surfacePerRegion) const {
    double maxR{0.}, minZ{std::numeric_limits<double>::max()}, maxZ{-std::numeric_limits<double>::max()};
    for (auto& surfaces : surfacePerRegion) {
        for (auto& surface : surfaces) {
            const Amg::Vector3D center = surface->center(tgContext);
            if (surface->type() == Acts::Surface::SurfaceType::Disc) {
              minZ = std::min(minZ, center.z() - 0.5*surface->thickness());
              maxZ = std::max(maxZ, center.z() + 0.5*surface->thickness());
              const auto& bounds = static_cast<const Acts::RadialBounds&>(surface->bounds());
              maxR = std::max(maxR, bounds.rMax());
            } else {
              const auto& bounds = static_cast<const Acts::CylinderBounds&>(surface->bounds());
              using enum Acts::CylinderBounds::BoundValues;
              minZ = std::min(minZ, center.z() - bounds.get(eHalfLengthZ));
              maxZ = std::max(maxZ, center.z() + bounds.get(eHalfLengthZ));
              maxR = std::max(maxR, bounds.get(eR) + 0.5*surface->thickness());
            }
        }
    }
    auto volBounds = std::make_shared<Acts::CylinderVolumeBounds>(0, maxR + 1_cm,  0.5*(maxZ - minZ) + 1._cm);
    ATH_MSG_DEBUG(__func__<<"() Create envelope volume with bounds "<<(*volBounds));
    auto volume = std::make_unique<Acts::TrackingVolume>(Amg::getTranslateZ3D(0.5*(minZ + maxZ)),
                                                         volBounds, "ITkCalo");
    volume->assignGeometryId(Acts::GeometryIdentifier{}.withVolume(s_caloEnvelopeID));
    return volume;
  }
  std::vector<std::shared_ptr<Acts::Surface>> 
        CaloBlueprintNodeBuilder::fillLayer(const Acts::GeometryContext& tgContext,
                                            std::shared_ptr<Acts::Surface>&& centralSurf) const{
        unsigned extraSurf = m_surfaceDuplicates.value().at(centralSurf->geometryId().layer());
        if (extraSurf == 0) {
            return {std::move(centralSurf)};
        }
        const double stepLength =  centralSurf->thickness() / (extraSurf +1u);
        const Acts::Transform3& trf = centralSurf->localToGlobalTransform(tgContext);
        centralSurf->assignGeometryId(Acts::GeometryIdentifier{});
        std::vector<std::shared_ptr<Acts::Surface>> surfaces{};
        if (centralSurf->type() == Acts::Surface::SurfaceType::Cylinder) {
            const auto& bounds = static_cast<const Acts::CylinderBounds&>(centralSurf->bounds());
            const double radius = bounds.get(Acts::CylinderBounds::BoundValues::eR);
            const double halfZ = bounds.get(Acts::CylinderBounds::BoundValues::eHalfLengthZ);
            for (unsigned extra = 1ul; extra <= extraSurf; ++extra) {
                surfaces.emplace_back(Acts::Surface::makeShared<Acts::CylinderSurface>(trf, radius - stepLength* extra, halfZ));
                surfaces.emplace_back(Acts::Surface::makeShared<Acts::CylinderSurface>(trf, radius + stepLength* extra, halfZ));
            }
        } else if (centralSurf->type() == Acts::Surface::SurfaceType::Disc) {
            using DiscB = Acts::RadialBounds;
            auto bounds = std::make_shared<DiscB>(static_cast<const DiscB&>(centralSurf->bounds()));
            for (unsigned extra = 1ul; extra <= extraSurf; ++extra) {
                const double shiftZ = stepLength *extra;
                surfaces.emplace_back(Acts::Surface::makeShared<Acts::DiscSurface>(trf * Amg::getTranslateZ3D(shiftZ),
                                                                                   bounds));
                surfaces.emplace_back(Acts::Surface::makeShared<Acts::DiscSurface>(trf * Amg::getTranslateZ3D(-shiftZ),
                                                                                   bounds));
            }
        }
        surfaces.push_back(std::move(centralSurf));
        return surfaces;
  }
}