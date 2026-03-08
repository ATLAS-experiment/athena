/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// ATHENA

#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElementCollection.h"

// PACKAGE
#include "ActsGeometry/ActsHGTDLayerBuilder.h"
#include "ActsInterop/IdentityHelper.h"

// ACTS
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/ApproachDescriptor.hpp"
#include "Acts/Geometry/GenericApproachDescriptor.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/LayerCreator.hpp"
#include "Acts/Geometry/ProtoLayer.hpp"
#include "Acts/Material/ProtoSurfaceMaterial.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Utilities/BinningType.hpp"
#include "Acts/Surfaces/AnnulusBounds.hpp"

#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"

#include <Acts/Utilities/AxisDefinitions.hpp>
#include <iterator>
#include <unordered_map>

using Acts::Surface;
using Acts::Transform3;
using Acts::Translation3;

using namespace Acts::UnitLiterals;

ActsHGTDLayerBuilder::ActsHGTDLayerBuilder(const Config&                cfg,
                      std::unique_ptr<const Acts::Logger> logger)
  : m_cfg (cfg),
    m_logger(std::move(logger))
{
}

const Acts::LayerVector
ActsHGTDLayerBuilder::negativeLayers(const Acts::GeometryContext &gctx) const {
  ACTS_VERBOSE("Building negative layers");
  Acts::LayerVector nVector;
  buildEndcap(gctx, nVector, -1);
  return nVector;
}

const Acts::LayerVector
ActsHGTDLayerBuilder::centralLayers(const Acts::GeometryContext & /*gctx*/) const {
  ACTS_VERBOSE("HGTD has no central layers");
  Acts::LayerVector layers;
  return layers;
}

const Acts::LayerVector
ActsHGTDLayerBuilder::positiveLayers(const Acts::GeometryContext &gctx) const {
  ACTS_VERBOSE("Building positive layers");
  Acts::LayerVector pVector;
  buildEndcap(gctx, pVector, 1);
  return pVector;
}

void ActsHGTDLayerBuilder::buildEndcap(const Acts::GeometryContext &gctx,
                                   Acts::LayerVector &layersOutput, int type) const
{
  using enum Acts::AxisDirection;

  ACTS_VERBOSE("Build layers: " << (type < 0 ? "NEGATIVE" : "POSITIVE")
                                << " ENDCAP");

  std::vector<std::shared_ptr<const ActsDetectorElement>> elements =
      getDetectorElements();
  std::map<int, std::vector<const Acts::Surface *>>
      initialLayers{};

  for (const auto &element : elements) {

    Identifier currentId(element->identify());
    // Check if the element is in the correct endcap (type -1 or 1)
    if (m_cfg.idHelper->endcap(currentId) * type <= 0) {
      continue;
    }

    m_cfg.elementStore->push_back(element);
    int currentLayer {m_cfg.idHelper->layer(currentId)};

    initialLayers[currentLayer].push_back(&element->surface());
  }

  ACTS_VERBOSE("Found " << initialLayers.size() << " "
                        << (type < 0 ? "NEGATIVE" : "POSITIVE")
                        << " ENDCAP inital layers");

  // Loops over a provided vector of surface and calculates the various
  // min/max values in one go. Also takes into account the thickness
  // of an associated DetectorElement, if it exists.
  //
  // @param gctx The current geometry context object, e.g. alignment
  // @param surfaces The vector of surfaces to consider

  std::vector<Acts::ProtoLayer> protoLayers;
  protoLayers.reserve(initialLayers.size());

  for (const auto &[key, surfaces] : initialLayers) {
    auto &pl = protoLayers.emplace_back(gctx, surfaces);
    pl.envelope[AxisR] = m_cfg.endcapEnvelopeR;
    pl.envelope[AxisZ] = m_cfg.endcapEnvelopeZ;
  }


  // sort proto layers by their medium z position
  std::sort(protoLayers.begin(), protoLayers.end(),
            [type](const Acts::ProtoLayer &a, const Acts::ProtoLayer &b) {
              double midA = (a.min(AxisZ) + a.max(AxisZ)) / 2.0;
              double midB = (b.min(AxisZ) + b.max(AxisZ)) / 2.0;
              if (type < 0) {
                return midA < midB;
              } else {
                return midA > midB;
              }
            });

  std::vector<std::shared_ptr<const Surface>> ownedSurfaces;
  for (const auto &pl : protoLayers) {

    std::unique_ptr<Acts::ApproachDescriptor> approachDescriptor = nullptr;
    std::shared_ptr<const Acts::ProtoSurfaceMaterial> materialProxy = nullptr;

    double layerZ = pl.medium(AxisZ);
    double layerHalfZ = 0.5 * pl.range(AxisZ);

    double layerZInner = layerZ - layerHalfZ;
    double layerZOuter = layerZ + layerHalfZ;

    if (std::abs(layerZInner) > std::abs(layerZOuter))
      std::swap(layerZInner, layerZOuter);

    std::vector<std::shared_ptr<const Acts::Surface>> aSurfaces;

    Acts::Transform3 transformNominal(Translation3(0., 0., layerZ));
    Acts::Transform3 transformInner(Translation3(0., 0., layerZInner));
    Acts::Transform3 transformOuter(Translation3(0., 0., layerZOuter));

    std::shared_ptr<Acts::DiscSurface> innerBoundary =
      Acts::Surface::makeShared<Acts::DiscSurface>(
        transformInner, pl.min(AxisR), pl.max(AxisR));
    aSurfaces.push_back(innerBoundary);

    std::shared_ptr<Acts::DiscSurface> nominalSurface =
      Acts::Surface::makeShared<Acts::DiscSurface>(
        transformNominal, pl.min(AxisR), pl.max(AxisR));
    aSurfaces.push_back(nominalSurface);

    std::shared_ptr<Acts::DiscSurface> outerBoundary =
      Acts::Surface::makeShared<Acts::DiscSurface>(
        transformOuter, pl.min(AxisR), pl.max(AxisR));
    aSurfaces.push_back(outerBoundary);

    size_t matBinsPhi = m_cfg.endcapMaterialBins.first;
    size_t matBinsR = m_cfg.endcapMaterialBins.second;

    Acts::BinUtility materialBinUtil(matBinsPhi, -M_PI, M_PI, Acts::closed,
                                     AxisPhi);
    materialBinUtil +=
        Acts::BinUtility(matBinsR, pl.min(AxisR), pl.max(AxisR),
                         Acts::open, AxisR, transformNominal);

    materialProxy =
        std::make_shared<const Acts::ProtoSurfaceMaterial>(materialBinUtil);

    ACTS_VERBOSE("[L] Layer is marked to carry support material on Surface ( "
                 "inner=0 / center=1 / outer=2 ) : "
                 << "inner");
    ACTS_VERBOSE("with binning: [" << matBinsPhi << ", " << matBinsR << "]");

    ACTS_VERBOSE("Created ApproachSurfaces for disc layer at:");
    ACTS_VERBOSE(" - inner:   Z=" << layerZInner);
    ACTS_VERBOSE(" - central: Z=" << layerZ);
    ACTS_VERBOSE(" - outer:   Z=" << layerZOuter);

    // set material on inner
    innerBoundary->assignSurfaceMaterial(materialProxy);

    std::set<int> phiModuleByRing;
    // want to figure out bins in phi
    for (const auto &srf : pl.surfaces()) {
      auto elm = dynamic_cast<const ActsDetectorElement *>(
          srf->surfacePlacement());
      if (elm) {
        auto id = elm->identify();
        phiModuleByRing.insert(m_cfg.idHelper->phi_module(id));
      }
    }

    size_t nModPhi = 50; //phiModuleByRing.size();
    size_t nModR = 1;// pl.surfaces().size()/nModPhi;

    ACTS_VERBOSE("Identifier reports: " << nModPhi << " is lowest for " << nModR
                                        << " r-rings");

    size_t nBinsPhi = nModPhi * m_cfg.numberOfBinsFactor;
    size_t nBinsR = nModR * m_cfg.numberOfBinsFactor;


    ACTS_VERBOSE("Creating r x phi binned layer with " << nBinsR << " x "
                                                       << nBinsPhi << " bins");


    approachDescriptor =
        std::make_unique<Acts::GenericApproachDescriptor>(aSurfaces);

    // get ownership of pl surfaces
    ownedSurfaces.clear();
    ownedSurfaces.reserve(pl.surfaces().size());
    std::transform(pl.surfaces().begin(), pl.surfaces().end(),
                   std::back_inserter(ownedSurfaces),
                   [](const auto &s) { return s->getSharedPtr(); });

    auto layer = m_cfg.layerCreator->discLayer(gctx, ownedSurfaces, nBinsR,
                                               nBinsPhi, pl, Transform3::Identity(),
                                               std::move(approachDescriptor));

    layersOutput.push_back( std::move(layer) );
  }
}

std::vector<std::shared_ptr<const ActsDetectorElement>>
ActsHGTDLayerBuilder::getDetectorElements() const {
  ACTS_VERBOSE("Retrieving detector elements from detector manager");
  if ( not m_cfg.mng ) {
    ACTS_ERROR("Manager is null");
    throw std::runtime_error{"Detector manager is null"};
  }
  auto hgtdDetMng = static_cast<const HGTD_DetectorManager *>(m_cfg.mng);
  ACTS_VERBOSE("Detector manager has "
               << std::distance(hgtdDetMng->getDetectorElementCollection()->begin(),
                                hgtdDetMng->getDetectorElementCollection()->end())
               << " elements");

  std::vector<std::shared_ptr<const ActsDetectorElement>> elements;

  for (auto iter = hgtdDetMng->getDetectorElementCollection()->begin();
       iter != hgtdDetMng->getDetectorElementCollection()->end(); ++iter) {
    const InDetDD::HGTD_DetectorElement *detElement =
        dynamic_cast<const InDetDD::HGTD_DetectorElement *>(*iter);

    if (not detElement) {
      ACTS_ERROR("Detector element was nullptr");
      throw std::runtime_error{"Corrupt detector element collection"};
    }
    elements.push_back(
        std::make_shared<const ActsDetectorElement>(*detElement, (*iter)->identify()));
  }
  ACTS_VERBOSE("Retrieved " << elements.size() << " elements");

  return elements;
}
