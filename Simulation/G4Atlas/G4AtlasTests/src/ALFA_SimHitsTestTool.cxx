/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ALFA_SimHitsTestTool.h"

#include "ALFA_SimEv/ALFA_HitCollection.h"


#include <TH2D.h>
#include <TH1.h>
#include <TProfile.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <array>

namespace{
  template <class T, std::size_t N> using A = std::array<T,N>;
}


ALFA_SimHitsTestTool::ALFA_SimHitsTestTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SimTestToolBase(type, name, parent)
{
  for (int i(0); i<8; ++i) {
    m_E_full_sum_h[i]=nullptr;
    m_E_layer_sum_h[i]=nullptr;
    m_hit_map_h[i]=nullptr;
  }
}

StatusCode ALFA_SimHitsTestTool::initialize()
{
  m_path+="ALFA/";

  _TH1D(m_E_gvs,"edep_full_in_gvs", 8, -0.5, 7.5);
  _SET_TITLE(m_E_gvs, "Energy deposit in GVS","Station","E (MeV)");

  std::stringstream s;
  for (unsigned int j=0; j<8; j++) {
    s.str("");
    s << j+1;

    _TH1D(m_E_full_sum_h[j],("edep_full_in_det_no."+s.str()).c_str(), 100, 0., 5.);
    _SET_TITLE(m_E_full_sum_h[j], "Energy deposit in full detector","E (MeV)","N");

    _TH2D(m_E_layer_sum_h[j],("edep_per_layer_detector_no."+s.str()).c_str(), 100, 0., 0.5, 22, 0., 21.);
    _SET_TITLE(m_E_layer_sum_h[j], "Energy deposit in layers","E (MeV)","layer");

    _TH2D(m_hit_map_h[j],("hitmap_det_no."+s.str()).c_str(), 22, 0., 21., 66, 0., 65.);
    _SET_TITLE(m_hit_map_h[j], "hit map - layer vs fiber","layer","fiber");
  }

  return StatusCode::SUCCESS;
}

StatusCode ALFA_SimHitsTestTool::processEvent() {

  int fiber{}, plate{}, sign{}, station{};
  //
  // cleaning
  constexpr int station_max{8};
  constexpr int plate_max{10};
  constexpr int fiber_max{64};

  //avoid large stack use; E_fiber_sum would use 81920 bytes
  using FiberArr = A<A<A<A<double,2>, fiber_max>, plate_max>,station_max>;
  auto pE_fiber_sum = std::make_unique<FiberArr>();
  auto & E_fiber_sum = *pE_fiber_sum;
  double E_full_sum[station_max]{};
  double E_layer_sum[station_max][20]{};


  const ALFA_HitCollection* coll_handle = nullptr;
  CHECK( evtStore()->retrieve(coll_handle,"ALFA_HitCollection") );


  for (const ALFA_Hit& hit : *coll_handle) {

    station = hit.GetStationNumber();
    plate   = hit.GetPlateNumber();
    fiber   = hit.GetFiberNumber();
    sign    = hit.GetSignFiber();
    if (plate==100) {
      // Treat hits in GVS separately.
      m_E_gvs->Fill(station,(hit.GetEnergyDeposit()));
      continue;
    }
    E_fiber_sum[station-1][plate-1][fiber-1][(1-sign)/2] += (hit.GetEnergyDeposit());
  }

  for ( int l = 0; l < station_max; l++ ) {
      for ( int i = 0; i < plate_max; i++ ){
          for ( int j = 0; j < fiber_max;  j++ ) {
              for ( int k = 0; k < 2; k++ ) {
                E_full_sum[l] += E_fiber_sum[l][i][j][k];
                E_layer_sum[l][2*i+k] += E_fiber_sum[l][i][j][k];
                if (E_fiber_sum[l][i][j][k] > 0.) {
                  m_hit_map_h[l]->Fill(2*i+k+1,j+1);
                }
              }
          }
      }
  }

  for ( int l = 0; l < station_max; l++ ) {
    m_E_full_sum_h[l]->Fill(E_full_sum[l]);
    for ( int i = 0; i < 2*plate_max; i++ ) {
      m_E_layer_sum_h[l]->Fill(E_layer_sum[l][i],i+1);
    }
  }

  return StatusCode::SUCCESS;
}
