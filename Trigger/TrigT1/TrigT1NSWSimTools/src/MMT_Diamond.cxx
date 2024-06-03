/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/MMT_Diamond.h"

MMT_Diamond::MMT_Diamond(const MuonGM::MuonDetectorManager* detManager): AthMessaging(Athena::getMessageSvc(), "MMT_Diamond") {
  m_detManager = detManager;
}

void MMT_Diamond::clearEvent() {
  if (!m_diamonds.empty()) {
    for (auto &diam : m_diamonds) {
      if (!diam.ev_roads.empty()) diam.ev_roads.clear();
      if (!diam.ev_hits.empty()) diam.ev_hits.clear();
      diam.slopes.clear();
    }
    m_diamonds.clear();
  }
}

void MMT_Diamond::createRoads_fillHits(const unsigned int iterator, std::vector<hitData_entry> &hitDatas, const MuonGM::MuonDetectorManager* detManager, std::shared_ptr<MMT_Parameters> par, const int phi) {
  ATH_MSG_DEBUG("createRoads_fillHits: Feeding hitDatas Start");

  diamond_t entry;
  entry.wedgeCounter = iterator;
  entry.sector = par->getSector();
  entry.stationPhi = (par->getSector() == 'S') ? phi*2-1 : phi*2-2;

  std::string sector = (par->getSector() == 'L') ? "MML" : "MMS";

  /*
   * The following for-loop merges all plane global coordinates in one single shot:
   * X & Y are constant in all layers, for a given phi (Not used at the moment)
   * Z (layer coordinates) changes only for Small and Large sectors --> USED and working!
   */
  std::vector<ROOT::Math::XYZVector> planeCoordinates{};
  planeCoordinates.reserve(8);
  for (unsigned int iphi = 0; iphi < 8; iphi++) {
    Amg::Vector3D globalPos(0.0, 0.0, 0.0);
    int multilayer = (iphi < 4) ? 1 : 2;
    int gasgap = ((iphi+1)%4 == 0) ? 4 : (iphi+1)%4;
    /*
     * Strip 100 (or 200) chosen as good compromise, considering offline strips.
     * channelMin() function (https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/MuonSpectrometer/MuonIdHelpers/MuonIdHelpers/MmIdHelper.h?v=21.3#0123)
     * could be used, instead of hardcoding the strip id, but it returns (always?) as initialized in level ranges
     */
    int strip = (iphi > 1 && iphi < 6) ? par->getMissedBottomStereoStrips()+1 : par->getMissedBottomEtaStrips()+1;
    Identifier strip_id = detManager->mmIdHelper()->channelID(sector, 1, iphi+1, multilayer, gasgap, strip);
    const MuonGM::MMReadoutElement* readout = detManager->getMMReadoutElement(strip_id);

    ROOT::Math::XYZVector coord(0.,0.,0.);
    if (readout->stripGlobalPosition(strip_id, globalPos)) coord.SetXYZ(globalPos.x(), globalPos.y(), globalPos.z());
    else ATH_MSG_WARNING("Wedge " << sector << " phi: " << iphi << " mult. " << multilayer << " gas " << gasgap <<  " | Unable to retrieve global positions");
    planeCoordinates.push_back(coord);
  }

  int nroad = 8192/this->getRoadSize();
  static const double B = (1./std::tan(1.5/180.*M_PI));
  int uvfactor = std::round( par->getlWidth() / (B * 0.4 * 2.)/this->getRoadSize() ); // full wedge has to be considered, i.e. S(L/M)2
  this->setUVfactor(uvfactor);

  for (const auto &hit_entry : hitDatas) {
    auto myhit = std::make_shared<MMT_Hit>(hit_entry, detManager, par, planeCoordinates);
    if (myhit->verifyHit()) {
      m_hitslopes.push_back(myhit->getRZSlope());
      entry.ev_hits.push_back(myhit);
    }
  }
  entry.side = (std::all_of(entry.ev_hits.begin(), entry.ev_hits.end(), [] (const auto &hit) { return hit->getStationEta() < 0; })) ? 'C' : 'A';

  for (int i = 0; i < nroad; i++) {
    auto myroad = std::make_shared<MMT_Road>(sector[2], m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV,
                                             m_xthr, m_uvthr, i);
    entry.ev_roads.push_back(myroad);

    int nuv = (this->getUV()) ? this->getUVfactor() : 0;
    for (int uv = 1; uv <= nuv; uv++) {
      if (i-uv < 0) continue;

      auto myroad_0 = std::make_shared<MMT_Road>(sector[2], m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV,
                                                 m_xthr, m_uvthr, i, i+uv, i-uv);
      entry.ev_roads.push_back(myroad_0);

      auto myroad_1 = std::make_shared<MMT_Road>(sector[2], m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV,
                                                 m_xthr, m_uvthr, i, i-uv, i+uv);
      entry.ev_roads.push_back(myroad_1);

      auto myroad_2 = std::make_shared<MMT_Road>(sector[2], m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV,
                                                 m_xthr, m_uvthr, i, i+uv-1, i-uv);
      entry.ev_roads.push_back(myroad_2);

      auto myroad_3 = std::make_shared<MMT_Road>(sector[2], m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV,
                                                 m_xthr, m_uvthr, i, i-uv, i+uv-1);
      entry.ev_roads.push_back(myroad_3);

      auto myroad_4 = std::make_shared<MMT_Road>(sector[2], m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV,
                                                 m_xthr, m_uvthr, i, i-uv+1, i+uv);
      entry.ev_roads.push_back(myroad_4);

      auto myroad_5 = std::make_shared<MMT_Road>(sector[2], m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV,
                                                 m_xthr, m_uvthr, i, i+uv, i-uv+1);
      entry.ev_roads.push_back(myroad_5);
    }
  }
  m_diamonds.push_back(entry);
  ATH_MSG_DEBUG("CreateRoadsAndFillHits: Feeding hitDatas Ended");
}

void MMT_Diamond::findDiamonds(const unsigned int iterator, const int event) {
  if (m_diamonds[iterator].ev_hits.empty()) return;

  auto t0 = std::chrono::high_resolution_clock::now();
  int bc_start = 999999;
  int bc_end = -1;
  int bc_wind = 4; // fixed time window (in bunch crossings) during which the algorithm collects ART hits
  unsigned int ibc = 0;

  m_diamonds[iterator].slopes.clear();

  // Comparison with lambda function (easier to implement)
  std::sort(m_diamonds[iterator].ev_hits.begin(), m_diamonds[iterator].ev_hits.end(), [](const auto &h1, const auto &h2){ return h1->getBC() < h2->getBC(); });
  bc_start = m_diamonds[iterator].ev_hits.front()->getBC();
  bc_end = m_diamonds[iterator].ev_hits.front()->getBC() + 16;
  ATH_MSG_DEBUG("Window Start: " << bc_start << " - Window End: " << bc_end);

  for (const auto &road : m_diamonds[iterator].ev_roads) road->reset();

  std::vector<std::shared_ptr<MMT_Hit> > hits_now = {};
  std::vector< std::pair<int, float> > vmm_same = {};
  std::vector< std::pair<int, int> > addc_same = {};
  std::vector<int> to_erase = {};
  int n_addc = 4;
  int n_vmm  = 128;

  // each road makes independent triggers, evaluated on each BC
  for (int bc = m_diamonds[iterator].ev_hits.front()->getBC(); bc < bc_end; bc++) {
    // Cleaning stuff
    hits_now.clear();

    for (unsigned int j = ibc; j < m_diamonds[iterator].ev_hits.size(); j++) {
      if (m_diamonds[iterator].ev_hits[j]->getBC() == bc) hits_now.push_back(m_diamonds[iterator].ev_hits[j]);
      else if (m_diamonds[iterator].ev_hits[j]->getBC() > bc) {
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
        for (auto hit_pointer: hits_now) {
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

    for (auto &road : m_diamonds[iterator].ev_roads) {
      road->incrementAge(bc_wind);
      if (!hits_now.empty()) road->addHits(hits_now);

      if (road->checkCoincidences(bc_wind) && bc >= (bc_start - 1)) {

        ATH_MSG_DEBUG("------------------------------------------------------------------");
        ATH_MSG_DEBUG("Coincidence FOUND @BC: " << bc);
        ATH_MSG_DEBUG("Road (x, u, v, count): (" << road->iRoadx() << ", " << road->iRoadu() << ", " << road->iRoadv() << ", " << road->countHits() << ")");
        ATH_MSG_DEBUG("------------------------------------------------------------------");

        std::vector<int> bcidVec;
        for (const auto &hit: road->getHitVector()) {
          bcidVec.push_back(hit->getBC());
        }
        std::sort(bcidVec.begin(), bcidVec.end());

        // evaluating mode of the BCID of the hits in the diamond
        // default setting in the firmware is the mode of the hits's bcid in the diamond
        int bcidVal=bcidVec.at(0), bcidCount=1, modeCount=1, bcidMode=bcidVec.at(0);
        for (unsigned int i=1; i<bcidVec.size(); i++){
          if (bcidVec.at(i) == bcidVal){
            bcidCount++;
          } else {
            bcidCount = 1;
            bcidVal = bcidVec.at(i);
          }
          if (bcidCount > modeCount) {
            modeCount = bcidCount;
            bcidMode = bcidVal;
          }
        }

        slope_t slope;
        slope.event = event;
        slope.BC = bcidMode;
        slope.totalCount = road->countHits();
        slope.realCount = road->countRealHits();
        slope.iRoad = road->iRoadx();
        slope.iRoadu = road->iRoadu();
        slope.iRoadv = road->iRoadv();
        slope.uvbkg = road->countUVHits(true); // the bool in the following 4 functions refers to background/noise hits
        slope.xbkg = road->countXHits(true);
        slope.uvmuon = road->countUVHits(false);
        slope.xmuon = road->countXHits(false);
        slope.age = slope.BC - bc_start;
        slope.mxl = road->mxl();
        slope.my = road->avgSofX(); // defined as my in ATL-COM-UPGRADE-2015-033
        slope.uavg = road->avgSofUV(2,4);
        slope.vavg = road->avgSofUV(3,5);
	static const double tan_stereo_angle = std::tan(0.02618); // The stereo angle is fixed and can be hardcoded
        slope.mx = (slope.uavg-slope.vavg)/(2.*tan_stereo_angle);
        double theta = std::atan(std::sqrt(std::pow(slope.mx,2) + std::pow(slope.my,2)));
        slope.theta = (slope.my > 0.) ? theta : M_PI - theta;
        slope.eta = -1.*std::log(std::tan(slope.theta/2.));
        slope.dtheta = (slope.mxl - slope.my)/(1. + slope.mxl*slope.my);
        slope.side = (slope.my > 0.) ? 'A' : 'C';
        double phi = std::atan(slope.mx/slope.my);
        double phiShifted = this->phiShift(this->getDiamond(iterator).stationPhi, phi, slope.side);
        slope.phi = phi;
        slope.phiShf = phiShifted;
        slope.lowRes = road->evaluateLowRes();

        m_diamonds[iterator].slopes.push_back(slope);
      }
    }
  }
  auto t1 = std::chrono::high_resolution_clock::now();
  ATH_MSG_DEBUG("Processing roads took " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms");
}

double MMT_Diamond::phiShift(const int n, const double phi, const char side) const {
  double Phi = (side == 'A') ? -phi : phi;
  double shift = (n > 8) ? (16-n)*M_PI/8. : n*M_PI/8.;
  if (n < 8)       return (Phi + shift);
  else if (n == 8) return (Phi + ((Phi > 0.) ? -1. : 1.)*shift);
  else             return (Phi - shift);
}

void MMT_Diamond::resetSlopes() {
  if (!m_hitslopes.empty()) m_hitslopes.clear();
}

slope_t::slope_t(int ev, int bc, unsigned int tC, unsigned int rC, int iX, int iU, int iV, unsigned int uvb, unsigned int xb, unsigned int uvm, unsigned int xm,
                 int age, double mxl, double my, double uavg, double vavg, double mx, double th, double eta, double dth, char side, double phi, double phiS,
                 bool lowRes) :
  event(ev), BC(bc), totalCount(tC), realCount(rC), iRoad(iX), iRoadu(iU), iRoadv(iV), uvbkg(uvb), xbkg(xb), uvmuon(uvm), xmuon(xm),
  age(age), mxl(mxl), my(my), uavg(uavg), vavg(vavg), mx(mx), theta(th), eta(eta), dtheta(dth), side(side), phi(phi), phiShf(phiS), lowRes(lowRes) {}
