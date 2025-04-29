/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingVolume.cxx, (b) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Gaudi Kernel
#include "GaudiKernel/MsgStream.h"
// Trk
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/CylinderLayerAttemptsCalculator.h"
#include "TrkGeometry/DiscLayerAttemptsCalculator.h"
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/NavigationLayer.h"
#include "TrkGeometry/PlaneLayer.h"
#include "TrkGeometry/SubtractedCylinderLayer.h"
#include "TrkGeometry/SubtractedPlaneLayer.h"
#include "TrkGeometry/TrackingVolume.h"
//
#include "TrkVolumes/BoundaryCylinderSurface.h"
#include "TrkVolumes/BoundaryDiscSurface.h"
#include "TrkVolumes/BoundaryPlaneSurface.h"
#include "TrkVolumes/BoundarySubtractedCylinderSurface.h"
#include "TrkVolumes/BoundarySubtractedPlaneSurface.h"
#include "TrkVolumes/CombinedVolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkVolumes/SimplePolygonBrepVolumeBounds.h"
#include "TrkVolumes/SubtractedVolumeBounds.h"
#include "TrkVolumes/VolumeBounds.h"
//
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/DistanceSolution.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/Surface.h"
//
#include "TrkGeometrySurfaces/SubtractedCylinderSurface.h"
#include "TrkGeometrySurfaces/SubtractedPlaneSurface.h"
//
#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/NavBinnedArray1D.h"
// CLHEP
#include "GeoPrimitives/GeoPrimitives.h"
#include <utility>

Trk::TrackingVolume::TrackingVolume()
  : Volume()
  , Material()
  , m_motherVolume(nullptr)
  , m_boundarySurfaces{}
  , m_confinedLayers(nullptr)
  , m_confinedVolumes(nullptr)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(nullptr)
  , m_confinedArbitraryLayers(nullptr)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name("undefined")
  , m_colorCode(20)
  , m_redoNavigation(false)
{}

// constructor: 1 a)
Trk::TrackingVolume::TrackingVolume(
  std::unique_ptr<Amg::Transform3D> htrans,
  std::shared_ptr<VolumeBounds> volbounds,
  LayerArray* subLayers,
  TrackingVolumeArray* subVolumes,
  const std::string& volumeName)
  : Volume(std::move(htrans), std::move(volbounds))
  , Material()
  , m_motherVolume(nullptr)
  , m_boundarySurfaces{}
  , m_confinedLayers(subLayers)
  , m_confinedVolumes(subVolumes)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(nullptr)
  , m_confinedArbitraryLayers(nullptr)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
  createLayerAttemptsCalculator();
  interlinkLayers();
}

// constructor: 2 a)
Trk::TrackingVolume::TrackingVolume(const Volume& volume,
                                    const Material& matprop,
                                    LayerArray* subLayers,
                                    TrackingVolumeArray* subVolumes,
                                    const std::string& volumeName)
  : Volume(volume)
  , Material(matprop)
  , m_motherVolume(nullptr)
  , m_boundarySurfaces{}
  , m_confinedLayers(subLayers)
  , m_confinedVolumes(subVolumes)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(nullptr)
  , m_confinedArbitraryLayers(nullptr)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
  createLayerAttemptsCalculator();
  interlinkLayers();
}

// constructor: 3 a)
Trk::TrackingVolume::TrackingVolume(
  std::unique_ptr<Amg::Transform3D> htrans,
  std::shared_ptr<VolumeBounds> volbounds,
  const Material& matprop,
  LayerArray* subLayers,
  TrackingVolumeArray* subVolumes,
  const std::string& volumeName)
  : Volume(std::move(htrans), std::move(volbounds))
  , Material(matprop)
  , m_motherVolume(nullptr)
  , m_confinedLayers(subLayers)
  , m_confinedVolumes(subVolumes)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(nullptr)
  , m_confinedArbitraryLayers(nullptr)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
  createLayerAttemptsCalculator();
  interlinkLayers();
}

// 1 b)
Trk::TrackingVolume::TrackingVolume(
  std::unique_ptr<Amg::Transform3D> htrans,
  std::shared_ptr<VolumeBounds> volbounds,
  const Material& matprop,
  std::vector<DetachedTrackingVolume*>* detachedSubVolumes,
  const std::string& volumeName)
  : Volume(std::move(htrans), std::move(volbounds))
  , Material(matprop)
  , m_motherVolume(nullptr)
  , m_confinedLayers(nullptr)
  , m_confinedVolumes(nullptr)
  , m_confinedDetachedVolumes(detachedSubVolumes)
  , m_confinedDenseVolumes(nullptr)
  , m_confinedArbitraryLayers(nullptr)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
}

// 2 b)
Trk::TrackingVolume::TrackingVolume(
  const Volume& volume,
  const Material& matprop,
  std::vector<DetachedTrackingVolume*>* detachedSubVolumes,
  const std::string& volumeName)
  : Volume(volume)
  , Material(matprop)
  , m_motherVolume(nullptr)
  , m_confinedLayers(nullptr)
  , m_confinedVolumes(nullptr)
  , m_confinedDetachedVolumes(detachedSubVolumes)
  , m_confinedDenseVolumes(nullptr)
  , m_confinedArbitraryLayers(nullptr)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
}

// 1 d)
Trk::TrackingVolume::TrackingVolume(
  std::unique_ptr<Amg::Transform3D> htrans,
  std::shared_ptr<VolumeBounds> volbounds,
  const Material& matprop,
  const std::vector<TrackingVolume*>* unorderedSubVolumes,
  const std::string& volumeName)
  : Volume(std::move(htrans), std::move(volbounds))
  , Material(matprop)
  , m_motherVolume(nullptr)
  , m_confinedLayers(nullptr)
  , m_confinedVolumes(nullptr)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(unorderedSubVolumes)
  , m_confinedArbitraryLayers(nullptr)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
}

// 2 d)
Trk::TrackingVolume::TrackingVolume(
  const Volume& volume,
  const Material& matprop,
  const std::vector<TrackingVolume*>* unorderedSubVolumes,
  const std::string& volumeName)
  : Volume(volume)
  , Material(matprop)
  , m_motherVolume(nullptr)
  , m_confinedLayers(nullptr)
  , m_confinedVolumes(nullptr)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(unorderedSubVolumes)
  , m_confinedArbitraryLayers(nullptr)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
}

// 1 c)
Trk::TrackingVolume::TrackingVolume(
  std::unique_ptr<Amg::Transform3D> htrans,
  std::shared_ptr<VolumeBounds> volbounds,
  const Material& matprop,
  const std::vector<Layer*>* layers,
  const std::string& volumeName)
  : Volume(std::move(htrans), std::move(volbounds))
  , Material(matprop)
  , m_motherVolume(nullptr)
  , m_confinedLayers(nullptr)
  , m_confinedVolumes(nullptr)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(nullptr)
  , m_confinedArbitraryLayers(layers)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
}

// 2 c)
Trk::TrackingVolume::TrackingVolume(const Volume& volume,
                                    const Material& matprop,
                                    const std::vector<Layer*>* layers,
                                    const std::string& volumeName)
  : Volume(volume)
  , Material(matprop)
  , m_motherVolume(nullptr)
  , m_confinedLayers(nullptr)
  , m_confinedVolumes(nullptr)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(nullptr)
  , m_confinedArbitraryLayers(layers)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
}

// 1 d)
Trk::TrackingVolume::TrackingVolume(
  std::unique_ptr<Amg::Transform3D> htrans,
  std::shared_ptr<VolumeBounds> volbounds,
  const std::vector<Layer*>* layers,
  const std::vector<TrackingVolume*>* unorderedSubVolumes,
  const Material& matprop,
  const std::string& volumeName)
  : Volume(std::move(htrans), std::move(volbounds))
  , Material(matprop)
  , m_motherVolume(nullptr)
  , m_confinedLayers(nullptr)
  , m_confinedVolumes(nullptr)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(unorderedSubVolumes)
  , m_confinedArbitraryLayers(layers)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
}

// 2 d)
Trk::TrackingVolume::TrackingVolume(
  const Volume& volume,
  const std::vector<Layer*>* layers,
  const std::vector<TrackingVolume*>* unorderedSubVolumes,
  const Material& matprop,
  const std::string& volumeName)
  : Volume(volume)
  , Material(matprop)
  , m_motherVolume(nullptr)
  , m_confinedLayers(nullptr)
  , m_confinedVolumes(nullptr)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(unorderedSubVolumes)
  , m_confinedArbitraryLayers(layers)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(volumeName)
  , m_colorCode(20)
  , m_redoNavigation(false)
{
  createBoundarySurfaces();
}

Trk::TrackingVolume::TrackingVolume(const Trk::TrackingVolume& trVol,
                                    Amg::Transform3D& transform)
  : Volume(trVol, transform)
  , Material(trVol)
  , m_motherVolume(trVol.m_motherVolume)
  , m_boundarySurfaces{}
  , m_confinedLayers(nullptr)
  , m_confinedVolumes(nullptr)
  , m_confinedDetachedVolumes(nullptr)
  , m_confinedDenseVolumes(nullptr)
  , m_confinedArbitraryLayers(nullptr)
  , m_outsideGlueVolumes(nullptr)
  , m_layerAttemptsCalculator(nullptr)
  , m_geometrySignature(Trk::Unsigned)
  , m_geometryType(Trk::NumberOfGeometryTypes)
  , m_name(trVol.m_name)
  , m_colorCode(trVol.m_colorCode)
  , m_redoNavigation(trVol.m_redoNavigation)
{
  // createBoundarySurfaces
  m_boundarySurfaces.reserve(trVol.boundarySurfaces().size());
  const Trk::TrackingVolume* in = nullptr;
  const Trk::TrackingVolume* out = nullptr;
  for (size_t ib = 0; ib < trVol.boundarySurfaces().size(); ib++) {
    in = trVol.boundarySurfaces()[ib]->insideVolume() == &trVol ? this
                                                                      : nullptr;
    out = in == nullptr ? this : nullptr;
    const Trk::CylinderSurface* cyl = dynamic_cast<const Trk::CylinderSurface*>(
      trVol.boundarySurfaces()[ib]);
    const Trk::DiscSurface* dis =
      dynamic_cast<const Trk::DiscSurface*>(trVol.boundarySurfaces()[ib]);
    const Trk::PlaneSurface* pla = dynamic_cast<const Trk::PlaneSurface*>(
      trVol.boundarySurfaces()[ib]);
    const Trk::SubtractedCylinderSurface* scyl =
      dynamic_cast<const Trk::SubtractedCylinderSurface*>(
        trVol.boundarySurfaces()[ib]);
    const Trk::SubtractedPlaneSurface* spla =
      dynamic_cast<const Trk::SubtractedPlaneSurface*>(
        trVol.boundarySurfaces()[ib]);
    if (scyl)
      m_boundarySurfaces.push_back(
        std::make_shared<Trk::BoundarySubtractedCylinderSurface<Trk::TrackingVolume>>(
            in, out, *scyl, transform));
    else if (spla)
      m_boundarySurfaces.push_back(
        std::make_shared<Trk::BoundarySubtractedPlaneSurface<Trk::TrackingVolume>>(
            in, out, *spla, transform));
    else if (cyl)
      m_boundarySurfaces.push_back(
        std::make_shared<Trk::BoundaryCylinderSurface<Trk::TrackingVolume>>(
            in, out, *cyl, transform));
    else if (dis)
      m_boundarySurfaces.push_back(
        std::make_shared<Trk::BoundaryDiscSurface<Trk::TrackingVolume>>(
            in, out, *dis, transform));
    else if (pla)
      m_boundarySurfaces.push_back(
        std::make_shared<Trk::BoundaryPlaneSurface<Trk::TrackingVolume>>(
            in, out, *pla, transform));
  }

  // confined layers
  const Trk::BinnedArray<Trk::Layer>* confinedLayers = trVol.confinedLayers();
  if (confinedLayers) {
    Trk::BinnedArraySpan<Trk::Layer const* const> layers = confinedLayers->arrayObjects();
    std::vector<std::shared_ptr<Trk::Layer>> layerOrder;
    layerOrder.reserve(layers.size());
    for (const auto *layer : layers) {
      const Trk::PlaneLayer* lay =
        dynamic_cast<const Trk::PlaneLayer*>(layer);
      if (lay) {
        Trk::PlaneLayer* newlay = new Trk::PlaneLayer(*lay, transform);
        layerOrder.push_back(std::shared_ptr<Trk::Layer>(newlay));
      }
    }
    const Trk::NavBinnedArray1D<Trk::Layer>* confLays =
      dynamic_cast<const Trk::NavBinnedArray1D<Trk::Layer>*>(confinedLayers);
    if (confLays)
      m_confinedLayers = new Trk::NavBinnedArray1D<Trk::Layer>(
        *confLays,
        std::vector<std::shared_ptr<Trk::Layer>>(layerOrder),
        transform);
  }

  // confined 'unordered' layers
  Trk::ArraySpan<const Trk::Layer* const> confinedArbitraryLayers =
    trVol.confinedArbitraryLayers();
  if (!confinedArbitraryLayers.empty()) {
    // clone & apply the transform
    std::vector<Trk::Layer*> uLayers;
    uLayers.reserve(confinedArbitraryLayers.size());
    for (const auto *confinedArbitraryLayer : confinedArbitraryLayers) {
      const Trk::SubtractedPlaneLayer* slayer =
        dynamic_cast<const Trk::SubtractedPlaneLayer*>(
          confinedArbitraryLayer);
      const Trk::SubtractedCylinderLayer* sclayer =
        dynamic_cast<const Trk::SubtractedCylinderLayer*>(
          confinedArbitraryLayer);
      const Trk::PlaneLayer* layer =
        dynamic_cast<const Trk::PlaneLayer*>(confinedArbitraryLayer);
      const Trk::CylinderLayer* clayer =
        dynamic_cast<const Trk::CylinderLayer*>(confinedArbitraryLayer);

      if (slayer) {
        Trk::SubtractedPlaneLayer* lay =
          new Trk::SubtractedPlaneLayer(*slayer, transform);
        uLayers.push_back(lay);
      } else if (layer) {
        Trk::PlaneLayer* lay = new Trk::PlaneLayer(*layer, transform);
        uLayers.push_back(lay);
      } else if (sclayer) {
        Trk::SubtractedCylinderLayer* lay =
          new Trk::SubtractedCylinderLayer(*sclayer, transform);
        uLayers.push_back(lay);
      } else if (clayer) {
        Trk::CylinderLayer* lay = new Trk::CylinderLayer(*clayer, transform);
        uLayers.push_back(lay);
      }
    }
    m_confinedArbitraryLayers = new std::vector<Trk::Layer*>(uLayers);
  }

  // confined volumes
  const Trk::BinnedArray<Trk::TrackingVolume>* confinedVolumes =
    trVol.confinedVolumes();
  if (confinedVolumes) {
    // retrieve array objects and apply the transform
    Trk::BinnedArraySpan<Trk::TrackingVolume const * const > volumes =
      confinedVolumes->arrayObjects();
    std::vector<std::shared_ptr<Trk::TrackingVolume>> volOrder;
    volOrder.reserve(volumes.size());
    for (const auto *volume : volumes) {
      Trk::TrackingVolume* vol = new Trk::TrackingVolume(*volume, transform);
      volOrder.push_back(std::shared_ptr<TrackingVolume>(vol));
    }
    const Trk::NavBinnedArray1D<Trk::TrackingVolume>* confVols =
      dynamic_cast<const Trk::NavBinnedArray1D<Trk::TrackingVolume>*>(confinedVolumes);
    if (confVols)
      m_confinedVolumes = new Trk::NavBinnedArray1D<Trk::TrackingVolume>(
        *confVols,
        std::vector<std::shared_ptr<Trk::TrackingVolume>>(volOrder),
        transform);
  }

  // confined unordered volumes
  Trk::ArraySpan<const Trk::TrackingVolume* const> confinedDenseVolumes =
    trVol.confinedDenseVolumes();
  if (!confinedDenseVolumes.empty()) {
    std::vector<Trk::TrackingVolume*> newVol;
    newVol.reserve(confinedDenseVolumes.size());
    // retrieve array objects and apply the transform
    for (const auto *confinedDenseVolume : confinedDenseVolumes) {
      Trk::TrackingVolume* vol =
        new Trk::TrackingVolume(*confinedDenseVolume, transform);
      newVol.push_back(vol);
    }
    m_confinedDenseVolumes =
      new std::vector<Trk::TrackingVolume*>(newVol);
  }
}

Trk::TrackingVolume::~TrackingVolume()
{
  delete m_confinedLayers;
  delete m_confinedVolumes;
  delete m_confinedDetachedVolumes;
  if (m_confinedDenseVolumes) {
    for (auto * confinedDenseVolume : *m_confinedDenseVolumes){
      delete confinedDenseVolume;
    }
    delete m_confinedDenseVolumes;
  }
  if (m_confinedArbitraryLayers) {
    for (auto * confinedArbitraryLayer : *m_confinedArbitraryLayers){
      delete confinedArbitraryLayer;
    }
    delete m_confinedArbitraryLayers;
  }
  delete m_layerAttemptsCalculator;
}

void
Trk::TrackingVolume::clear()
{
  if (m_confinedVolumes) {
    delete m_confinedVolumes;
    m_confinedVolumes = nullptr;
  }
  if (m_confinedLayers) {
    delete m_confinedLayers;
    m_confinedLayers = nullptr;
  }
  if (m_confinedDenseVolumes) {
    for (auto * confinedDenseVolume : *m_confinedDenseVolumes){
      delete confinedDenseVolume;
    }
    delete m_confinedDenseVolumes;
    m_confinedDenseVolumes = nullptr;
  }
  if (m_confinedArbitraryLayers) {
    for (auto *confinedArbitraryLayer : *m_confinedArbitraryLayers){
      delete confinedArbitraryLayer;
    }
    delete m_confinedArbitraryLayers;
    m_confinedArbitraryLayers = nullptr;
  }
}

const Trk::Layer*
Trk::TrackingVolume::associatedLayer(const Amg::Vector3D& gp) const
{
  // confined layers
  if (m_confinedLayers)
    return (confinedLayers()->object(gp));
  // confined arbitrary
  if (m_confinedArbitraryLayers) {
    for (auto * confinedArbitraryLayer : *m_confinedArbitraryLayers){
      if (confinedArbitraryLayer->isOnLayer(gp)){
        return confinedArbitraryLayer;
      }
    }
  }
  return nullptr;
}

Trk::Layer*
Trk::TrackingVolume::associatedLayer(const Amg::Vector3D& gp)
{
  // confined layers
  if (m_confinedLayers)
    return (confinedLayers()->object(gp));
  // confined arbitrary
  if (m_confinedArbitraryLayers) {
    for (auto * confinedArbitraryLayer : *m_confinedArbitraryLayers){
      if (confinedArbitraryLayer->isOnLayer(gp)){
        return confinedArbitraryLayer;
      }
    }
  }
  return nullptr;
}



const Trk::Layer*
Trk::TrackingVolume::nextLayer(const Amg::Vector3D& gp,
                               const Amg::Vector3D& mom,
                               bool associatedResult,
                               bool skipNavLayer) const
{
  const Trk::Layer* nextLayer = nullptr;
  if (m_confinedLayers)
    nextLayer = (confinedLayers()->nextObject(gp, mom, associatedResult));
  // forward it in this case
  if (!skipNavLayer)
    return nextLayer;
  // if only material or layers
  if (nextLayer &&
      (nextLayer->layerMaterialProperties() || nextLayer->surfaceArray()))
    return nextLayer;
  // try to get the next layer that has either material or sub surfaces
  while (nextLayer && (!(nextLayer->layerMaterialProperties()) &&
                       !(nextLayer->surfaceArray())))
    nextLayer = (confinedLayers()->nextObject(gp, mom, associatedResult));
  return nextLayer;
}

Trk::LayerIntersection<Amg::Vector3D>
Trk::TrackingVolume::closestMaterialLayer(const Amg::Vector3D& gp,
                                          const Amg::Vector3D& dir,
                                          PropDirection pDir,
                                          const BoundaryCheck& bchk) const
{
  // the layer candidates to check
  std::vector<const Layer*> layerCandidates;

  // ---------------- BOUNDARY LAYER SECTION (only for mapping) ----------
  if (pDir == mappingMode) {
    const auto& bSurfaces = boundarySurfaces();
    for (size_t ib = 0; ib < bSurfaces.size(); ++ib) {
      if (bSurfaces[ib]->surfaceRepresentation().materialLayer())
        layerCandidates.push_back(
          bSurfaces[ib]->surfaceRepresentation().materialLayer());
    }
  }
  // ---------------- CONFINED LAYER SECTION --------------
  if (m_confinedLayers) {
    // the associated layer
    const Trk::Layer* assocLayer = associatedLayer(gp);
    const Trk::Layer* previousMatLayer = nullptr;
    const Trk::Layer* nextMatLayer = nullptr;
    // if the associated layer is a navigation layer - get the previous and next
    // from this one
    const Trk::NavigationLayer* navLayer =
      dynamic_cast<const Trk::NavigationLayer*>(assocLayer);
    if (navLayer) {
      // get previous / next
      previousMatLayer = navLayer->previousLayer();
      nextMatLayer = navLayer->nextLayer();
      if (previousMatLayer)
        layerCandidates.push_back(previousMatLayer);
      if (nextMatLayer)
        layerCandidates.push_back(nextMatLayer);
    } else
      layerCandidates.push_back(assocLayer);
  }
  // --- solve for the layer candidates ---------------------
  //
  Trk::Intersection laySurfIntersection(
    Amg::Vector3D(0., 0., 0.), 10e10, false);
  // layer candidates found - continue
  if (!layerCandidates.empty()) {
    const Layer* cLayer = nullptr;
    // iterate through and chose
    for (auto& lcIter : layerCandidates) {
      // only the forceFwd solution is possible here
      bool forceDir = (pDir == alongMomentum || pDir == oppositeMomentum);
      double dirScalor = (pDir == oppositeMomentum) ? -1. : 1.;
      // get the intersection soltuion
      Trk::Intersection sfI =
        (*lcIter).surfaceRepresentation().straightLineIntersection(
          gp, dirScalor * dir, forceDir, bchk);
      if (sfI.valid &&
          (sfI.pathLength * sfI.pathLength) <
            (laySurfIntersection.pathLength * laySurfIntersection.pathLength)) {
        laySurfIntersection = sfI;
        cLayer = lcIter;
      }
    }
    // now return the pair: in case of a valid intersection, or if no mapping
    // mode is chosen
    if (cLayer)
      return Trk::LayerIntersection<Amg::Vector3D>(
        laySurfIntersection,
        cLayer,
        &(cLayer->surfaceRepresentation()),
        nullptr,
        pDir);
  }
  // mapping mode chosen, but no valid intersection yet
  const Trk::TrackingVolume* nVolume = nextVolume(gp, dir, pDir);

  // forward the next Volume solution or a 0 solution
  return (nVolume && nVolume != this)
           ? nVolume->closestMaterialLayer(gp, dir, pDir, bchk)
           : Trk::LayerIntersection<Amg::Vector3D>(
               laySurfIntersection, nullptr, nullptr, nullptr, pDir);
}

const Trk::TrackingVolume*
Trk::TrackingVolume::associatedSubVolume(const Amg::Vector3D& gp) const
{
  if (m_confinedVolumes)
    return (m_confinedVolumes->object(gp));

  if (m_confinedDetachedVolumes) {
    for (auto *confinedDetachedVolume : *m_confinedDetachedVolumes) {
      if (confinedDetachedVolume->trackingVolume()->inside(gp, 0.001)){
        return confinedDetachedVolume->trackingVolume();
      }
    }
  }

  if (m_confinedDenseVolumes) {
    for (auto *confinedDenseVolume : *m_confinedDenseVolumes)
      if (confinedDenseVolume->inside(gp, 0.001)){
        return confinedDenseVolume;
      }
  }

  return this;
}

Trk::TrackingVolume*
Trk::TrackingVolume::associatedSubVolume(const Amg::Vector3D& gp)
{
  if (m_confinedVolumes)
    return (m_confinedVolumes->object(gp));

  if (m_confinedDetachedVolumes) {
    for (auto *confinedDetachedVolume : *m_confinedDetachedVolumes) {
      if (confinedDetachedVolume->trackingVolume()->inside(gp, 0.001)){
        return confinedDetachedVolume->trackingVolume();
      }
    }
  }

  if (m_confinedDenseVolumes) {
    for (auto * confinedDenseVolume : *m_confinedDenseVolumes){
      if (confinedDenseVolume->inside(gp, 0.001)){
        return confinedDenseVolume;
      }
    }
  }

  return this;
}


const Trk::TrackingVolume*
Trk::TrackingVolume::nextVolume(const Amg::Vector3D& gp,
                                const Amg::Vector3D& dir,
                                Trk::PropDirection pDir) const
{
  // get the boundary surfaces & intersect them
  const Trk::TrackingVolume* nVolume = nullptr;
  // fix the direction once
  bool forceDir = (pDir == Trk::alongMomentum || pDir == Trk::oppositeMomentum);
  double dirScalor = (pDir == Trk::oppositeMomentum) ? -1. : 1.;
  Amg::Vector3D cDir = dirScalor * dir;
  double pathLength = 10e10;
  // now loop through the and find the closest
  const auto& bSurfaces = boundarySurfaces();
  for (size_t ib = 0; ib < bSurfaces.size(); ++ib) {
    // get the intersection soltuion
    Trk::Intersection sfI =
      bSurfaces[ib]->surfaceRepresentation().straightLineIntersection(
        gp, cDir, forceDir, true);
    if (sfI.valid &&
        (sfI.pathLength * sfI.pathLength) < (pathLength * pathLength)) {
      // assign the next Volume
      Trk::PropDirection attachedDir =
        sfI.pathLength > 0. ? Trk::alongMomentum : Trk::oppositeMomentum;
      pathLength = sfI.pathLength;
      nVolume = bSurfaces[ib]->attachedVolume(gp, cDir, attachedDir);
    }
  }
  return nVolume;
}

const Trk::TrackingVolume*
Trk::TrackingVolume::nextSubVolume(const Amg::Vector3D& gp,
                                   const Amg::Vector3D& mom) const
{
  if (m_confinedVolumes)
    return (m_confinedVolumes->nextObject(gp, mom));
  return this;
}

std::vector<const Trk::DetachedTrackingVolume*>
Trk::TrackingVolume::assocDetachedSubVolumes(const Amg::Vector3D& gp,
                                             double tol) const
{
  auto currVols = std::vector<const Trk::DetachedTrackingVolume*>();

  Trk::ArraySpan<const Trk::DetachedTrackingVolume* const> detVols =
    confinedDetachedVolumes();
  if (!detVols.empty()) {
    for (const auto *detVol : detVols) {
      if (detVol->trackingVolume()->inside(gp, tol))
        currVols.push_back(detVol);
    }
  }
  return currVols;
}

void
Trk::TrackingVolume::indexContainedStaticLayers(GeometrySignature geoSig,
                                                int& offset)
{
  // the offset gets increased internally
  // the static layers first
  // ------------------------------------------------------------------
  if (m_confinedLayers) {
    Trk::BinnedArraySpan<Trk::Layer* const> layers = confinedLayers()->arrayObjects();
    for (Trk::Layer* layerptr : layers) {
      // only index the material layers & only those that have not yet been
      // singed
      if (layerptr && layerptr->layerIndex().value() < 0) {
        // sign only those with material properties - rest goes to 0
        Trk::LayerIndex layIndex =
          layerptr->layerMaterialProperties()
            ? Trk::LayerIndex(
                int(geoSig) * TRKDETDESCR_GEOMETRYSIGNATUREWEIGHT + (++offset))
            : Trk::LayerIndex(0);
        // now register the index
        layerptr->registerLayerIndex(layIndex);
      }
    }
  }

  // the boundary surface layer
  auto& bSurfaces = boundarySurfaces();
  for (const auto& bsIter : bSurfaces) {
    Trk::Layer* mLayer = bsIter->surfaceRepresentation().materialLayer();
    if (mLayer && mLayer->layerIndex().value() < 0.) {
      Trk::LayerIndex layIndex = Trk::LayerIndex(
        int(geoSig) * TRKDETDESCR_GEOMETRYSIGNATUREWEIGHT + (++offset));
      mLayer->registerLayerIndex(layIndex);
    }
  }

  // step down the hierarchy to the contained volumes and index those
  // ------------------------
  if (confinedVolumes()) {
    Trk::BinnedArraySpan<Trk::TrackingVolume* const > volumes = confinedVolumes()->arrayObjects();
    for (const auto& volumesIter : volumes) {
      if (volumesIter)
        volumesIter->indexContainedStaticLayers(geoSig, offset);
    }
  }
}

void
Trk::TrackingVolume::indexContainedMaterialLayers(GeometrySignature geoSig,
                                                  int& offset)
{
  // the offset gets increased internally
  // the static layers first and check if they have surfaces with material
  // layers that need index
  if (m_confinedLayers) {
    Trk::BinnedArraySpan<Trk::Layer * const> layers = confinedLayers()->arrayObjects();
    for (Trk::Layer* layerIter : layers) {
      // only index the material layers & only those that have not yet been
      // singed
      if (layerIter) {
        Trk::SurfaceArray* surfArray = layerIter->surfaceArray();
        if (surfArray) {
          Trk::BinnedArraySpan<Trk::Surface * const> layerSurfaces = surfArray->arrayObjects();
          // loop over the surfaces - there can be 0 entries
          for (Trk::Surface* const laySurf : layerSurfaces) {
            Trk::Layer* materialLayer = laySurf ? laySurf->materialLayer() : nullptr;
            if (materialLayer && materialLayer->layerIndex().value() < 0) {
              // sign only those with material properties - rest goes to 0
              Trk::LayerIndex layIndex =
                materialLayer->layerMaterialProperties()
                  ? Trk::LayerIndex(int(geoSig) *
                                      TRKDETDESCR_GEOMETRYSIGNATUREWEIGHT +
                                    (++offset))
                  : Trk::LayerIndex(0);
              // now register the index
              materialLayer->registerLayerIndex(layIndex);
            }
          }
        }
      }
    }
  }

  // step down the hierarchy to the contained volumes and index those
  // ------------------------
  if (confinedVolumes()) {
    Trk::BinnedArraySpan<Trk::TrackingVolume * const> volumes = confinedVolumes()->arrayObjects();
    for (Trk::TrackingVolume* volumesIter : volumes) {
      if (volumesIter)
        volumesIter->indexContainedMaterialLayers(geoSig, offset);
    }
  }
}

void
Trk::TrackingVolume::addMaterial(const Trk::Material& mprop, float fact)
{
  // assume the scaling factor refers to the volume scaling
  float flin = pow(fact, 0.33);
  // average X0
  double invX0 = X0 > 0. ? 1. / X0 : 0.;
  double sum_invX0 = invX0 + flin / mprop.X0;
  X0 = 1. / sum_invX0;
  // average L0
  double invL0 = L0 > 0. ? 1. / L0 : 0.;
  double sum_invL0 = invL0 + flin / mprop.L0;
  L0 = 1. / sum_invL0;
  // add density
  float rho1 = rho;
  rho += fact * mprop.rho;
  // averageZ
  float n1 = Z > 0. ? rho1 / Z : 0.;
  float n2 = fact * mprop.rho / mprop.Z;
  Z = rho / (n1 + n2);
  // averageA
  n1 = A > 0. ? rho1 / A : 0.;
  n2 = fact * mprop.rho / mprop.A;
  A = rho / (n1 + n2);
  // zOverAtimesRho
  zOaTr = Z / A * rho;
  // mean energy loss (linear scaling)
  dEdX += flin * mprop.dEdX;
}

void
Trk::TrackingVolume::sign(Trk::GeometrySignature geosign,
                          Trk::GeometryType geotype)
{
  // never overwrite what is already signed, that's a crime
  if (m_geometrySignature == Trk::Unsigned) {
    m_geometrySignature = geosign;
  }
  m_geometryType = geotype;

  // confined volumes
  Trk::BinnedArray<Trk::TrackingVolume>* confVolumes = confinedVolumes();
  if (confVolumes) {
    Trk::BinnedArraySpan<Trk::TrackingVolume* const> volumes =
      confVolumes->arrayObjects();
    for (const auto& volumesIter : volumes)
      if (volumesIter)
        volumesIter->sign(geosign, geotype);
  }
  // same procedure for the detached volumes
  Trk::ArraySpan<Trk::DetachedTrackingVolume* const> confDetachedVolumes =
    confinedDetachedVolumes();
  if (!confDetachedVolumes.empty()) {
    for (const auto& volumesIter : confDetachedVolumes) {
      if (volumesIter) {
        volumesIter->sign(geosign, geotype);
      }
    }
  }
  // confined dense volumes
  Trk::ArraySpan<Trk::TrackingVolume* const> confDenseVolumes =
    confinedDenseVolumes();
  if (!confDenseVolumes.empty()) {
    for (const auto& volumesIter : confDenseVolumes) {
      if (volumesIter) {
        volumesIter->sign(geosign, geotype);
      }
    }
  }
}

std::vector<std::shared_ptr<Trk::BoundarySurface<Trk::TrackingVolume>>>&
Trk::TrackingVolume::boundarySurfaces()
{
  return m_boundarySurfaces;
}

Trk::ConstSharedPtrSpan<Trk::BoundarySurface<Trk::TrackingVolume>>
Trk::TrackingVolume::boundarySurfaces() const
{
  return Trk::ConstSharedPtrSpan<Trk::BoundarySurface<Trk::TrackingVolume>>(
    m_boundarySurfaces);
}

const Trk::BoundarySurface<Trk::TrackingVolume>*
Trk::TrackingVolume::boundarySurface(const ObjectAccessor::value_type& oa) const
{
  return (std::as_const(m_boundarySurfaces)[oa]).get();
}

void
Trk::TrackingVolume::createBoundarySurfaces()
{
  // prepare the BoundarySurfaces
  m_boundarySurfaces =
    std::vector<std::shared_ptr<Trk::BoundarySurface<Trk::TrackingVolume>>>();
  // transform Surfaces To BoundarySurfaces
  std::vector<std::unique_ptr<Trk::Surface>> surfaces =
    Trk::Volume::volumeBounds().decomposeToSurfaces(transform());
  auto surfIter = surfaces.begin();

  // counter to flip the inner/outer position for Cylinders
  unsigned int sfCounter = 0;
  unsigned int sfNumber = surfaces.size();

  // memory optimisation
  m_boundarySurfaces.reserve(sfNumber + 1);

  // identify Subtracted/CombinedVolumes
  const Trk::SubtractedVolumeBounds* subtrVol =
    dynamic_cast<const Trk::SubtractedVolumeBounds*>(
      &(Trk::Volume::volumeBounds()));
  const Trk::CombinedVolumeBounds* combVol =
    dynamic_cast<const Trk::CombinedVolumeBounds*>(
      &(Trk::Volume::volumeBounds()));
  bool subtr = (subtrVol) ? 1 : 0;
  bool comb = (combVol) ? 1 : 0;

  if (!subtr && !comb) {
    const Trk::SimplePolygonBrepVolumeBounds* spbVol =
      dynamic_cast<const Trk::SimplePolygonBrepVolumeBounds*>(
        &(Trk::Volume::volumeBounds()));

    for (; surfIter != surfaces.end(); ++surfIter) {
      sfCounter++;

      Trk::TrackingVolume* in = this;
      Trk::TrackingVolume* out = nullptr;

      // ST update: subtracted surfaces may appear in 'simple' volumes
      // (SimplePolygonBrep...)
      const Trk::SubtractedPlaneSurface* spsf =
        dynamic_cast<const Trk::SubtractedPlaneSurface*>((*surfIter).get());
      const Trk::PlaneSurface* psf =
        dynamic_cast<const Trk::PlaneSurface*>((*surfIter).get());
      if (spsf) {
        if (spbVol && sfCounter == 1) {
          in = nullptr;
          out = this;
        }
        m_boundarySurfaces.push_back(
          std::make_shared<Trk::BoundarySubtractedPlaneSurface<Trk::TrackingVolume>>(
              in, out, *spsf));
        continue;
      }
      if (psf) {
        m_boundarySurfaces.push_back(
          std::make_shared<Trk::BoundaryPlaneSurface<Trk::TrackingVolume>>(in, out, *psf));
        continue;
      }

      const Trk::DiscSurface* dsf =
        dynamic_cast<const Trk::DiscSurface*>((*surfIter).get());
      if (dsf) {
        m_boundarySurfaces.push_back(
          std::make_shared<Trk::BoundaryDiscSurface<Trk::TrackingVolume>>(in, out, *dsf));
        continue;
      }

      const Trk::SubtractedCylinderSurface* scsf =
        dynamic_cast<const Trk::SubtractedCylinderSurface*>((*surfIter).get());
      const Trk::CylinderSurface* csf =
        dynamic_cast<const Trk::CylinderSurface*>((*surfIter).get());
      if (scsf) {
        Trk::TrackingVolume* inner =
          (sfCounter == 4 && sfNumber > 3) ? nullptr : this;
        Trk::TrackingVolume* outer = (inner) ? nullptr : this;
        m_boundarySurfaces.push_back(
          std::make_shared<Trk::BoundarySubtractedCylinderSurface<Trk::TrackingVolume>>(inner, outer, *scsf));
        continue;
      }
      if (csf) {
        Trk::TrackingVolume* inner =
          (sfCounter == 4 && sfNumber > 3) ? nullptr : this;
        Trk::TrackingVolume* outer = (inner) ? nullptr : this;
        m_boundarySurfaces.push_back(
          std::make_shared<Trk::BoundaryCylinderSurface<Trk::TrackingVolume>>(
              inner, outer, *csf));
        continue;
      }
    }

  } else {
    const std::vector<bool> bOrient =
      subtrVol ? subtrVol->boundsOrientation() : combVol->boundsOrientation();

    for (; surfIter != surfaces.end(); ++surfIter) {
      Trk::TrackingVolume* in = bOrient[sfCounter] ? this : nullptr;
      Trk::TrackingVolume* out = bOrient[sfCounter] ? nullptr : this;
      sfCounter++;

      const Trk::SubtractedPlaneSurface* psf =
        dynamic_cast<const Trk::SubtractedPlaneSurface*>((*surfIter).get());
      if (psf) {
        m_boundarySurfaces.push_back(
          std::make_shared<Trk::BoundarySubtractedPlaneSurface<Trk::TrackingVolume>>(
              in, out, *psf));
        continue;
      }

      const Trk::SubtractedCylinderSurface* csf =
        dynamic_cast<const Trk::SubtractedCylinderSurface*>((*surfIter).get());
      if (csf) {
        m_boundarySurfaces.push_back(
          std::make_shared<Trk::BoundarySubtractedCylinderSurface<Trk::TrackingVolume>>(
              in, out, *csf));
        continue;
      }
    }
  }
}

void
Trk::TrackingVolume::createLayerAttemptsCalculator()
{
  // check whether there's a static array
  if (m_confinedLayers) {
    // BinUtility
    const Trk::BinUtility* binUtility = m_confinedLayers->binUtility();
    if (binUtility) {
      if (binUtility->binningValue() == Trk::binR)
        m_layerAttemptsCalculator =
          new Trk::CylinderLayerAttemptsCalculator(1, 5);
      if (binUtility->binningValue() == Trk::binZ)
        m_layerAttemptsCalculator = new Trk::DiscLayerAttemptsCalculator(1, 5);
    }
  }
}

void Trk::TrackingVolume::interlinkLayers()
{
  if (m_confinedLayers) {
    BinnedArraySpan<Trk::Layer* const> layers = m_confinedLayers->arrayObjects();
    // forward loop
    const Trk::Layer* lastLayer = nullptr;
    BinnedArraySpan<Trk::Layer* const>::iterator layerIter = layers.begin();
    for (; layerIter != layers.end(); ++layerIter) {
      if (*layerIter) {
        // register the layers
        (**layerIter).setBinUtility(confinedLayers()->binUtility()
                                    ? confinedLayers()->binUtility()
                                    : nullptr);
        (**layerIter).setPreviousLayer(lastLayer);
        // register the volume
        (**layerIter).encloseTrackingVolume(*this);
      }
      lastLayer = (*layerIter);
    }
    // backward loop
    lastLayer = nullptr;
    layerIter = layers.end();
    --layerIter;
    for (;; --layerIter) {
      if (*layerIter) {
        (**layerIter).setNextLayer(lastLayer);
      }
      lastLayer = (*layerIter);
      if (layerIter == layers.begin()) {
        break;
      }
    }
  }
}

const Trk::LayerArray*
Trk::TrackingVolume::checkoutConfinedLayers() const
{
  const Trk::LayerArray* checkoutLayers = m_confinedLayers;
  return checkoutLayers;
}

void
Trk::TrackingVolume::registerOutsideGlueVolumes(Trk::GlueVolumesDescriptor* gvd)
{
  m_outsideGlueVolumes.store(std::unique_ptr<Trk::GlueVolumesDescriptor>(gvd));
}

Trk::GlueVolumesDescriptor&
Trk::TrackingVolume::glueVolumesDescriptor()
{
  if (!m_outsideGlueVolumes) {
    m_outsideGlueVolumes.store(std::make_unique<Trk::GlueVolumesDescriptor>());
  }
  return (*m_outsideGlueVolumes);
}

const Trk::GlueVolumesDescriptor&
Trk::TrackingVolume::glueVolumesDescriptor() const
{
  if (!m_outsideGlueVolumes) {
    m_outsideGlueVolumes.set(std::make_unique<Trk::GlueVolumesDescriptor>());
  }
  return (*m_outsideGlueVolumes);
}

void
Trk::TrackingVolume::moveVolume(Amg::Transform3D& shift)
{
  if (m_transform) {
    Amg::Transform3D transf = shift * (*m_transform);
    this->m_transform = std::make_unique<Amg::Transform3D>(transf);
  } else {
    this->m_transform = std::make_unique<Amg::Transform3D>(shift);
  }
  this->m_center.store(
    std::make_unique<Amg::Vector3D>(m_transform->translation()));
}

Trk::TrackingVolume* Trk::TrackingVolume::cloneTV (Amg::Transform3D& transform) const
{
  // clone the mother volume
  Trk::Volume* vol = Trk::Volume::clone();

  // clone 'ordered layers
  Trk::LayerArray* layerArray = nullptr;
  // confined layers
  const Trk::BinnedArray<Trk::Layer>* confLayers = confinedLayers();
  if (confLayers) {
    // retrieve array objects and apply the transform
    Trk::BinnedArraySpan<Trk::Layer const * const> layers = confLayers->arrayObjects();
    std::vector<std::shared_ptr<Trk::Layer>> layerOrder;
    for (const auto *layer : layers) {
      const Trk::PlaneLayer* lay =
        dynamic_cast<const Trk::PlaneLayer*>(layer);
      if (lay) {
        Trk::PlaneLayer* newlay = new Trk::PlaneLayer(*lay, transform);
        layerOrder.push_back(std::shared_ptr<Trk::Layer>(newlay));
      }
    }
    // recreate LayerArray
    const Trk::NavBinnedArray1D<Trk::Layer>* confLaysNav =
      dynamic_cast<const Trk::NavBinnedArray1D<Trk::Layer>*>(confLayers);
    if (confLaysNav)
      layerArray = new Trk::NavBinnedArray1D<Trk::Layer>(
        *confLaysNav,
        std::vector<std::shared_ptr<Trk::Layer>>(layerOrder),
        transform);
  }

  // clone 'unordered' layers
  std::vector<Trk::Layer*>* unorderedLayers = nullptr;
  Trk::ArraySpan<const Trk::Layer* const> confArbLayers =
    confinedArbitraryLayers();
  if (!confArbLayers.empty()) {
    // clone & apply the transform
    std::vector<Trk::Layer*> uLayers;
    for (const auto *confArbLayer : confArbLayers) {
      const Trk::SubtractedPlaneLayer* slayer =
        dynamic_cast<const Trk::SubtractedPlaneLayer*>(confArbLayer);
      const Trk::SubtractedCylinderLayer* sclayer =
        dynamic_cast<const Trk::SubtractedCylinderLayer*>(confArbLayer);
      const Trk::PlaneLayer* layer =
        dynamic_cast<const Trk::PlaneLayer*>(confArbLayer);
      const Trk::CylinderLayer* clayer =
        dynamic_cast<const Trk::CylinderLayer*>(confArbLayer);

      if (slayer) {
        Trk::SubtractedPlaneLayer* lay = new Trk::SubtractedPlaneLayer(*slayer);
        lay->moveLayer(transform);
        uLayers.push_back(lay);
      } else if (layer) {
        Trk::PlaneLayer* lay = new Trk::PlaneLayer(*layer);
        lay->moveLayer(transform);
        uLayers.push_back(lay);
      } else if (sclayer) {
        Trk::SubtractedCylinderLayer* lay =
          new Trk::SubtractedCylinderLayer(*sclayer);
        lay->moveLayer(transform);
        uLayers.push_back(lay);
      } else if (clayer) {
        Trk::CylinderLayer* lay = new Trk::CylinderLayer(*clayer);
        lay->moveLayer(transform);
        uLayers.push_back(lay);
      }
    }
    unorderedLayers = new std::vector<Trk::Layer*>(uLayers);
  }

  // cloning confined volumes
  Trk::TrackingVolumeArray* volumeArray = nullptr;
  const Trk::BinnedArray<Trk::TrackingVolume>* confVolumes = confinedVolumes();
  if (confVolumes) {
    // retrieve array objects and apply the transform
    Trk::BinnedArraySpan<Trk::TrackingVolume const* const> volumes = confVolumes->arrayObjects();
    std::vector<std::shared_ptr<TrackingVolume>> volOrder;
    for (const auto *volume : volumes) {
      Trk::TrackingVolume* vol = volume->cloneTV(transform);
      volOrder.push_back(std::shared_ptr<TrackingVolume>(vol));
    }
    // recreate TrackingVolumeArray
    const Trk::NavBinnedArray1D<Trk::TrackingVolume>* confVolsNav =
      dynamic_cast<const Trk::NavBinnedArray1D<Trk::TrackingVolume>*>(
        confVolumes);
    if (confVolsNav)
      volumeArray = new Trk::NavBinnedArray1D<Trk::TrackingVolume>(
        *confVolsNav, std::vector<std::shared_ptr<TrackingVolume>>(volOrder), transform);
  }

  // cloning confined unordered volumes
  Trk::ArraySpan<const Trk::TrackingVolume* const> confDenseVolumes =
    confinedDenseVolumes();
  std::vector<Trk::TrackingVolume*>* newDenseVol = nullptr;
  if (!confDenseVolumes.empty()) {
    std::vector<Trk::TrackingVolume*> newVol;
    // retrieve array objects and apply the transform
    for (const auto *confDenseVolume : confDenseVolumes) {
      Trk::TrackingVolume* vol =
        confDenseVolume->cloneTV(transform);
      newVol.push_back(vol);
    }
    newDenseVol = new std::vector<Trk::TrackingVolume*>(newVol);
  }

  // create the Tracking Volume
  Trk::TrackingVolume* newTrkVol = nullptr;
  if (!confArbLayers.empty() || !confDenseVolumes.empty()) {
    if (!confArbLayers.empty() && !confDenseVolumes.empty()) {
      newTrkVol = new Trk::TrackingVolume(
        *vol, unorderedLayers, newDenseVol, *this, volumeName());
    } else if (!confArbLayers.empty()) {
      newTrkVol =
        new Trk::TrackingVolume(*vol, *this, unorderedLayers, volumeName());
    } else {
      newTrkVol =
        new Trk::TrackingVolume(*vol, *this, newDenseVol, volumeName());
    }
    delete layerArray;
  } else {
    newTrkVol = new Trk::TrackingVolume(
      *vol, *this, layerArray, volumeArray, volumeName());
  }
  delete vol;
  // finally, position the mother volume
  newTrkVol->moveVolume(transform);
  // create boundary surfaces
  newTrkVol->m_boundarySurfaces.clear();
  newTrkVol->createBoundarySurfaces();

  delete volumeArray;
  return newTrkVol;
}

const Trk::Layer*
Trk::TrackingVolume::closest(const Amg::Vector3D& pos,
                             const Amg::Vector3D& dir,
                             const Trk::Layer& first,
                             const Trk::Layer& second)
{
  // use the distance solution for assigning the layer
  Trk::DistanceSolution distSolToPrevious =
    first.surfaceRepresentation().straightLineDistanceEstimate(pos, dir);
  Trk::DistanceSolution distSolToNext =
    second.surfaceRepresentation().straightLineDistanceEstimate(pos, dir);
  // find out which one
  return (distSolToPrevious.absClosest() < distSolToNext.absClosest()
            ? &first
            : &second);
}

void
Trk::TrackingVolume::moveTV(Amg::Transform3D& transform)
{

  this->moveVolume(transform);

  // confined 'ordered' layers
  Trk::BinnedArray<Trk::Layer>* confLayers = confinedLayers();
  if (confLayers)
    for (Trk::Layer* clayIter : confLayers->arrayObjects()){
      clayIter->moveLayer(transform);
    }

  // confined 'unordered' layers
  Trk::ArraySpan<Trk::Layer* const> confArbLayers =
    confinedArbitraryLayers();
  if (!confArbLayers.empty()){
    for (Trk::Layer* calayIter : confArbLayers){
      calayIter->moveLayer(transform);
    }
  }

  // confined volumes
  Trk::BinnedArray<Trk::TrackingVolume>* confVolumes = confinedVolumes();
  if (confVolumes)
    // retrieve array objects and apply the transform
    for (Trk::TrackingVolume* cVolumesIter : confVolumes->arrayObjects()){
      (cVolumesIter)->moveTV(transform);
    }

  // confined unordered volumes
  Trk::ArraySpan<Trk::TrackingVolume* const> confDenseVolumes =
    confinedDenseVolumes();
  if (!confDenseVolumes.empty())
    // retrieve array objects and apply the transform
    for (Trk::TrackingVolume* cVolumesIter : confDenseVolumes){
      cVolumesIter->moveTV(transform);
    }
}

void
Trk::TrackingVolume::synchronizeLayers(MsgStream& msgstream, double envelope)
{
  // case a : Layers exist
  Trk::BinnedArray<Trk::Layer>* confLayers = confinedLayers();
  if (confLayers) {
    for (Trk::Layer* clay : confLayers->arrayObjects())
      if (clay) {
        if (clay->surfaceRepresentation().type() ==
              Trk::SurfaceType::Cylinder &&
            !(center().isApprox(clay->surfaceRepresentation().center()))) {
          clay->resizeAndRepositionLayer(volumeBounds(), center(), envelope);
        } else {
          clay->resizeLayer(volumeBounds(), envelope);
        }
      } else
        msgstream << MSG::WARNING
                  << "  ---> found 0 pointer to layer in Volume [ "
                  << volumeName() << " ], indicates problem." << endmsg;
  }
  // case b : container volume -> step down
  Trk::BinnedArray<Trk::TrackingVolume>* confVolumes = confinedVolumes();
  if (confVolumes) {
    for (const auto& cVolumesIter : confVolumes->arrayObjects())
      cVolumesIter->synchronizeLayers(msgstream, envelope);
  }
}

void
Trk::TrackingVolume::compactify(size_t& cSurfaces, size_t& tSurfaces)
{
  // confined 'ordered' layers
  Trk::BinnedArray<Trk::Layer>* confLayers = confinedLayers();
  if (confLayers) {
    Trk::BinnedArraySpan<Trk::Layer* const> layers = confLayers->arrayObjects();
    for (const auto& clayIter : layers) {
      if (&(*clayIter) != nullptr)
        clayIter->compactify(cSurfaces, tSurfaces);
      else
        std::cout << "WARNING: Attempt to compactify nullptr layer in volume : "
                  << volumeName() << std::endl;
    }
  }
  // confined 'unordered' layers
  Trk::ArraySpan<Trk::Layer* const> confArbLayers = confinedArbitraryLayers();
  if (!confArbLayers.empty()) {
    for (const auto& calayIter : confArbLayers) {
      if (calayIter != nullptr) {
        calayIter->compactify(cSurfaces, tSurfaces);
      } else {
        std::cout << "WARNING: Attempt to compactify nullptr layer."
                  << std::endl;
      }
    }
  }
  // confined volumes
  Trk::BinnedArray<Trk::TrackingVolume>* confVolumes = confinedVolumes();
  if (confVolumes) {
    Trk::BinnedArraySpan<Trk::TrackingVolume* const> volumes =
      confVolumes->arrayObjects();
    for (const auto& cVolumesIter : volumes) {
      cVolumesIter->compactify(cSurfaces, tSurfaces);
    }
  }
  // confined unordered volumes
  Trk::ArraySpan<Trk::TrackingVolume* const> confDenseVolumes =
    confinedDenseVolumes();
  if (!confDenseVolumes.empty())
    for (const auto& cVolumesIter : (confDenseVolumes)) {
      cVolumesIter->compactify(cSurfaces, tSurfaces);
    }

  // detached volumes if any
  if (m_confinedDetachedVolumes)
    for (const auto& cdVolumesIter : (*m_confinedDetachedVolumes)) {
      cdVolumesIter->compactify(cSurfaces, tSurfaces);
    }
}

void
Trk::TrackingVolume::screenDump(MsgStream& msg) const
{
  msg << "[[ Trk::TrackingVolume ]] called: " << volumeName() << std::endl;
  msg << '\t' << '\t' << "# position (x,y,z) : " << center().x() << ", "
      << center().y() << ", " << center().z() << std::endl;
  msg << '\t' << '\t' << "# bounds           : " << volumeBounds() << endmsg;
}

