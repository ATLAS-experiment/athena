/*
  Copyright (C) 2002-2023  CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingGeometry.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometry/TrackingGeometry.h"

#include "TrkGeometry/DetachedTrackingVolume.h"
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkVolumes/VolumeBounds.h"
// GaudiKernel
#include "GaudiKernel/MsgStream.h"



Trk::TrackingGeometry::TrackingGeometry(TrackingVolume* highestVolume,
                                        NavigationLevel navLev):
   m_world(highestVolume),
  m_navigationLevel(navLev) {
  // for the time being only world
  if (!m_world) return;
  registerTrackingVolumes(*m_world);
}

Trk::TrackingGeometry::~TrackingGeometry() {
    if (m_world) delete m_world;
}
void Trk::TrackingGeometry::addToGarbage(std::vector<std::unique_ptr<DetachedTrackingVolume>>&& garbageVec) {
  std::copy_if(std::make_move_iterator(garbageVec.begin()), std::make_move_iterator(garbageVec.end()),
               std::back_inserter(m_detachedVolGarbage),
               [](const std::unique_ptr<DetachedTrackingVolume>& ptr){
                  return ptr.get () != nullptr;
               });
  
}
void Trk::TrackingGeometry::addToGarbage(std::vector<std::unique_ptr<TrackingVolume>>&& garbageVec){
      std::copy_if(std::make_move_iterator(garbageVec.begin()), std::make_move_iterator(garbageVec.end()),
                  std::back_inserter(m_trkVolumeGarbage),
                  [](const std::unique_ptr<TrackingVolume>& ptr){
                      return ptr.get () != nullptr;
                  });
}


const Trk::TrackingVolume*
Trk::TrackingGeometry::lowestTrackingVolume(const Amg::Vector3D& gp) const
{
  const Trk::TrackingVolume* searchVolume = highestTrackingVolume();
  const Trk::TrackingVolume* currentVolume = nullptr;
  while (currentVolume != searchVolume && searchVolume) {
    currentVolume = searchVolume;
    searchVolume = searchVolume->associatedSubVolume(gp);
  }
  return (currentVolume);
}

std::vector<const Trk::DetachedTrackingVolume*>
Trk::TrackingGeometry::lowestDetachedTrackingVolumes(
  const Amg::Vector3D& gp) const
{
  double tol = 0.001;
  const Trk::TrackingVolume* currentVolume = lowestStaticTrackingVolume(gp);
  if (currentVolume){
    return currentVolume->assocDetachedSubVolumes(gp, tol);
  }
  return {};
}

const Trk::TrackingVolume*
Trk::TrackingGeometry::lowestStaticTrackingVolume(const Amg::Vector3D& gp) const
{
  const Trk::TrackingVolume* searchVolume = highestTrackingVolume();
  const Trk::TrackingVolume* currentVolume = nullptr;
  while (currentVolume != searchVolume && searchVolume) {
    currentVolume = searchVolume;
    if (searchVolume->confinedDetachedVolumes().empty())
      searchVolume = searchVolume->associatedSubVolume(gp);
  }
  return (currentVolume);
}

void
Trk::TrackingGeometry::registerTrackingVolumes(Trk::TrackingVolume& tvol,
                                               Trk::TrackingVolume* mvol,
                                               int lvl)
{
  int sublvl = lvl + 1;
  std::string indent = "";
  for (int l = 0; l < lvl; ++l, indent += "  ")
    ;

  tvol.setMotherVolume(mvol);
  m_trackingVolumes[tvol.volumeName()] = (&tvol);
  Trk::BinnedArray<Trk::TrackingVolume>* confinedVolumes =
    tvol.confinedVolumes();
  if (confinedVolumes) {
    Trk::BinnedArraySpan<Trk::TrackingVolume* const> volumes =
      confinedVolumes->arrayObjects();
    for (const auto& volumesIter : volumes)
      if (volumesIter)
        registerTrackingVolumes(*volumesIter, &tvol, sublvl);
  }

  Trk::ArraySpan<Trk::TrackingVolume* const> confinedDenseVolumes =
    tvol.confinedDenseVolumes();
  if (!confinedDenseVolumes.empty()) {
    for (const auto& volumesIter : confinedDenseVolumes)
      if (volumesIter)
        registerTrackingVolumes(*volumesIter, &tvol, sublvl);
  }
  /** should detached tracking volumes be part of the tracking geometry ? */
  Trk::ArraySpan<Trk::DetachedTrackingVolume* const> confinedDetachedVolumes =
    tvol.confinedDetachedVolumes();
  if (!confinedDetachedVolumes.empty()) {
    for (const auto& volumesIter : confinedDetachedVolumes)
      if (volumesIter &&
          tvol.inside(volumesIter->trackingVolume()->center(), 0.))
        registerTrackingVolumes(
          *(volumesIter->trackingVolume()), &tvol, sublvl);
  }
  /** register the boundary layers */
  // boundary layers
  const auto& bounds = tvol.boundarySurfaces();
  for (const auto & bound : bounds) {
    Trk::Layer* bLayer =
      bound->surfaceRepresentation().materialLayer();
    if (bLayer) {
      int& layerCount{m_boundaryLayers[bLayer]};
      ++layerCount;
    }
  }
}

void Trk::TrackingGeometry::compactify(MsgStream& msg, TrackingVolume* vol)
{
  msg << MSG::VERBOSE
      << "====== Calling TrackingGeometry::compactify() ===== " << std::endl;
  Trk::TrackingVolume* tVolume = vol ? vol : highestTrackingVolume();
  size_t cSurfaces = 0;
  size_t tSurfaces = 0;
  if (tVolume) {
    tVolume->compactify(cSurfaces, tSurfaces);
  }
  msg << MSG::VERBOSE << "  --> set TG ownership of " << cSurfaces << " out of "
      << tSurfaces << std::endl;
  for (const auto& bLayerIter : m_boundaryLayers) {
    bLayerIter.first->surfaceRepresentation().setOwner(Trk::TGOwn);
  }
  msg << MSG::VERBOSE << "  --> set TG ownership of " << m_boundaryLayers.size()
      << " boundary layers." << std::endl;
  msg << MSG::VERBOSE << endmsg;
}

void
Trk::TrackingGeometry::synchronizeLayers(MsgStream& msg, TrackingVolume* vol)
{
  Trk::TrackingVolume* tVolume = vol ? vol : highestTrackingVolume();
  tVolume->synchronizeLayers(msg);
}

Trk::TrackingVolume*
Trk::TrackingGeometry::checkoutHighestTrackingVolume() {
  Trk::TrackingVolume* checkoutVolume{nullptr};
  std::swap(m_world, checkoutVolume);
  // clear the boundary layers they go with the highest volume
  m_boundaryLayers.clear();
  return checkoutVolume;
}

void
Trk::TrackingGeometry::printVolumeHierarchy(MsgStream& msg) const
{
  msg << "TrackingGeometry Summary : " << std::endl;
  const Trk::TrackingVolume* highestVolume = highestTrackingVolume();
  int level = 0;
  if (highestVolume)
    printVolumeInformation(msg, *highestVolume, level);
  msg << endmsg;
}

void
Trk::TrackingGeometry::printVolumeInformation(MsgStream& msg,
                                              const Trk::TrackingVolume& tvol,
                                              int lvl) const
{
  int sublevel = lvl + 1;

  for (int indent = 0; indent < sublevel; ++indent)
    msg << "  ";
  msg << "TrackingVolume ( at : " << (&tvol) << ") name: " << tvol.volumeName()
      << std::endl;

  const Trk::BinnedArray<Trk::Layer>* confinedLayers = tvol.confinedLayers();
  if (confinedLayers) {
    Trk::BinnedArraySpan<Trk::Layer const* const> layers =
      confinedLayers->arrayObjects();
    for (int indent = 0; indent < sublevel; ++indent)
      msg << "  ";
    msg << "- found : " << layers.size() << " confined Layers" << std::endl;
  }

  const Trk::BinnedArray<Trk::TrackingVolume>* confinedVolumes =
    tvol.confinedVolumes();
  if (confinedVolumes) {
    Trk::BinnedArraySpan<Trk::TrackingVolume const* const> volumes =
      confinedVolumes->arrayObjects();

    for (int indent = 0; indent < sublevel; ++indent)
      msg << "  ";
    msg << "- found : " << volumes.size() << " confined TrackingVolumes"
        << std::endl;

    for (const auto& volumesIter : volumes)
      if (volumesIter)
        printVolumeInformation(msg, *volumesIter, sublevel);
  }

  Trk::ArraySpan<const Trk::TrackingVolume* const> confinedDenseVolumes =
    tvol.confinedDenseVolumes();
  if (!confinedDenseVolumes.empty()) {
    for (int indent = 0; indent < sublevel; ++indent) {
      msg << "  ";
    }
    msg << "- found : " << confinedDenseVolumes.size()
        << " confined unordered (dense) TrackingVolumes" << std::endl;

    for (const auto& volumesIter : (confinedDenseVolumes))
      if (volumesIter) {
        printVolumeInformation(msg, *volumesIter, sublevel);
      }
  }
}

void Trk::TrackingGeometry::indexStaticLayers(GeometrySignature geosit, int offset)
{
  if (m_world) {
    m_world->indexContainedStaticLayers(geosit, offset);
    m_world->indexContainedMaterialLayers(geosit, offset);
  }
}

bool
Trk::TrackingGeometry::atVolumeBoundary(const Amg::Vector3D& gp,
                                        const Trk::TrackingVolume* vol,
                                        double tol)
{
  bool isAtBoundary = false;
  if (!vol)
    return isAtBoundary;
  const auto& bounds = vol->boundarySurfaces();
  for (size_t ib = 0; ib < bounds.size(); ++ib) {
    const Trk::Surface& surf = bounds[ib]->surfaceRepresentation();
    if (surf.isOnSurface(gp, true, tol, tol))
      isAtBoundary = true;
  }
  return isAtBoundary;
}

/** check position at volume boundary + navigation link */
bool
Trk::TrackingGeometry::atVolumeBoundary(const Amg::Vector3D& gp,
                                        const Amg::Vector3D& mom,
                                        const TrackingVolume* vol,
                                        const TrackingVolume*& nextVol,
                                        Trk::PropDirection dir,
                                        double tol)
{
  bool isAtBoundary = false;
  nextVol = nullptr;
  if (!vol)
    return isAtBoundary;
  const auto& bounds = vol->boundarySurfaces();
  for (size_t ib = 0; ib < bounds.size(); ++ib) {
    const Trk::Surface& surf = bounds[ib]->surfaceRepresentation();
    if (surf.isOnSurface(gp, true, tol, tol)) {
      isAtBoundary = true;
      const Trk::TrackingVolume* attachedVol =
        bounds[ib]->attachedVolume(gp, mom, dir);
      if (!nextVol && attachedVol)
        nextVol = attachedVol;
    }
  }
  return isAtBoundary;
}

void
Trk::TrackingGeometry::dump(MsgStream& out, const std::string& head) const
{
  out << MSG::ALWAYS;
  for (const auto& bound_layers : m_boundaryLayers) {
    out << head << " [" << bound_layers.second << "] ";
    dumpLayer(out, "", bound_layers.first);
  }
  int counter = 0;
  for (const std::pair<const std::string, const TrackingVolume*>& volume :
       m_trackingVolumes) {
    out << head << " [" << counter++ << "] " << volume.first << " volumeBound=";
    volume.second->volumeBounds().dump(out);
    out << std::endl;
    Trk::ArraySpan<const Trk::Layer* const> confArbLayers =
      volume.second->confinedArbitraryLayers();
    if (!confArbLayers.empty()) {
      int j = 0;
      for (const Layer* confined_layer : confArbLayers) {
        out << head << " [" << counter++ << "] " << volume.first
            << " confinedArbitrary layer " << j++ << " ";
        dumpLayer(out, "", confined_layer);
      }
    }
    if (volume.second->confinedLayers()) {
      int j = 0;
      for (const Layer* confined_layer :
           volume.second->confinedLayers()->arrayObjects()) {
        out << head << " [" << counter++ << "] " << volume.first
            << " confined layer" << j++ << " ";
        dumpLayer(out, "", confined_layer);
      }
    }
  }
  out << endmsg;
}
void
Trk::TrackingGeometry::dumpLayer(MsgStream& out,
                                 const std::string& head,
                                 const Layer* layer)
{
  if (!layer) {
    return;
  }
  out << head << layer->layerIndex().value() << " [t=" << layer->layerType()
      << "] d=" << layer->thickness();
  out << layer->surfaceRepresentation();
  out << std::endl;
}
