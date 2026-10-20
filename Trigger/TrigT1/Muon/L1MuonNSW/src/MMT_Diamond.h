/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef L1MUONNSW_MMT_DIAMOND_H
#define L1MUONNSW_MMT_DIAMOND_H

#include "AthenaBaseComps/AthMessaging.h"
#include "MMT_Road.h"
#include "L1MMDefinitions.h"
#include <cmath>
#include <cstdint>
#include <vector>

namespace L1Muon::L1MM {

  /**
   * @brief One MM trigger candidate as found by MMT_Diamond::findDiamonds: road indices, hit counts,
   *        track slopes and the reconstructed theta, eta, phi and Delta theta.
   */
  struct slope_t {
    uint64_t event{0};
    int BC{-1};
    unsigned int totalCount{999};
    int iRoad{-1};
    int iRoadu{-1};
    int iRoadv{-1};
    unsigned int xCount{999};
    unsigned int uCount{999};
    int age{-1};
    double mxl{999};
    double my{999};
    double uavg{999};
    double vavg{999};
    double mx{999};
    double theta{999};
    double eta{999};
    double dtheta{999};
    double phi{999};
    double phiShf{999};
    SideType side{SideType::Undefined};
    bool lowRes{false};
  };

  /**
   * @brief Diamond Road algorithm of the MM trigger processor: builds the roads of one MM station
   *        (createRoads) and finds the coincidences among its hits (findDiamonds).
   *
   * Copy of the Run 3 NSWL1::MMT_Diamond (Trigger/TrigT1/TrigT1NSWSimTools), logic unchanged.
   */
  class MMT_Diamond : public AthMessaging {
    public:
      MMT_Diamond(const int diamXthreshold, const bool uv, const int diamUVthreshold, const int roadSize,
                  const int olapEtaUp, const int olapEtaDown, const int olapStereoUp, const int olapStereoDown);
      ~MMT_Diamond() = default;

      void createRoads(std::vector<MMT_Road>& roads, const bool isLarge, const bool isEta1) const;
      void findDiamonds(std::vector<MMT_Hit>& hits, std::vector<MMT_Road>& roads, std::vector<slope_t>& diamondSlopes,
                        const int sectorPhi, const double tanStereoAngle) const;
      double phiShift(const int n, const double phi, const SideType side) const;

      int getRoadSize() const { return m_roadSize; }
      unsigned int getXthreshold() const { return m_xthr; }
      unsigned int getUVthreshold() const { return m_uvthr; }

    private:
      bool m_uvflag{};
      int m_roadSize{};
      int m_roadSizeUpX{};
      int m_roadSizeDownX{};
      int m_roadSizeUpUV{};
      int m_roadSizeDownUV{};
      int m_xthr{};
      int m_uvthr{};
  };

}  // namespace L1Muon::L1MM

#endif  // L1MUONNSW_MMT_DIAMOND_H
