/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4BARREL_ACCMAP_H
#define LARG4BARREL_ACCMAP_H

#include "CurrMap.h"
#include <string>
#include <array>
#include <memory>

class AccMap {
public:
  ~AccMap() = default;
  static const AccMap* GetAccMap();

  // Use constexpr for fixed dimensions
  static constexpr int MAX_FOLDS = 23;    // Folds 0-13 and straight 21-22
  static constexpr int MAX_REGIONS = 10;  // Electronic regions 0-9
  static constexpr int N_MAX_VEC = 14;    // Original m_nmax limit

  int  Region(int region, int sampling, int eta) const noexcept;
  const CurrMap* GetMap(int ifold, int ielecregion) const noexcept;
  const CurrMap* GetMap(int ifold, int region, int sampling, int eta) const noexcept;
  float GetXmin(int ifold) const { return (ifold >= 0 && ifold < N_MAX_VEC) ? m_xmin[ifold] : -999.0f; }
  float GetXmax(int ifold) const { return (ifold >= 0 && ifold < N_MAX_VEC) ? m_xmax[ifold] : -999.0f; }
  float GetYmin(int ifold) const { return (ifold >= 0 && ifold < N_MAX_VEC) ? m_ymin[ifold] : -999.0f; }
  float GetYmax(int ifold) const { return (ifold >= 0 && ifold < N_MAX_VEC) ? m_ymax[ifold] : -999.0f; }
private:
  AccMap();
  std::array<std::array<std::unique_ptr<CurrMap>, MAX_REGIONS>, MAX_FOLDS> m_fastMap;
  std::array<float, N_MAX_VEC> m_xmin{};
  std::array<float, N_MAX_VEC> m_xmax{};
  std::array<float, N_MAX_VEC> m_ymin{};
  std::array<float, N_MAX_VEC> m_ymax{};

#ifdef LARG4_STAND_ALONE
public:
  void SetDirectory(const std::string& dir) { m_directory=dir; }
private:
  std::string m_directory{"/afs/cern.ch/atlas/offline/data/lar/calo_data"};
#endif

};
#endif
