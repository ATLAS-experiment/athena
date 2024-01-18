/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaUtils/egPhotonWrtPoint.h"

#include "egammaUtils/ShowerDepthTool.h"
#include "xAODEgamma/Egamma.h"

photonWrtPoint::PtEtaPhi photonWrtPoint::PtEtaPhiWrtZ(const xAOD::Egamma& ph,
                                                      double z) {
  std::pair<double, float> RZ1 = {0., 0.};
  float etaBE1 = ph.caloCluster()->etaBE(1);
  if(std::abs(etaBE1) < 10) RZ1 = CP::ShowerDepthTool::getRZ(etaBE1, 1);

  double rCalo = RZ1.first;
  double zCalo = RZ1.second;
  double correctedZ = zCalo - z;
  double eta = std::asinh(correctedZ / rCalo);
  return {ph.e() / std::cosh(eta), eta, ph.phi()};
}

void photonWrtPoint::correctForZ(xAOD::Egamma& ph, double z) {
  auto corr = photonWrtPoint::PtEtaPhiWrtZ(ph, z);
  ph.setP4(corr.pt, corr.eta, corr.phi, ph.m());
}

