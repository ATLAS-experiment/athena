/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DetachedTrackingVolume.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometry/DetachedTrackingVolume.h"

#include <utility>

#include "TrkGeometry/Layer.h"
#include "TrkGeometry/TrackingVolume.h"

Trk::DetachedTrackingVolume::DetachedTrackingVolume(std::string name,
                                                    std::unique_ptr<Trk::TrackingVolume> volume)
  : m_trkVolume(std::move(volume))
  , m_name(std::move(name))
{}

Trk::DetachedTrackingVolume::DetachedTrackingVolume(
  std::string name,
  std::unique_ptr<Trk::TrackingVolume> volume,
  std::unique_ptr<Trk::Layer> lay,
  std::unique_ptr<const std::vector<Trk::Layer*>> multilay)
  : m_trkVolume(std::move(volume))
  , m_layerRepresentation(std::move(lay))
  , m_multilayerRepresentation(std::move(multilay))
  , m_name(std::move(name))
  , m_baseTransform(nullptr)
{}

Trk::DetachedTrackingVolume::~DetachedTrackingVolume() {
  //These we have to delete manually
  if (m_multilayerRepresentation) {
    for (auto *layer : *m_multilayerRepresentation)
      delete layer;
  }
}

void
Trk::DetachedTrackingVolume::move(Amg::Transform3D& shift)
{
  m_trkVolume->moveTV(shift);
  if (m_layerRepresentation) {
    m_layerRepresentation->moveLayer(shift);
  }
  if (m_multilayerRepresentation) {
    for (auto* layer : *m_multilayerRepresentation) {
      layer->moveLayer(shift);
    }
  }
}

Trk::DetachedTrackingVolume*
Trk::DetachedTrackingVolume::clone(const std::string& name,
                                   Amg::Transform3D& shift) const
{
  auto newTV = std::make_unique<TrackingVolume>(*(this->trackingVolume()), shift);
  TrackingVolumeArray* confinedVolumes = newTV->confinedVolumes();
  LayerArray* confinedLayers= newTV->confinedLayers();
  Trk::ArraySpan<Trk::Layer* const> confinedArbitraryLayers = newTV->confinedArbitraryLayers();
  //
  Trk::DetachedTrackingVolume* newStat = nullptr;
  // layer representation ?
  std::unique_ptr<Trk::PlaneLayer> newLay = nullptr;
  if (this->layerRepresentation()) {
    std::unique_ptr<std::vector<Trk::Layer*>> newMulti = nullptr;
    const Trk::PlaneLayer* pl = dynamic_cast<const Trk::PlaneLayer*>(this->layerRepresentation());
    if (pl) {
      newLay = std::make_unique<Trk::PlaneLayer>(*pl);
      newLay->moveLayer(shift);
      if (!this->multilayerRepresentation().empty()) {
        newMulti = std::make_unique<std::vector<Trk::Layer*>>();
        for (unsigned int i = 0; i < this->multilayerRepresentation().size(); i++) {
          const Trk::PlaneLayer* mpl = dynamic_cast<const Trk::PlaneLayer*>((this->multilayerRepresentation())[i]);
          if (mpl) {
            Trk::PlaneLayer* newPl = new Trk::PlaneLayer(*mpl);
            newPl->moveLayer(shift);
            newMulti->push_back(newPl);
          } else
            std::cout << "WARNING   Trk::DetachedTrackingVolume::clone()   "
                         "dynamic cast to 'const Trk::PlaneLayer* mpl' failed!"
                      << std::endl;
        }
      }
      newStat = new Trk::DetachedTrackingVolume(name, std::move(newTV), std::move(newLay), std::move(newMulti));
    } else {
      std::cout << "WARNING   Trk::DetachedTrackingVolume::clone()   dynamic "
                   "cast to 'const Trk::PlaneLayer* pl' failed!"
                << std::endl;
      newStat = new Trk::DetachedTrackingVolume(name, std::move(newTV));
    }
  } else {
    newStat = new Trk::DetachedTrackingVolume(name, std::move(newTV));
  }
  //
  // enclose layers
  if (confinedVolumes) {
    std::span<Trk::TrackingVolume * const> vols = confinedVolumes->arrayObjects();
    for (auto *vol : vols) {
      Trk::LayerArray* layAr = vol->confinedLayers();
      Trk::ArraySpan<Trk::Layer* const> alays = vol->confinedArbitraryLayers();
      if (layAr) {
        std::span<Trk::Layer* const> lays = layAr->arrayObjects();
        for (auto *lay : lays) {
          lay->encloseDetachedTrackingVolume(*newStat);
        }
      }
      if (!alays.empty()) {
        for (auto *alay : alays) {
          alay->encloseDetachedTrackingVolume(*newStat);
        }
      }
    }
  }
  if (confinedLayers) {
    std::span<Trk::Layer* const> lays = confinedLayers->arrayObjects();
    for (auto *lay : lays){
      lay->encloseDetachedTrackingVolume(*newStat);
    }
  }
  if (!confinedArbitraryLayers.empty()) {
    for (auto *alay : confinedArbitraryLayers) {
      alay->encloseDetachedTrackingVolume(*newStat);
    }
  }
  return newStat;
}

void
Trk::DetachedTrackingVolume::compactify(size_t& cSurfaces, size_t& tSurfaces)
{
  // deal with the Tracking Volume representation
  if (m_trkVolume){
    m_trkVolume->compactify(cSurfaces, tSurfaces);
  }

  // deal with the layer representation
  if (layerRepresentation()) {
    ++tSurfaces;
    if (layerRepresentation()->surfaceRepresentation().owner() == Trk::noOwn) {
      layerRepresentation()->surfaceRepresentation().setOwner(Trk::TGOwn);
      ++cSurfaces;
    }
  }
  // deal with the multi-layer representation
  if (!multilayerRepresentation().empty()) {
    tSurfaces += m_multilayerRepresentation->size();
    for (const auto& mLayerIter : (*m_multilayerRepresentation)) {
      if ((*mLayerIter).surfaceRepresentation().owner() == Trk::noOwn) {
        (*mLayerIter).surfaceRepresentation().setOwner(Trk::TGOwn);
        ++cSurfaces;
      }
    }
  }
}

void
Trk::DetachedTrackingVolume::sign(GeometrySignature signat,
                                  GeometryType geotype)
{
  m_trkVolume->sign(signat, geotype);
}

Trk::GeometrySignature Trk::DetachedTrackingVolume::geometrySignature() const {
  return m_trkVolume->geometrySignature();
}

Trk::GeometryType Trk::DetachedTrackingVolume::geometryType() const {
  return m_trkVolume->geometryType();
}

void Trk::DetachedTrackingVolume::setBaseTransform(std::unique_ptr<Amg::Transform3D> transf) {
  if (transf){
    m_baseTransform = std::move(transf);
  }
  else {
    m_baseTransform = std::make_unique<Amg::Transform3D>(this->trackingVolume()->transform());
  }
}

