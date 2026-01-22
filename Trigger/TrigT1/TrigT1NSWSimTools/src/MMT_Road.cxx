/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "MMT_Road.h"

MMT_Road::MMT_Road(const char sector, const int roadSize,
                   const int UpX, const int DownX, const int UpUV, const int DownUV,
                   const int xthr, const int uvthr,
                   const int iroadx, const int iroadu, const int iroadv) :
  m_iroadx(iroadx),
  m_iroadu(iroadu != -1 ? iroadu : iroadx),
  m_iroadv(iroadv != -1 ? iroadv : iroadx),
  m_xthr(xthr),
  m_uvthr(uvthr),
  m_sector(sector)
{
  // Pre-calculate slopes
  m_slopeXlow  = roadSize*m_iroadx     + 0.5 - DownX;
  m_slopeXhigh = roadSize*(m_iroadx+1) + 0.5 + UpX;

  m_slopeUlow  = roadSize*m_iroadu     + 0.5 - DownUV;
  m_slopeUhigh = roadSize*(m_iroadu+1) + 0.5 + UpUV;

  m_slopeVlow  = roadSize*m_iroadv     + 0.5 - DownUV;
  m_slopeVhigh = roadSize*(m_iroadv+1) + 0.5 + UpUV;
}

void MMT_Road::addHits(std::vector<std::shared_ptr<MMT_Hit> > &hits) {
  for (const auto &hit_i : hits) {
    double slow, shigh;
    if (hit_i->isX()) {
      slow = hit_i->getShift() + m_slopeXlow * hit_i->getPitchOverZ();
      shigh = hit_i->getShift() + m_slopeXhigh * hit_i->getPitchOverZ();
    }
    else if (hit_i->isU()) {
      slow = hit_i->getShift() + m_slopeUlow * hit_i->getPitchOverZ();
      shigh = hit_i->getShift() + m_slopeUhigh * hit_i->getPitchOverZ();
    }
    else {
      slow = hit_i->getShift() + m_slopeVlow * hit_i->getPitchOverZ();
      shigh = hit_i->getShift() + m_slopeVhigh * hit_i->getPitchOverZ();
    }

    const double val = hit_i->getRZSlope();
    bool has_hit = (val > 0.) ? (val > slow && val < shigh) : (val > shigh && val < slow);

    if (!has_hit) continue;

    const int bo = hit_i->getPlane();
    auto it = std::find_if(m_road_hits.begin(), m_road_hits.end(), [&bo](const auto &hit) { return (hit.getPlane() == bo); });
    if (it != m_road_hits.end()) continue;

    m_road_hits.emplace_back(*hit_i.get());
    m_road_hits.back().setAge(0);
  }
}

double MMT_Road::avgSofX() const {
  double sum = 0;
  int N = 0;
  for (const auto &hit : m_road_hits) {
    if (hit.isX()) {
      sum += hit.getRZSlope();
      ++N;
    }
  }
  return sum/N;
}

double MMT_Road::avgSofUV(const int uv1, const int uv2) const {
  double sum = 0;
  int N = 0;
  for (const auto &hit : m_road_hits) {
    const int bo = hit.getPlane();
    if (bo == uv1 || bo == uv2) {
      sum += hit.getRZSlope();
      ++N;
    }
  }
  return sum/N;
}

unsigned int MMT_Road::countUHits() const {
  return std::count_if(m_road_hits.begin(), m_road_hits.end(),
                       [](const auto& hit) { return hit.isU(); });
}

unsigned int MMT_Road::countXHits() const {
  return std::count_if(m_road_hits.begin(), m_road_hits.end(),
                       [](const auto& hit) { return hit.isX(); });
}

bool MMT_Road::evaluateLowRes() const {
  unsigned int nhits1 = 0, nhits2 = 0;
  for (const auto &hit : m_road_hits) {
    nhits1 += hit.getPlane() < 4;
    nhits2 += hit.getPlane() > 3;
  }
  return (nhits1 < 4 || nhits2 < 4);
}

bool MMT_Road::horizontalCheck() const {
  int nx1 = 0, nx2 = 0;
  for (const auto &hit : m_road_hits) {
    nx1 += hit.getPlane() >-1 && hit.getPlane() < 2;
    nx2 += hit.getPlane() > 5 && hit.getPlane() < 8;

    if (nx1 > 0 && nx2 > 0 && (nx1+nx2) >= m_xthr) return true;
  }
  return false;
}

void MMT_Road::incrementAge(const int bcwind) {
  short int old_ihits = 0;
  for (auto &hit : m_road_hits) {
    hit.setAge(hit.getAge()+1);
    if (hit.getAge() > (bcwind-1)) ++old_ihits;
  }
  m_road_hits.erase(m_road_hits.begin(), m_road_hits.begin()+old_ihits);
}

bool MMT_Road::matureCheck(const int bcwind) const {
  for (const auto &hit : m_road_hits) {
    if (hit.getAge() == (bcwind - 1)) return true;
  }
  return false;
}

double MMT_Road::mxl() const {
  std::vector<double> ys, zs;
  for (const auto &hit : m_road_hits) {
    if (hit.isX()) {
      ys.push_back(hit.getR());
      zs.push_back(hit.getZ());
    }
  }
  double mxl = 0;
  double avg_z = std::accumulate(zs.begin(), zs.end(), 0.0)/(double)zs.size();
  double sum_sq_z = std::inner_product(zs.begin(), zs.end(), zs.begin(), 0.0);
  for (unsigned int i = 0; i < ys.size(); i++) mxl += ys[i]*( (zs[i]-avg_z) / (sum_sq_z - zs.size()*std::pow(avg_z,2)) );

  return mxl;
}

bool MMT_Road::stereoCheck() const {

  if (m_uvthr == 0) return true;

  int nu = 0, nv = 0;
  for (const auto &hit : m_road_hits) {
    nu += hit.isU();
    nv += hit.isV();

    if (nu > 0 && nv > 0 && (nu+nv) >= m_uvthr) return true;
  }
  return false;
}
