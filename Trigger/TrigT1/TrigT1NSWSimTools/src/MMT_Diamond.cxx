/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MMT_Diamond.h"


namespace {
  // The stereo angle is fixed and can be hardcoded
  const double tan_stereo_angle = std::tan(0.02618);

  constexpr int bc_wind = 4; // fixed time window (in bunch crossings) during which the algorithm collects ART hits
  constexpr int n_addc = 4;
  constexpr int n_vmm  = 128;
}


MMT_Diamond::MMT_Diamond(const int diamXthreshold, const bool uv, const int diamUVthreshold, const int roadSize,
    const int olapEtaUp, const int olapEtaDown, const int olapStereoUp, const int olapStereoDown): AthMessaging(Athena::getMessageSvc(), "MMT_Diamond") {
    m_xthr = diamXthreshold;
    m_uvthr = diamUVthreshold;
    m_uvflag = uv;
    m_roadSize = roadSize;
    m_roadSizeUpX = olapEtaUp;
    m_roadSizeDownX = olapEtaDown;
    m_roadSizeUpUV =  olapStereoUp;
    m_roadSizeDownUV = olapStereoDown;
}

void MMT_Diamond::createRoads(std::vector<MMT_Road>& roads, const bool isLarge) const {
  const char sec = (isLarge) ? 'L' : 'S';
  /*
   * This computation is done as follows: 1024 X roads
   * MML: for i in [0,8] -> i*6 UV roads. Then: (1024-9)*6*9 UV roads
   * MMS: for i in [0,6] -> i*6 UV roads. Then: (1024-7)*6*7 UV roads
   */
  const unsigned int vecRoads = (isLarge) ? 56050 : 43864;
  roads.reserve(vecRoads);
  int nroad = 8192/this->getRoadSize();
  for (int i = 0; i < nroad; ++i) {
    roads.emplace_back(sec, m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV, m_xthr, m_uvthr, i);

    /*
     * The computation of "nuv" is:
     * B = (1./std::tan(1.5/180.*M_PI));
     * nuv = std::round( par->getlWidth() / (B * 0.4 * 2.)/this->getRoadSize() );
     * As getlWidth() has to deal with the full wedge, only two (fixed by construction) integer values are allowed, according to small (7) or large (9) wedge
     */
    if(m_uvflag) {
      const int nuv = (isLarge) ? 9 : 7;
      for (int uv = 1; uv <= nuv; uv++) {
        if (i-uv < 0) continue;
        roads.emplace_back(sec, m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV, m_xthr, m_uvthr, i, i+uv, i-uv);
        roads.emplace_back(sec, m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV, m_xthr, m_uvthr, i, i-uv, i+uv);
        roads.emplace_back(sec, m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV, m_xthr, m_uvthr, i, i+uv-1, i-uv);
        roads.emplace_back(sec, m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV, m_xthr, m_uvthr, i, i-uv, i+uv-1);
        roads.emplace_back(sec, m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV, m_xthr, m_uvthr, i, i-uv+1, i+uv);
        roads.emplace_back(sec, m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV, m_xthr, m_uvthr, i, i+uv, i-uv+1);
      }
    }
  }
}

void MMT_Diamond::findDiamonds(std::vector<std::shared_ptr<MMT_Hit> >& hits, std::vector<MMT_Road>& roads, std::vector<slope_t>& diamondSlopes, const int sectorPhi) const {

  // Comparison with lambda function (easier to implement)
  std::sort(hits.begin(), hits.end(), [](const auto &h1, const auto &h2){ return h1->getBC() < h2->getBC(); });
  const int bc_start = hits.front()->getBC();
  const int bc_end = hits.front()->getBC() + 16;
  ATH_MSG_DEBUG("Window Start: " << bc_start << " - Window End: " << bc_end);

  std::vector<std::shared_ptr<MMT_Hit> > hits_now;
  std::vector< std::pair<int, float> > vmm_same;
  std::vector< std::pair<int, int> > addc_same;
  std::vector<int> to_erase;

  // each road makes independent triggers, evaluated on each BC
  unsigned int ibc = 0;
  for (int bc = hits.front()->getBC(); bc < bc_end; bc++) {
    // Cleaning stuff
    hits_now.clear();

    for (unsigned int j = ibc; j < hits.size(); j++) {
      if (hits[j]->getBC() == bc) hits_now.push_back(hits[j]);
      else if (hits[j]->getBC() > bc) {
        ibc = j;
        break;
      }
    }

    // Simulate harware filters: ART hits + ADDC filter
    for (unsigned int ib = 0; ib < 8; ib++) { //loop on plane from 0 to 7
      // VMM-ART-hit filter
      for (int j = 0; j < n_vmm; j++) {
        vmm_same.clear();
        unsigned int k = 0;
        for (const auto &hit_pointer: hits_now) {
          if (static_cast<unsigned int>(hit_pointer->getPlane()) != ib) continue;
          if (hit_pointer->getVMM() == j){
            vmm_same.push_back( std::make_pair(k, hit_pointer->getTime()) );
          }
          k++;
        }
        if (vmm_same.size() > 1) {
          to_erase.clear();
          std::sort(vmm_same.begin(), vmm_same.end(), [](const std::pair<int, float>& p1, const std::pair<int, float>& p2) { return p1.second < p2.second; });
          for (auto pair: vmm_same) to_erase.push_back(pair.first);
          // reverse and erase
          std::sort(to_erase.rbegin(), to_erase.rend());
          to_erase.pop_back(); //removes the hit with the earliest time
          for (auto l : to_erase) {
            hits_now.erase(hits_now.begin() + l);
          }
        }
      }
      // ADDC-like filter
      for (int ia = 0; ia < n_addc; ia++) { // From 0 to 3 (local index of the ART ASIC in the layer)
        addc_same.clear();
        for (unsigned int k = 0; k < hits_now.size(); k++) {
          if ((unsigned int)(hits_now[k]->getPlane()) != ib) continue;
          int istrip = (std::abs(hits_now[k]->getStationEta())-1) * (64*8*10) + hits_now[k]->getChannel(); //needed the global strip index on the sector layer (getChannel returns chamber's local strip index)
          if (hits_now[k]->getART() == ia) addc_same.emplace_back(k, istrip);
        }

        if (addc_same.size() > 8) {
          // priority encode the hits by channel number; remember hits 8+
          to_erase.clear();

          std::sort(addc_same.begin(), addc_same.end(), [](const std::pair<int, int>& p1, const std::pair<int, int>& p2) { return p1.second < p2.second; });
          for (unsigned int it = 8; it < addc_same.size(); it++) to_erase.push_back(addc_same[it].first);

          // reverse and erase
          std::sort(to_erase.rbegin(), to_erase.rend());
          for (auto l : to_erase) {
            hits_now.erase(hits_now.begin() + l);
          }
        }
      }
    } // loop on plane for VMM and ART ASIC filter

    for (auto &road : roads) {

      if (!road.getHitVector().empty()) road.incrementAge(bc_wind);
      if (!hits_now.empty()) road.addHits(hits_now);

      if (road.checkCoincidences(bc_wind) && bc >= (bc_start - 1)) {

        ATH_MSG_DEBUG("------------------------------------------------------------------");
        ATH_MSG_DEBUG("Coincidence FOUND @BC: " << bc);
        ATH_MSG_DEBUG("Road (x, u, v, count): (" << road.iRoadx() << ", " << road.iRoadu() << ", " << road.iRoadv() << ", " << road.countHits() << ")");
        ATH_MSG_DEBUG("------------------------------------------------------------------");

        std::vector<int> bcidVec;
        for (const auto &hit: road.getHitVector()) {
          bcidVec.push_back(hit.getBC());
        }
        std::sort(bcidVec.begin(), bcidVec.end());

        // evaluating mode of the BCID of the hits in the diamond
        // default setting in the firmware is the mode of the hits's bcid in the diamond
        int bcidVal=bcidVec[0], bcidCount=1, modeCount=1, bcidMode=bcidVec[0];
        for (unsigned int i=1; i<bcidVec.size(); i++){
          if (bcidVec[i] == bcidVal){
            bcidCount++;
          } else {
            bcidCount = 1;
            bcidVal = bcidVec[i];
          }
          if (bcidCount > modeCount) {
            modeCount = bcidCount;
            bcidMode = bcidVal;
          }
        }

        slope_t slope;
        slope.BC = bcidMode;
        slope.totalCount = road.countHits();
        slope.realCount = road.countRealHits();
        slope.iRoad = road.iRoadx();
        slope.iRoadu = road.iRoadu();
        slope.iRoadv = road.iRoadv();
        slope.uvbkg = road.countUVHits(true); // the bool in the following 4 functions refers to background/noise hits
        slope.xbkg = road.countXHits(true);
        slope.uvmuon = road.countUVHits(false);
        slope.xmuon = road.countXHits(false);
        slope.age = slope.BC - bc_start;
        slope.mxl = road.mxl();
        slope.my = road.avgSofX(); // defined as my in ATL-COM-UPGRADE-2015-033
        slope.uavg = road.avgSofUV(2,4);
        slope.vavg = road.avgSofUV(3,5);
        slope.mx = (slope.uavg-slope.vavg)/(2.*tan_stereo_angle);
        const double theta = std::atan(std::sqrt(std::pow(slope.mx,2) + std::pow(slope.my,2)));
        slope.theta = (slope.my > 0.) ? theta : M_PI - theta;
        slope.eta = -1.*std::log(std::tan(slope.theta/2.));
        slope.dtheta = (slope.mxl - slope.my)/(1. + slope.mxl*slope.my);
        slope.side = (slope.my > 0.) ? 'A' : 'C';
        slope.phi = std::atan(slope.mx/slope.my);
        slope.phiShf = phiShift(sectorPhi, slope.phi, slope.side);
        slope.lowRes = road.evaluateLowRes();

        diamondSlopes.push_back(slope);
      }
    }
  }
}

double MMT_Diamond::phiShift(const int n, const double phi, const char side) const {
  double Phi = (side == 'A') ? -phi : phi;
  double shift = (n > 8) ? (16-n)*M_PI/8. : n*M_PI/8.;
  if (n < 8)       return (Phi + shift);
  else if (n == 8) return (Phi + ((Phi > 0.) ? -1. : 1.)*shift);
  else             return (Phi - shift);
}
