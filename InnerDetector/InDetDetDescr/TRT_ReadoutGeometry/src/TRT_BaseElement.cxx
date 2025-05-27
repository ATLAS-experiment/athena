/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "TRT_ReadoutGeometry/TRT_Conditions.h"

#include "GeoModelUtilities/GeoAlignmentStore.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"

#include "InDetIdentifier/TRT_ID.h"

#include <vector>

namespace InDetDD {

TRT_BaseElement::TRT_BaseElement(const GeoVFullPhysVol* volume,
                                 const Identifier& id,
                                 const TRT_ID* idHelper,
                                 const TRT_Conditions* conditions)
  : Trk::TrkDetElementBase(volume)
  , m_id(id)
  , m_idHelper(idHelper)
  , m_conditions(conditions)
  , m_surface{}
  , m_surfaces{}
  , m_surfaceCache{}
{
  m_idHash = m_idHelper->straw_layer_hash(id);
}

TRT_BaseElement::TRT_BaseElement(const TRT_BaseElement& right)
  : Trk::TrkDetElementBase(right.getMaterialGeom())
  , m_id(right.m_id)
  , m_idHash(right.m_idHash)
  , m_idHelper(right.m_idHelper)
  , m_conditions(right.m_conditions)
{}

// [0] GeoModel / CLHEP Access
const HepGeom::Transform3D
TRT_BaseElement::getAbsoluteTransform(int straw) const
{
  return Amg::EigenTransformToCLHEP(strawTransform(straw));
}

const Trk::Surface&
TRT_BaseElement::surface(const Identifier& id) const
{
  int straw = m_idHelper->straw(id);
  if (!m_strawSurfaces[straw]) {
    createSurfaceCache(id);
  }
  return *(m_strawSurfaces[straw]);
}

const std::vector<const Trk::Surface*>&
TRT_BaseElement::surfaces() const
{
  if (!m_surfaces.isValid()) {
    std::vector<const Trk::Surface*> tmp_surfaces;
    tmp_surfaces.reserve(nStraws());
    for (unsigned is = 0; is < nStraws(); ++is) {
      tmp_surfaces.push_back(&strawSurface(is));
    }
    m_surfaces.set(tmp_surfaces);
  }
  return *(m_surfaces.ptr());
}

const Trk::SurfaceBounds&
TRT_BaseElement::bounds(const Identifier&) const
{
  return strawBounds();
}

const Amg::Transform3D&
TRT_BaseElement::transform(const Identifier& id) const
{
  int straw = m_idHelper->straw(id);
  if (!m_strawSurfacesCache[straw]) {
    createSurfaceCache(id);
  }
  // forward the transform of the cache
  return m_strawSurfacesCache[straw]->transform();
}

const Amg::Transform3D&
TRT_BaseElement::strawTransform(unsigned int straw) const
{
  if (!m_strawSurfacesCache[straw]) {
    Identifier id = m_idHelper->straw_id(identify(), straw);
    createSurfaceCache(id);
  }
  // forward the transform of the cache
  return m_strawSurfacesCache[straw]->transform();
}

const Amg::Vector3D&
TRT_BaseElement::normal(const Identifier&) const
{
  // Not sure if the normal of the straw is ever used.
  // nor is there a well defined normal.
  // This wont be corrected for alignments.
  // Just return the element normal
  return normal();
}

const Amg::Vector3D&
TRT_BaseElement::center(const Identifier& id) const
{
  int straw = m_idHelper->straw(id);
  if (!m_strawSurfacesCache[straw]) {
    createSurfaceCache(id);
  }
  // forward the transform of the cache
  return m_strawSurfacesCache[straw]->center();
}

const Trk::StraightLineSurface&
TRT_BaseElement::strawSurface(int straw) const
{
  if (!m_strawSurfaces[straw]) {
    // get the straw identifier to the given straw number and element identifier
    Identifier id = m_idHelper->straw_id(identify(), straw);
    createSurfaceCache(id);
  }
  return *(m_strawSurfaces[straw].get());
}

const Amg::Transform3D&
TRT_BaseElement::strawTransform(int straw) const
{
  if (!m_strawSurfacesCache[straw]) {
    Identifier id = m_idHelper->straw_id(identify(), straw);
    createSurfaceCache(id);
  }
  // forward the transform of the cache
  return m_strawSurfacesCache[straw]->transform();
}

const Amg::Vector3D&
TRT_BaseElement::strawCenter(int straw) const
{
  if (!m_strawSurfacesCache[straw]) {
    Identifier id = m_idHelper->straw_id(identify(), straw);
    createSurfaceCache(id);
  }
  // forward the transform of the cache
  return m_strawSurfacesCache[straw]->center();
}

Amg::Vector3D
TRT_BaseElement::strawAxis(int straw) const
{
  return (strawTransform(straw).linear() * Amg::Vector3D::UnitZ() *
          strawDirection());
}

void
TRT_BaseElement::createSurfaceCache(Identifier id, GeoAlignmentStore* alignStore) const
{
  int straw = m_idHelper->straw(id);

  // convert neccessary parts to Amg
  if (!m_strawSurfacesCache[straw]) {
    // create the surface cache & fill it
    m_strawSurfacesCache[straw].set(createSurfaceCacheHelper(straw,alignStore));
  }
  // creaete the surface only if needed (the links are still intact)
  if (!m_strawSurfaces[straw]) {
    m_strawSurfaces[straw].set(
      std::make_unique<Trk::StraightLineSurface>(*this, id));
  }
}

std::unique_ptr<SurfaceCacheBase>
TRT_BaseElement::createSurfaceCacheHelper(int straw, GeoAlignmentStore* alignStore) const
{
  // get the StrawTransform from GeoModel
  HepGeom::Transform3D cStrawTransform = calculateStrawTransform(straw,alignStore);
  auto sTransform =
    Amg::Transform3D(Amg::CLHEPTransformToEigen(cStrawTransform));
  auto sCenter = Amg::Vector3D(sTransform.translation());
  // create the surface cache & fill it
  return std::make_unique<SurfaceCacheBase>(sTransform, sCenter);
}

void
TRT_BaseElement::invalidate()
{
  // Invalidate the caches
  // Call and barrel or endcap specific invalidation
  invalidateOther();
  // Its enough to delete and zero the caches.
  deleteCache();
}

void
TRT_BaseElement::deleteCache()
{
  // for all straws
  for (auto & i : m_strawSurfacesCache) {
    i.release();
  }
}

void
TRT_BaseElement::updateAllCaches(GeoAlignmentStore* alignStore) const
{
  // Strawlayer caches
  if (!m_surfaceCache.isValid()){
    createSurfaceCache(alignStore);
  }
  // Loop over all straws and request items that get cached.
  for (unsigned int iStraw = 0; iStraw < nStraws(); iStraw++) {
    Identifier strawId = m_idHelper->straw_id(identify(), iStraw);
    createSurfaceCache(strawId,alignStore);
  }
}

}
