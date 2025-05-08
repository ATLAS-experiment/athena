/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SubtractedVolumeBounds.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkVolumes/SubtractedVolumeBounds.h"
#include "TrkVolumes/CombinedVolumeBounds.h"
#include "TrkVolumes/SimplePolygonBrepVolumeBounds.h"
#include "TrkVolumes/VolumeExcluder.h"
// TrkSurfaces
#include "TrkGeometrySurfaces/SubtractedCylinderSurface.h"
#include "TrkGeometrySurfaces/SubtractedPlaneSurface.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscBounds.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/EllipseBounds.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/Surface.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkVolumes/Volume.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <cmath>
#include <iostream>
#include <utility>

Trk::SubtractedVolumeBounds::SubtractedVolumeBounds() = default;

Trk::SubtractedVolumeBounds::SubtractedVolumeBounds(std::unique_ptr<Volume> vol1, std::unique_ptr<Volume> vol2)
  : VolumeBounds()
  , m_outer(std::move(vol1))
  , m_inner(std::move(vol2))
  , m_objectAccessor()
  , m_boundsOrientation()
{}

Trk::SubtractedVolumeBounds::SubtractedVolumeBounds(
  const Trk::SubtractedVolumeBounds& bobo)
  : VolumeBounds()
  , m_outer{bobo.m_outer->clone()}
  , m_inner{bobo.m_inner->clone()}
  , m_objectAccessor(bobo.m_objectAccessor)
  , m_boundsOrientation(bobo.m_boundsOrientation)
{}

Trk::SubtractedVolumeBounds::~SubtractedVolumeBounds() = default;

Trk::SubtractedVolumeBounds&
Trk::SubtractedVolumeBounds::operator=(const Trk::SubtractedVolumeBounds& bobo)
{
  if (this != &bobo) {
    m_outer.reset(bobo.m_outer->clone());
    m_inner.reset(bobo.m_inner->clone());
    m_objectAccessor = bobo.m_objectAccessor;
    m_boundsOrientation = bobo.m_boundsOrientation;
  }
  return *this;
}

std::vector<std::unique_ptr<Trk::Surface>>
Trk::SubtractedVolumeBounds::decomposeToSurfaces(
    const Amg::Transform3D& transf) {
  // double tol=0.001;
  // get surfaces for outer boundaries
  std::vector<std::unique_ptr<Trk::Surface>> outerSurfaces =
    m_outer->volumeBounds().decomposeToSurfaces(transf * m_outer->transform());
  // get surfaces for inner boundaries
  std::vector<std::unique_ptr<Trk::Surface>> innerSurfaces =
    m_inner->volumeBounds().decomposeToSurfaces(transf * m_inner->transform());
  std::vector<unsigned int> subtrInner;


  auto retsf = std::vector<std::unique_ptr<Trk::Surface>>();

  unsigned int nSurf = outerSurfaces.size() + innerSurfaces.size();
  m_boundsOrientation.resize(nSurf);

  const Trk::CylinderVolumeBounds* cylVol = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(m_outer->volumeBounds()));
  const Trk::SimplePolygonBrepVolumeBounds* spbVol = dynamic_cast<const Trk::SimplePolygonBrepVolumeBounds*>(&(m_outer->volumeBounds()));
  const Trk::CombinedVolumeBounds* comVol = dynamic_cast<const Trk::CombinedVolumeBounds*>(&(m_outer->volumeBounds()));
  const Trk::SubtractedVolumeBounds* subVol = dynamic_cast<const Trk::SubtractedVolumeBounds*>(&(m_outer->volumeBounds()));

  // loop over 'outer' boundary surfaces; modified by subtracted volume
  for (unsigned int out = 0; out < outerSurfaces.size(); out++) {
    const SubtractedPlaneSurface* splo = dynamic_cast<const SubtractedPlaneSurface*>(outerSurfaces[out].get());
    const PlaneSurface* plo = dynamic_cast<const PlaneSurface*>(outerSurfaces[out].get());
    const SubtractedCylinderSurface* sclo = dynamic_cast<const SubtractedCylinderSurface*>(outerSurfaces[out].get());
    const CylinderSurface* clo = dynamic_cast<const CylinderSurface*>(outerSurfaces[out].get());
    const DiscSurface* dlo = dynamic_cast<const DiscSurface*>(outerSurfaces[out].get());

    if (!(splo || plo || sclo || clo || dlo)) {
      throw std::runtime_error("Unhandled surface.");
    }
    // resolve bounds orientation : copy from combined/subtracted, swap inner
    // cyl, swap bottom spb
    if (comVol){
      m_boundsOrientation[out] = comVol->boundsOrientation()[out];
    }
    else if (subVol){
      m_boundsOrientation[out] = subVol->boundsOrientation()[out];
    }
    else if (cylVol && clo && out == 3){
      m_boundsOrientation[out] = false;
    }
    else if (spbVol && out == 0){
      m_boundsOrientation[out] = false;
    }
    else{
      m_boundsOrientation[out] = true;
    }
    //
    auto innerSub = std::unique_ptr<Trk::Volume>(createSubtractedVolume(
      outerSurfaces[out]->transform().inverse() * transf, m_inner.get()));

    if (splo || sclo) { // multiple subtraction
      const Trk::AreaExcluder* vEx;
      bool shared = false;
      if (splo) {
        vEx = splo->subtractedVolume();
        shared = splo->shared();
      }
      if (sclo) {
        vEx = sclo->subtractedVolume();
        shared = sclo->shared();
      }
      const Trk::VolumeExcluder* volExcl = dynamic_cast<const Trk::VolumeExcluder*>(vEx);
      if (!volExcl){
        throw std::logic_error("Not a VolumeExcluder");
      }

      auto outerSub = std::make_unique<Trk::Volume>(*volExcl->volume());
      std::unique_ptr<Trk::Volume> comb_sub;
      if (!shared){
        comb_sub = std::make_unique<Trk::Volume>(
            nullptr,
            std::make_shared<Trk::CombinedVolumeBounds>(std::move(innerSub), std::move(outerSub), false));
      } else {
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::SubtractedVolumeBounds>(std::move(outerSub), std::move(innerSub)));
      }
      auto volEx = std::make_unique<Trk::VolumeExcluder>(std::move(comb_sub));
      if (splo){
        retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(*splo, std::move(volEx), shared));
      }
      else if (sclo){
        retsf.push_back(
          std::make_unique<Trk::SubtractedCylinderSurface>(*sclo, std::move(volEx), shared));
      }
    } else {
      auto volEx = std::make_unique<Trk::VolumeExcluder>(std::move(innerSub));
      if (plo){
        retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(*plo, std::move(volEx), false));
      }
      else if (clo){
        retsf.push_back(std::make_unique<Trk::SubtractedCylinderSurface>(*clo, std::move(volEx), false));
      }
      else if (dlo) {
        // turn disc into ellipse for simplification
        const DiscBounds* db = dynamic_cast<const DiscBounds*>(&(dlo->bounds()));
        if (!db){
          throw std::logic_error("Not DiscBounds");
        }
        auto eb = std::make_shared<EllipseBounds>(db->rMin(), db->rMin(), db->rMax(), db->rMax(), db->halfPhiSector());
        auto ploA = PlaneSurface(Amg::Transform3D(dlo->transform()), eb);
        retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(ploA, std::move(volEx), false));
      }
    }
  }

  // loop over 'inner' boundary surfaces; include only if represent a new
  // surface change: include allways otherwise orientation messed up bonus :
  // solves 'double boundary' problem
  cylVol = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(m_inner->volumeBounds()));
  spbVol = dynamic_cast<const Trk::SimplePolygonBrepVolumeBounds*>(&(m_inner->volumeBounds()));
  comVol = dynamic_cast<const Trk::CombinedVolumeBounds*>(&(m_inner->volumeBounds()));
  subVol = dynamic_cast<const Trk::SubtractedVolumeBounds*>(&(m_inner->volumeBounds()));
  unsigned int nOut = outerSurfaces.size();

  for (unsigned int in = 0; in < innerSurfaces.size(); in++) {
    const SubtractedPlaneSurface* spli = dynamic_cast<const SubtractedPlaneSurface*>(innerSurfaces[in].get());
    const PlaneSurface* pli = dynamic_cast<const PlaneSurface*>(innerSurfaces[in].get());
    const SubtractedCylinderSurface* scli = dynamic_cast<const SubtractedCylinderSurface*>(innerSurfaces[in].get());
    const CylinderSurface* cli = dynamic_cast<const CylinderSurface*>(innerSurfaces[in].get());
    const DiscSurface* dli = dynamic_cast<const DiscSurface*>(innerSurfaces[in].get());
    // resolve bounds orientation : copy from combined/subtracted, swap inner
    // cyl, swap bottom spb, swap all
    if (comVol){
      m_boundsOrientation[nOut + in] = !comVol->boundsOrientation()[in];
    }
    else if (subVol){
      m_boundsOrientation[nOut + in] = !subVol->boundsOrientation()[in];
    }
    else if (cylVol && cli && in == 3){
      m_boundsOrientation[nOut + in] = true;
    }
    else if (spbVol && in == 0){
      m_boundsOrientation[nOut + in] = true;
    }
    else{
      m_boundsOrientation[nOut + in] = false;
    }
    //
    auto outerSub = std::unique_ptr<Trk::Volume>(createSubtractedVolume(
      innerSurfaces[in]->transform().inverse() * transf, m_outer.get()));

    if (spli || scli) {
      bool shared = false;
      const Trk::AreaExcluder* vEx;
      if (spli) {
        vEx = spli->subtractedVolume();
        shared = spli->shared();
      }
      if (scli) {
        vEx = scli->subtractedVolume();
        shared = scli->shared();
      }
      const Trk::VolumeExcluder* volExcl = dynamic_cast<const Trk::VolumeExcluder*>(vEx);
      if (!volExcl){
        throw std::logic_error("Not a VolumeExcluder");
      }

      auto innerSub = std::make_unique<Trk::Volume>(*volExcl->volume());
      // combined volume
      std::unique_ptr<Trk::Volume> comb_sub;
      if (!shared){
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::SubtractedVolumeBounds>(std::move(outerSub), std::move(innerSub)));
      }
      else{
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::CombinedVolumeBounds>(std::move(innerSub), std::move(outerSub), true));
      }
      auto volEx = std::make_unique<Trk::VolumeExcluder>(std::move(comb_sub));
      if (spli){
        retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(*spli, std::move(volEx), true));
      }
      else if (scli){
        retsf.push_back(std::make_unique<Trk::SubtractedCylinderSurface>(*scli, std::move(volEx), true));
      }

    } else if (pli || cli) {
      auto volEx = std::make_unique<Trk::VolumeExcluder>(std::move(outerSub));
      if (pli){
        retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(*pli, std::move(volEx), true));
      }
      else if (cli){
        retsf.push_back(std::make_unique<Trk::SubtractedCylinderSurface>(*cli, std::move(volEx), true));
      }
    } else if (dli) {
      // turn disc into ellipse for simplification
      const DiscBounds* db = dynamic_cast<const DiscBounds*>(&(dli->bounds()));
      if (!db){
        throw std::logic_error("Not DiscBounds");
      }
      auto eb = std::make_shared<EllipseBounds>(db->rMin(), db->rMin(), db->rMax(), db->rMax(), db->halfPhiSector());
      PlaneSurface pla(Amg::Transform3D(dli->transform()), eb);
      auto volEx = std::make_unique<Trk::VolumeExcluder>(std::move(outerSub));
      retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(pla, std::move(volEx), true));
    } else {
      throw std::runtime_error(
        "Unhandled surface in "
        "Trk::SubtractedVolumeBounds::decomposeToSurfaces.");
    }
  }

  return retsf;
}

// ostream operator overload

MsgStream&
Trk::SubtractedVolumeBounds::dump(MsgStream& sl) const
{
  std::stringstream temp_sl;
  temp_sl << std::setiosflags(std::ios::fixed);
  temp_sl << std::setprecision(7);
  temp_sl << "Trk::SubtractedVolumeBounds: outer,inner ";
  sl << temp_sl.str();
  std::as_const(*m_outer).volumeBounds().dump(sl);
  std::as_const(*m_inner).volumeBounds().dump(sl);
  return sl;
}

std::ostream&
Trk::SubtractedVolumeBounds::dump(std::ostream& sl) const
{
  std::stringstream temp_sl;
  temp_sl << std::setiosflags(std::ios::fixed);
  temp_sl << std::setprecision(7);
  temp_sl << "Trk::SubtractedVolumeBounds: outer,inner ";
  sl << temp_sl.str();
  std::as_const(*m_outer).volumeBounds().dump(sl);
  std::as_const(*m_inner).volumeBounds().dump(sl);
  return sl;
}

Trk::Volume*
Trk::SubtractedVolumeBounds::createSubtractedVolume(
  const Amg::Transform3D& transf,
  Trk::Volume* subtrVol)
{
  if (!subtrVol){
    return nullptr;
  }
  return new Trk::Volume(*subtrVol, transf);
}

