/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "TrigT1NSWSimTools/MMT_Road.h"

MMT_Road::MMT_Road(const char sector, const int roadSize, const int UpX, const int DownX, const int UpUV, const int DownUV, const int xthr, const int uvthr,
                   const int iroadx, const int iroadu, const int iroadv) {
  m_sector = sector;
  m_iroadx = iroadx;
  m_iroadu = (iroadu != -1) ? iroadu : iroadx;
  m_iroadv = (iroadv != -1) ? iroadv : iroadx;
  m_xthr = xthr;
  m_uvthr = uvthr;

  m_roadSize = roadSize;
  m_roadSizeUpX = UpX;
  m_roadSizeDownX = DownX;
  m_roadSizeUpUV = UpUV;
  m_roadSizeDownUV = DownUV;
}

void MMT_Road::addHits(std::vector<std::shared_ptr<MMT_Hit> > &hits) {
  for (const auto &hit_i : hits) {
    if (m_sector != hit_i->getSector()) continue;

    int iroad = 0;
    unsigned short int olow = 0, ohigh = 0;
    if (hit_i->isX()) {
      iroad = m_iroadx;
      olow  = m_roadSizeDownX;
      ohigh = m_roadSizeUpX;
    }
    else if (hit_i->isU()) {
      iroad = m_iroadu;
      olow  = m_roadSizeDownUV;
      ohigh = m_roadSizeUpUV;
    }
    else if (hit_i->isV()) {
      iroad = m_iroadv;
      olow  = m_roadSizeDownUV;
      ohigh = m_roadSizeUpUV;
    }
    else continue;

    double val = hit_i->getShift();
    double slow  = val + (m_roadSize*iroad     + 0.5 - olow )*hit_i->getPitchOverZ();
    double shigh = val + (m_roadSize*(iroad+1) + 0.5 + ohigh)*hit_i->getPitchOverZ();

    val = hit_i->getRZSlope();
    bool has_hit = (val > 0.) ? (val > slow && val < shigh) : (val > shigh && val < slow);
    if (!has_hit) continue;

    has_hit = false;
    unsigned short int bo = hit_i->getPlane();
    auto it = std::find_if(m_road_hits.begin(), m_road_hits.end(), [&bo](const auto &hit) { return (hit->getPlane() == bo); });
    if (it != m_road_hits.end()) {
      has_hit = true;
      if (!hit_i->isNoise() && (*it)->isNoise()) {
        m_road_hits.erase(it);
        has_hit = false;
      }
    }

    if (has_hit) continue;
    auto hit = std::make_unique<MMT_Hit>(*hit_i.get());
    hit->setAge(0);
    m_road_hits.push_back(std::move(hit));
  }
}

double MMT_Road::avgSofX() const {
  std::vector<double> sl;
  for (const auto &hit : m_road_hits) {
    int bo = hit->getPlane();
    if (bo < 2 || bo > 5) sl.push_back(hit->getRZSlope());
  }
  double avg_x = std::accumulate(sl.begin(), sl.end(), 0.0)/(double)sl.size();
  return avg_x;
}

double MMT_Road::avgSofUV(const int uv1, const int uv2) const {
  std::vector<double> sl;
  for (const auto &hit : m_road_hits) {
    int bo = hit->getPlane();
    if (bo == uv1 || bo == uv2) sl.push_back(hit->getRZSlope());
  }
  double avg_uv = std::accumulate(sl.begin(), sl.end(), 0.0)/(double)sl.size();
  return avg_uv;
}

double MMT_Road::avgZofUV(const int uv1, const int uv2) const {
  std::vector<double> zs;
  for (const auto &hit : m_road_hits) {
    int bo = hit->getPlane();
    if (bo == uv1 || bo == uv2) zs.push_back(hit->getZ());
  }
  double avg_z = std::accumulate(zs.begin(), zs.end(), 0.0)/(double)zs.size();
  return avg_z;
}

bool MMT_Road::checkCoincidences(const int bcwind) const {
  bool passHorizontalCheck = this->horizontalCheck();
  if (!passHorizontalCheck) return false;
  bool passStereoCheck = this->stereoCheck();
  if (!passStereoCheck) return false;
  bool passMatureCheck = this->matureCheck(bcwind);
  return (passHorizontalCheck && passStereoCheck && passMatureCheck);
}

unsigned int MMT_Road::countRealHits() const {
  int nreal = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->isNoise() == false) nreal++;
  }
  return nreal;
}

unsigned int MMT_Road::countUVHits(bool flag) const {
  unsigned int nuv = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->getPlane() == 2 || hit->getPlane() == 4) {
      if (hit->isNoise() == flag) nuv++;
    }
    if (hit->getPlane() == 3 || hit->getPlane() == 5) {
      if (hit->isNoise() == flag) nuv++;
    }
  }
  return nuv;
}

unsigned int MMT_Road::countXHits(bool flag) const {
  unsigned int nx = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->getPlane() < 2 || hit->getPlane() > 5) {
      if (hit->isNoise() == flag) nx++;
    }
  }
  return nx;
}

bool MMT_Road::evaluateLowRes() const {
  unsigned int nhits1 = 0, nhits2 = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->getPlane() < 4 && !hit->isNoise()) nhits1++;
    else if (hit->getPlane() > 3 && !hit->isNoise()) nhits2++;
  }
  return (nhits1 < 4 || nhits2 < 4);
}

bool MMT_Road::horizontalCheck() const {
  int nx1 = 0, nx2 = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->getPlane() >-1 && hit->getPlane() < 2) nx1++;
    if (hit->getPlane() > 5 && hit->getPlane() < 8) nx2++;
  }
  return (nx1 > 0 && nx2 > 0 && (nx1+nx2) >= m_xthr);
}

void MMT_Road::incrementAge(const int bcwind) {
  std::vector<unsigned int> old_ihits;
  for (unsigned int j = 0; j < m_road_hits.size(); j++) {
    m_road_hits[j]->setAge(m_road_hits[j]->getAge() +1);
    if (m_road_hits[j]->getAge() > (bcwind-1)) old_ihits.push_back(j);
  }
  for (int j = old_ihits.size()-1; j > -1; j--) m_road_hits.erase(m_road_hits.begin()+j);
}

bool MMT_Road::matureCheck(const int bcwind) const {
  for (const auto &hit : m_road_hits) {
    if (hit->getAge() == (bcwind - 1)) return true;
  }
  return false;
}

double MMT_Road::mxl() const {
  std::vector<double> ys, zs;
  for (const auto &hit : m_road_hits) {
    int bo = hit->getPlane();
    if (bo < 2 || bo > 5) {
      ys.push_back(hit->getR());
      zs.push_back(hit->getZ());
    }
  }
  double mxl = 0;
  double avg_z = std::accumulate(zs.begin(), zs.end(), 0.0)/(double)zs.size();
  double sum_sq_z = std::inner_product(zs.begin(), zs.end(), zs.begin(), 0.0);
  for (unsigned int i = 0; i < ys.size(); i++) mxl += ys[i]*( (zs[i]-avg_z) / (sum_sq_z - zs.size()*std::pow(avg_z,2)) );

  return mxl;
}

void MMT_Road::reset() {
  if (!m_road_hits.empty()) m_road_hits.clear();
}

bool MMT_Road::stereoCheck() const {

  if (this->getUVthreshold() == 0) return true;

  int nu = 0, nv = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->getPlane() == 2 || hit->getPlane() == 4) nu++;
    if (hit->getPlane() == 3 || hit->getPlane() == 5) nv++;
  }

  return (nu > 0 && nv > 0 && (nu+nv) >= m_uvthr);
}
