/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_RPAPATFINDER_H
#define  TRIGL2MUONSA_RPAPATFINDER_H

#include <string> 
#include <list> 

#include "AthenaBaseComps/AthAlgTool.h"

// Original author: Massimo Corradi

namespace TrigL2MuonSA {

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
struct RpcLayerHits
{
  std::vector<std::vector<double>> hits_in_layer_eta;  
  std::vector<std::vector<double>> hits_in_layer_phi;  
  std::vector<std::vector<double>> hits_in_layer_Z;  
  std::vector<std::vector<double>> hits_in_layer_R; 
  void clear() {
    hits_in_layer_eta.assign(8,std::vector<double> {});
    hits_in_layer_phi.assign(8,std::vector<double> {});
    hits_in_layer_R.assign(8,std::vector<double> {});
    hits_in_layer_Z.assign(8,std::vector<double> {});
  }
};

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------


class RpcPatFinder: public AthAlgTool
{

 public:

  using AthAlgTool::AthAlgTool;

 public:

  void addHit(const std::string& stationName,
	      int stationEta,
	      bool  measuresPhi,
	      unsigned int  gasGap,
	      unsigned int doubletR,
	      double gPosX, double gPosY, double gPosZ,
        TrigL2MuonSA::RpcLayerHits& rpcLayerHits) const;
  bool findPatternEta(
    std::array<std::reference_wrapper<double>, 3>& result_aw, 
    std::array<std::reference_wrapper<double>, 3>& result_bw,  
    const TrigL2MuonSA::RpcLayerHits& rpcLayerHits) const;

  bool findPatternPhi(double &phi_middle, double &phi_outer, const TrigL2MuonSA::RpcLayerHits& rpcLayerHits) const;
  
 private:
  bool deltaOK(int l1, int l2, double x1, double x2, int isphi, double &delta) const;  
  double calibR(const std::string& stationName, double R, double Phi) const;  
  void abcal(const std::bitset<8>& result_pat, 
             const std::array<size_t, 8>& index, 
             std::array<std::reference_wrapper<double>, 3>& aw, 
             std::array<std::reference_wrapper<double>, 3>& bw,  
             const TrigL2MuonSA::RpcLayerHits& rpcLayerHits) const;
};

}
#endif

