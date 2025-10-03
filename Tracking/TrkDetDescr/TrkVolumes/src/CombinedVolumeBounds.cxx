/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CombinedVolumeBounds.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkVolumes/CombinedVolumeBounds.h"

#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkVolumes/SimplePolygonBrepVolumeBounds.h"
#include "TrkVolumes/SubtractedVolumeBounds.h"
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
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>

Trk::CombinedVolumeBounds::CombinedVolumeBounds() = default;

Trk::CombinedVolumeBounds::CombinedVolumeBounds(
  std::unique_ptr<Volume> vol1,
  std::unique_ptr<Volume> vol2,
  bool intersection)
  : VolumeBounds()
  , m_first(std::move(vol1))
  , m_second(std::move(vol2))
  , m_intersection(intersection)
  , m_objectAccessor()
  , m_boundsOrientation()
{}

Trk::CombinedVolumeBounds::CombinedVolumeBounds(
  const Trk::CombinedVolumeBounds& bobo)
  : VolumeBounds()
  , m_first{bobo.m_first->clone()}
  , m_second{bobo.m_second->clone()}
  , m_intersection(bobo.m_intersection)
  , m_objectAccessor(bobo.m_objectAccessor)
  , m_boundsOrientation(bobo.m_boundsOrientation)
{}

Trk::CombinedVolumeBounds::~CombinedVolumeBounds() = default;

Trk::CombinedVolumeBounds&
Trk::CombinedVolumeBounds::operator=(const Trk::CombinedVolumeBounds& bobo)
{
  if (this != &bobo) {
    m_first.reset(bobo.m_first->clone());
    m_second.reset(bobo.m_second->clone());
    m_intersection = bobo.m_intersection;
    m_objectAccessor = bobo.m_objectAccessor;
    m_boundsOrientation = bobo.m_boundsOrientation;
  }
  return *this;
}

std::vector<std::unique_ptr<Trk::Surface>>
Trk::CombinedVolumeBounds::decomposeToSurfaces(const Amg::Transform3D& transf) {

  auto retsf = std::vector<std::unique_ptr<Trk::Surface>>();

  const Trk::CylinderVolumeBounds* cylVol = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(m_first->volumeBounds()));
  const Trk::SimplePolygonBrepVolumeBounds* spbVol = dynamic_cast<const Trk::SimplePolygonBrepVolumeBounds*>(&(m_first->volumeBounds()));
  const Trk::CombinedVolumeBounds* comVol = dynamic_cast<const Trk::CombinedVolumeBounds*>(&(m_first->volumeBounds()));
  const Trk::SubtractedVolumeBounds* subVol = dynamic_cast<const Trk::SubtractedVolumeBounds*>(&(m_first->volumeBounds()));

  // get surfaces for first boundaries
  std::vector<std::unique_ptr<Trk::Surface>> firstSurfaces =
    m_first->volumeBounds().decomposeToSurfaces(transf * m_first->transform());
  // get surfaces for second boundaries
  std::vector<std::unique_ptr<Trk::Surface>> secondSurfaces =
      m_second->volumeBounds().decomposeToSurfaces(transf * m_second->transform());
  unsigned int nSurf = firstSurfaces.size() + secondSurfaces.size();
  m_boundsOrientation.resize(nSurf);

  std::vector<unsigned int> subtrSecond;

  // loop over surfaces; convert disc surface to a plane surface using elliptic
  // bounds
  for (unsigned int out = 0; out < firstSurfaces.size(); out++) {
    //
    const SubtractedPlaneSurface* splo = dynamic_cast<const SubtractedPlaneSurface*>(firstSurfaces[out].get());
    const PlaneSurface* plo = dynamic_cast<const PlaneSurface*>(firstSurfaces[out].get());
    const SubtractedCylinderSurface* sclo = dynamic_cast<const SubtractedCylinderSurface*>(firstSurfaces[out].get());
    const CylinderSurface* clo = dynamic_cast<const CylinderSurface*>(firstSurfaces[out].get());
    const DiscSurface* dlo = dynamic_cast<const DiscSurface*>(firstSurfaces[out].get());

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

    std::unique_ptr<Trk::Volume> secondSub(createSubtractedVolume(
      firstSurfaces[out]->transform().inverse() * transf, m_second.get()));

    if (sclo || splo) {
      bool shared = false;
      const Trk::AreaExcluder* vEx;
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

      auto firstSub = std::make_unique<Trk::Volume>(*volExcl->volume());

      std::unique_ptr<Trk::Volume> comb_sub{};
      if (!shared && !m_intersection){
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::CombinedVolumeBounds>(std::move(secondSub), std::move(firstSub), m_intersection));
      }
      if (!shared && m_intersection){
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::SubtractedVolumeBounds>(std::move(secondSub), std::move(firstSub)));
      }
      if (shared && m_intersection){
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::CombinedVolumeBounds>(std::move(secondSub), std::move(firstSub), m_intersection));
      }
      if (shared && !m_intersection){
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::SubtractedVolumeBounds>(std::move(firstSub), std::move(secondSub)));
      }
      auto volEx = std::make_shared<const Trk::VolumeExcluder>(std::move(comb_sub));
      bool new_shared = shared;
      if (m_intersection){
        new_shared = true;
      }
      if (splo){
        retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(*splo, std::move(volEx), new_shared));
      }
      else if (sclo){
        retsf.push_back(std::make_unique<Trk::SubtractedCylinderSurface>(*sclo, std::move(volEx), new_shared));
      }
    } else if (plo || clo || dlo) {
      auto volEx = std::make_shared<const Trk::VolumeExcluder>(std::move(secondSub));
      if (plo){
        retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(*plo, std::move(volEx), m_intersection));
      }
      else if (clo){
        retsf.push_back(std::make_unique<Trk::SubtractedCylinderSurface>(*clo, std::move(volEx), m_intersection));
      }
      else if (dlo) {
        const DiscBounds* db = dynamic_cast<const DiscBounds*>(&(dlo->bounds()));
        if (!db){
          throw std::logic_error("Not DiscBounds");
        }
        auto eb = std::make_shared<EllipseBounds>(db->rMin(), db->rMin(), db->rMax(), db->rMax(), db->halfPhiSector());
        retsf.push_back( std::make_unique<Trk::SubtractedPlaneSurface>(
            PlaneSurface(Amg::Transform3D(dlo->transform()), eb),
            std::move(volEx),
            m_intersection));
      }
    } else {
      throw std::runtime_error(
        "Unhandled surface in CombinedVolumeBounds::decomposeToSurfaces.");
    }
  }

  cylVol =
    dynamic_cast<const Trk::CylinderVolumeBounds*>(&(m_second->volumeBounds()));
  spbVol = dynamic_cast<const Trk::SimplePolygonBrepVolumeBounds*>(
    &(m_second->volumeBounds()));
  comVol =
    dynamic_cast<const Trk::CombinedVolumeBounds*>(&(m_second->volumeBounds()));
  subVol = dynamic_cast<const Trk::SubtractedVolumeBounds*>(
    &(m_second->volumeBounds()));
  unsigned int nOut = firstSurfaces.size();

  for (unsigned int in = 0; in < secondSurfaces.size(); in++) {
    //
    const SubtractedPlaneSurface* spli = dynamic_cast<const SubtractedPlaneSurface*>(secondSurfaces[in].get());
    const PlaneSurface* pli = dynamic_cast<const PlaneSurface*>(secondSurfaces[in].get());
    const SubtractedCylinderSurface* scli = dynamic_cast<const SubtractedCylinderSurface*>(secondSurfaces[in].get());
    const CylinderSurface* cli = dynamic_cast<const CylinderSurface*>(secondSurfaces[in].get());
    const DiscSurface* dli = dynamic_cast<const DiscSurface*>(secondSurfaces[in].get());

    // resolve bounds orientation : copy from combined/subtracted, swap inner
    // cyl, swap bottom spb
    if (comVol){
      m_boundsOrientation[nOut + in] = comVol->boundsOrientation()[in];
    }
    else if (subVol){
      m_boundsOrientation[nOut + in] = subVol->boundsOrientation()[in];
    }
    else if (cylVol && cli && in == 3){
      m_boundsOrientation[nOut + in] = false;
    }
    else if (spbVol && in == 0){
      m_boundsOrientation[nOut + in] = false;
    }
    else{
      m_boundsOrientation[nOut + in] = true;
    }

    std::unique_ptr<Trk::Volume> firstSub(createSubtractedVolume(
      secondSurfaces[in]->transform().inverse() * transf, m_first.get()));
    if (scli || spli) {
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
      auto secondSub = std::make_unique<Trk::Volume>(*volExcl->volume());

      std::unique_ptr<Trk::Volume> comb_sub{};
      if (!shared && !m_intersection){
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::CombinedVolumeBounds>(std::move(firstSub), std::move(secondSub), m_intersection));
      }
      if (!shared && m_intersection){
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::SubtractedVolumeBounds>(std::move(firstSub), std::move(secondSub)));
      }
      if (shared && m_intersection){
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::CombinedVolumeBounds>(std::move(firstSub), std::move(secondSub), m_intersection));
      }
      if (shared && !m_intersection){
        comb_sub = std::make_unique<Trk::Volume>(
          nullptr,
          std::make_shared<Trk::SubtractedVolumeBounds>(std::move(secondSub), std::move(firstSub)));
      }
      auto volEx = std::make_shared<const Trk::VolumeExcluder>(std::move(comb_sub));
      bool new_shared = shared;
      if (m_intersection){
        new_shared = true;
      }
      if (spli){
        retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(*spli, std::move(volEx), new_shared));
      }
      else if (scli)
        retsf.push_back( std::make_unique<Trk::SubtractedCylinderSurface>(*scli, std::move(volEx), new_shared));

    } else if (pli || cli || dli) {
      auto volEx = std::make_shared<const Trk::VolumeExcluder>(std::move(firstSub));
      if (pli){
        retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(*pli, std::move(volEx), m_intersection));
      }
      else if (cli){
        retsf.push_back(std::make_unique<Trk::SubtractedCylinderSurface>(*cli, std::move(volEx), m_intersection));
      }
      else if (dli) {
        const DiscBounds* db = dynamic_cast<const DiscBounds*>(&(dli->bounds()));
        if (!db){
          throw std::logic_error("Not DiscBounds");
        }
        auto eb = std::make_shared<EllipseBounds>(db->rMin(), db->rMin(), db->rMax(), db->rMax(), db->halfPhiSector());
        auto pliN = PlaneSurface(Amg::Transform3D(dli->transform()), eb);
        retsf.push_back(std::make_unique<Trk::SubtractedPlaneSurface>(pliN, std::move(volEx), m_intersection));
      }
    } else {
      throw std::runtime_error(
        "Unhandled surface in CombinedVolumeBounds::decomposeToSurfaces.");
    }
  }
  return retsf;
}

// ostream operator overload

MsgStream&
Trk::CombinedVolumeBounds::dump(MsgStream& sl) const
{
  std::stringstream temp_sl;
  temp_sl << std::setiosflags(std::ios::fixed);
  temp_sl << std::setprecision(7);
  temp_sl << "Trk::CombinedVolumeBounds: first,second ";
  sl << temp_sl.str();
  std::as_const(*m_first).volumeBounds().dump(sl);
  std::as_const(*m_second).volumeBounds().dump(sl);
  return sl;
}

std::ostream&
Trk::CombinedVolumeBounds::dump(std::ostream& sl) const
{
  std::stringstream temp_sl;
  temp_sl << std::setiosflags(std::ios::fixed);
  temp_sl << std::setprecision(7);
  temp_sl << "Trk::CombinedVolumeBounds: first,second ";
  sl << temp_sl.str();
  std::as_const(*m_first).volumeBounds().dump(sl);
  std::as_const(*m_second).volumeBounds().dump(sl);
  return sl;
}

Trk::Volume*
Trk::CombinedVolumeBounds::createSubtractedVolume(
  const Amg::Transform3D& transf,
  const Trk::Volume* subtrVol)
{
  if (!subtrVol){
    return nullptr;
  }
  return new Trk::Volume(*subtrVol, transf);
}

