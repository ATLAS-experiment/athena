/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthTrackSmearer.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"

namespace L1Muon {

TruthTrackSmearer::TruthTrackSmearer(ATHRNG::RNGWrapper* rngWrapper)
: m_rngWrapper(rngWrapper) {
  m_efficiencyMap[0] = 0.0;   // pT < 3 GeV
  m_efficiencyMap[1] = 0.95;  // pT > 3 GeV
}

double TruthTrackSmearer::effFunc(double pt) const {
  // TODO: Efficiency is changed at only 3 GeV...
  return (pt < 3000.) ? m_efficiencyMap[0] : m_efficiencyMap[1];
}
  
bool TruthTrackSmearer::emulateL0MuonTrack(double curv, float eta, float phi, L0MuonTrack& otrack) const {
  // input curv(q/pT) is in MeV
  CLHEP::HepRandomEngine* engine = m_rngWrapper->getEngine(Gaudi::Hive::currentContext());

  // efficiency emulation
  double abspt = std::abs(1.0 / curv);
  if (CLHEP::RandFlat::shoot(engine) > effFunc(abspt)) return false;

  // TODO: extrapolate to the pivot plane

  // position efficiency map
  if (std::abs(eta) > 2.41) return false;  // for the time being...

  // TODO: The position is NOT smeared here - since the binning of RoIs could be larger. To be checked.

  // pt smearing
  double sigma = curv * 0.05;  // TODO: 5% uniform for the time being...

  double gencurv = CLHEP::RandGauss::shoot(engine, curv, sigma);

  otrack.setTrack(gencurv, eta, phi);

  return true;
}

}   // end of namespace
