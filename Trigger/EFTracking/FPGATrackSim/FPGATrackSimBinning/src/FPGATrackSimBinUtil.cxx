// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimBinUtil.cxx
 * @author Elliot Lipeles
 * @date Feb 13, 2025
 * @brief See header file.
 */

#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"
#include "FourMomUtils/xAODP4Helpers.h"


namespace FPGATrackSimBinUtil {

// ----------------------------------------------------------------------------------------
// IdxSet and ParSet implementations
// ----------------------------------------------------------------------------------------
ParSet::ParSet(const std::vector<double> &val) {
  if (val.size() != 5) {
    throw std::invalid_argument(
        "Not enough parameters in ParSet initialization");
  }
  std::copy(val.begin(), val.end(), this->begin());
}
ParSet::operator const std::vector<double>() const {
  return std::vector<double>(this->begin(), this->end());
}

IdxSet::IdxSet(const std::vector<unsigned> &val) {
  if (val.size() != 5) {
    throw std::invalid_argument(
        "Not enough parameters in IdxSet initialization");
  }
  std::copy(val.begin(), val.end(), this->begin());
}
IdxSet::operator const std::vector<unsigned>() const {
  return std::vector<unsigned>(this->begin(), this->end());
}

// ----------------------------------------------------------------------------------------
// StoredHit implementations
// ----------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream &os, const StoredHit &hit)
{
  os << "lyr: " << hit.layer << " ";
  os << "(" << hit.hitptr->getR() << ", " << hit.hitptr->getGPhi() << ", " << hit.hitptr->getZ() << ") ";
  os << "[" << hit.phiShift << ", " << hit.etaShift << "]";
  return os;
}

double StoredHit::rzrad() const {
  return sqrt(hitptr->getR()*hitptr->getR()+hitptr->getZ()*hitptr->getZ());
}


// ----------------------------------------------------------------------------------------
// SubVec implementations
// ----------------------------------------------------------------------------------------
std::vector<unsigned> subVec(const std::vector<unsigned> &elems,
                             const IdxSet &invec) {
  std::vector<unsigned> retv;
  for (auto elem : elems) {
    retv.push_back(invec[elem]);
  }
  return retv;
}

void setIdxSubVec(IdxSet &idx, const std::vector<unsigned> &subvecelems,
                  const std::vector<unsigned> &subvecidx) {

  if (subvecelems.size() != subvecidx.size()) {
    throw std::invalid_argument(
        "Setting FPGATrackSimGenScanBinningBase::setIdxSubVec with mismatched "
        "sizes");
  }

  for (unsigned i = 0; i < subvecelems.size(); i++) {
    if (subvecelems[i] >= idx.size()) {
      throw std::invalid_argument(
          "FPGATrackSimGenScanBinningBase::setIdxSubVec input out of range");
    }
    idx[subvecelems[i]] = subvecidx[i];
  }
}

// ----------------------------------------------------------------------------------------
// makeVariationSet implementation
// ----------------------------------------------------------------------------------------
// This gives a list tracks parameters for the corners of bin of dimensions
// scanpars.size()
std::vector<IdxSet> makeVariationSet(const std::vector<unsigned> &scanpars,
                                     const IdxSet &idx) {
  std::vector<IdxSet> retv;
  for (unsigned corners = 0; corners < unsigned((1 << scanpars.size()));
       corners++) {
    IdxSet newidx = idx;
    int scanDimCnt = 0;
    for (auto &par : scanpars) {
      newidx[par] = idx[par] + ((corners >> scanDimCnt) & 1);
      scanDimCnt++;
    }
    retv.push_back(newidx);
  }
  return retv;
}

// ----------------------------------------------------------------------------------------
//  StreamManager implementation
// ----------------------------------------------------------------------------------------
// Class for writing const files formatted for firmware
StreamManager::~StreamManager() {
  for (auto &f : m_map) {
    f.second << "\n";
  }
}

// FW constants writer
std::ostream &operator<<(std::ostream &os, const std::vector<unsigned>& idx) {
  bool first = true;
  for (auto &val : idx) {
    if (!first)
      os << ",";
    os << val;
    first = false;
  }
  return os;
}



//-------------------------------------------------------------------------------------------------------
//
// Geometry Helpers -- does basic helix calculations
//
//-------------------------------------------------------------------------------------------------------
double GeomHelpers::ThetaFromEta(double eta)
{
    return 2.0 * atan(exp(-1.0 * eta));
}

double GeomHelpers::EtaFromTheta(double theta)
{
    return -log(tan(theta / 2.0));
}




double GeomHelpers::zFromPars(double r, const FPGATrackSimTrackPars &pars)
{
    double theta = ThetaFromEta(pars.eta);
    double zhit = pars.z0 + r / tan(theta);
    if (std::abs(pars.qOverPt) > 0)
    {
        zhit = pars.z0 + 1.0 / tan(theta) * asin(r * CurvatureConstant * pars.qOverPt) / (CurvatureConstant * pars.qOverPt);
    }
    return zhit;
}

double GeomHelpers::dPhiHitTrkFromPars(double r, const FPGATrackSimTrackPars &pars) {
    return asin(r * CurvatureConstant * pars.qOverPt - pars.d0 / r);
}

double GeomHelpers::phiFromPars(double r, const FPGATrackSimTrackPars &pars) {
    double phi_hit = xAOD::P4Helpers::deltaPhi(pars.phi,dPhiHitTrkFromPars(r,pars));
    return phi_hit;
}

double GeomHelpers::parsToTrkPhi(const FPGATrackSimTrackPars &pars, FPGATrackSimHit const *hit)
{
    double r = hit->getR();          // mm
    double phi_hit = hit->getGPhi(); // radians
    double phi_trk = xAOD::P4Helpers::deltaPhi(phi_hit,dPhiHitTrkFromPars(r,pars));
    return phi_trk;
}

double GeomHelpers::dZdEta(double eta)
{
  // dtheta/deta = -sin(theta)
  return -1.0/std::sin(ThetaFromEta(eta));
}

double GeomHelpers::dPhidQOverPt(double hitr)
{
  // full answer = rA/cos(dphi)
  // approx answer = rA  b/c  dphi is small (~0.4 max)
  return hitr*CurvatureConstant;
}

} // namespace FPGATrackSimBinUtil
