// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimGenScanBinning.cxx
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief See header file.
 */

#include "FPGATrackSimGenScanBinning.h"

//--------------------------------------------------------------------------------------------------
//
//   FPGATrackSimGenScanBinningBase base class implementation
//
//--------------------------------------------------------------------------------------------------

bool FPGATrackSimGenScanBinningBase::inRange(const ParSet &pars) const
{
    for (unsigned i = 0; i < NPars; i++)
    {
        if (!inRange(i, pars[i]))
            return false;
    }
    return true;
}

FPGATrackSimGenScanBinningBase::IdxSet FPGATrackSimGenScanBinningBase::binIdx(const ParSet &pars) const
{
    IdxSet retv;
    for (unsigned i = 0; i < NPars; i++)
    {
        retv[i] = binIdx(i, pars[i]);
    }
    return retv;
}
const FPGATrackSimGenScanBinningBase::IdxSet FPGATrackSimGenScanBinningBase::parsToBin(FPGATrackSimTrackPars &pars) const
{
    ParSet parset = trackParsToParSet(pars);
    if (inRange(parset))
    {
        return binIdx(parset);
    }
    return m_invalidBin;
}

FPGATrackSimGenScanBinningBase::ParSet FPGATrackSimGenScanBinningBase::center() const
{
    ParSet parset;
    for (unsigned i = 0; i < NPars; i++)
    {
        parset[i] = (m_parMin[i] + m_parMin[i]) / 2.0;
    }
    return parset;
}

FPGATrackSimGenScanBinningBase::ParSet FPGATrackSimGenScanBinningBase::binLowEdge(const IdxSet &idx) const
{
    ParSet parset;
    for (unsigned i = 0; i < NPars; i++)
    {
        parset[i] = binLowEdge(i, idx[i]);
    }
    return parset;
}

FPGATrackSimGenScanBinningBase::ParSet FPGATrackSimGenScanBinningBase::binCenter(const IdxSet &idx) const
{
    ParSet parset;
    for (unsigned i = 0; i < NPars; i++)
    {
        parset[i] = binCenter(i, idx[i]);
    }
    return parset;
}

std::vector<unsigned> FPGATrackSimGenScanBinningBase::subVec(const std::vector<unsigned>& elems, const IdxSet& invec) const
{
    std::vector<unsigned> retv;
    for (auto elem : elems)
    {
        retv.push_back(invec[elem]);
    }
    return retv;
}

StatusCode FPGATrackSimGenScanBinningBase::setIdxSubVec(IdxSet &idx, const std::vector<unsigned>& subvecelems, const std::vector<unsigned>& subvecidx) const
{
    if (subvecelems.size()!=subvecidx.size()) {
        return StatusCode::FAILURE;
    }
    for (unsigned i = 0; i < subvecelems.size(); i++)
    {        
        if (subvecelems[i] >= idx.size()) {
            return StatusCode::FAILURE;
        }
        idx[subvecelems[i]] = subvecidx[i];
    }
    return StatusCode::SUCCESS;
}


// This gives a list tracks parameters for the corners of bin of dimensions scanpars.size()
std::vector<FPGATrackSimGenScanBinningBase::ParSet> FPGATrackSimGenScanBinningBase::makeVariationSet(const std::vector<unsigned> &scanpars, const IdxSet &idx) const
{
  std::vector<ParSet> retv;
  for (unsigned corners = 0; corners < unsigned((1 << scanpars.size())); corners++)
  {
    IdxSet newidx = idx;
    int scanDimCnt = 0;
    for (auto &par : scanpars)
    {
      newidx[par] = idx[par] + ((corners >> scanDimCnt) & 1);
      scanDimCnt++;
    }
    retv.push_back(binLowEdge(newidx));
  }
  return retv;
}

// find valid range for sliceVar
bool FPGATrackSimGenScanBinningBase::hitInSlice(const FPGATrackSimGenScanBinningBase::IdxSet &idx, FPGATrackSimHit const *hit) const
{
  // Get float values
  double slicevarmin = std::numeric_limits<double>::max();
  double slicevarmax = std::numeric_limits<double>::min();

  auto variations = makeVariationSet(slicePars(), idx);

  for (auto &pars : variations)
  {
    double slicevarhit = sliceVarExpected(pars, hit);
    slicevarmin = std::min(slicevarmin, slicevarhit);
    slicevarmax = std::max(slicevarmax, slicevarhit);
  }
  double slicevar = sliceVar(hit);
  return ( slicevar > slicevarmin) && (slicevar <  slicevarmax);
}



std::pair<unsigned, unsigned> FPGATrackSimGenScanBinningBase::idxsetToRowParBinRange(const FPGATrackSimGenScanBinningBase::IdxSet &idx, FPGATrackSimHit const *hit) const
{
  // Get float values
  double rowparmin = std::numeric_limits<double>::max();
  double rowparmax = std::numeric_limits<double>::min();

  auto variations = makeVariationSet(scanPars(), idx);

  for (auto &pars : variations)
  {
    double rowparhit = rowPar(pars, hit);
    rowparmin = std::min(rowparmin, rowparhit);
    rowparmax = std::max(rowparmax, rowparhit);
  }

  // only consider parameter bins in range
  unsigned lowbin =  std::max(rowParBinIdx(rowparmin), unsigned(0));
  unsigned highbin = std::min(rowParBinIdx(rowparmax) + 1, m_parBins[rowParIdx()] );

  return {lowbin, highbin};
}

// Default implementations
double FPGATrackSimGenScanBinningBase::sliceVar([[maybe_unused]] FPGATrackSimHit const *hit) const { return 0.0; };
double FPGATrackSimGenScanBinningBase::sliceVarExpected([[maybe_unused]] const ParSet &pars, [[maybe_unused]] FPGATrackSimHit const *hit) const {return 0.0; };
double FPGATrackSimGenScanBinningBase::rowPar([[maybe_unused]] const ParSet &pars, [[maybe_unused]] FPGATrackSimHit const *hit) const {return 0.0; };
    

//--------------------------------------------------------------------------------------------------
//
// Geometry Helpers 
//
//--------------------------------------------------------------------------------------------------

double FPGATrackSimGenScanGeomHelpers::ThetaFromEta(double eta)
{
    return 2.0 * atan(exp(-1.0 * eta));
}

double FPGATrackSimGenScanGeomHelpers::EtaFromTheta(double theta)
{
    return -log(tan(theta / 2.0));
}

double FPGATrackSimGenScanGeomHelpers::zFromPars(double r, const FPGATrackSimTrackPars &pars)
{
    double theta = ThetaFromEta(pars.eta);
    double zhit = pars.z0 + r / tan(theta);
    if (std::abs(pars.qOverPt) > 0)
    {
        zhit = pars.z0 + 1.0 / tan(theta) * asin(r * CurvatureConstant * pars.qOverPt) / (CurvatureConstant * pars.qOverPt);
    }
    return zhit;
}

double FPGATrackSimGenScanGeomHelpers::phiFromPars(double r, const FPGATrackSimTrackPars &pars)
{
    double phi_hit = xAOD::P4Helpers::deltaPhi(pars.phi,asin(r * CurvatureConstant * pars.qOverPt - pars.d0 / r));

    return phi_hit;
}

double FPGATrackSimGenScanGeomHelpers::parsToTrkPhi(const FPGATrackSimTrackPars &pars, FPGATrackSimHit const *hit)
{
    double r = hit->getR();          // mm
    double phi_hit = hit->getGPhi(); // radians
    double phi_trk = xAOD::P4Helpers::deltaPhi(phi_hit,asin(r * CurvatureConstant * pars.qOverPt - pars.d0 / r));    
    return phi_trk;
}

//
//
// Tool for doing Key Layer Math
//
//

std::pair<double, double> FPGATrackSimGenScanKeyLyrHelper::getXY(double r, double phi) const
{
    return std::pair<double, double>(r * cos(phi), r * sin(phi));
}

std::pair<double, double> FPGATrackSimGenScanKeyLyrHelper::getRotationAngles(const std::pair<double, double>& xy1, const std::pair<double, double>& xy2) const
{
    double dr12x = xy2.first - xy1.first;
    double dr12y = xy2.second - xy1.second;
    double dr12mag = std::hypot(dr12x, dr12y);
    double cos_rot = dr12y / dr12mag;
    double sin_rot = -dr12x / dr12mag;
    return std::pair<double, double>(cos_rot, sin_rot);
}

std::pair<double, double> FPGATrackSimGenScanKeyLyrHelper::rotateXY(const std::pair<double, double>& xy, const std::pair<double, double>& ang) const
{
    return std::pair<double, double>(xy.first * ang.first + xy.second * ang.second,
                                     -xy.first * ang.second + xy.second * ang.first);
}

FPGATrackSimGenScanKeyLyrHelper::rotatedConfig FPGATrackSimGenScanKeyLyrHelper::getRotatedConfig(const KeyLyrPars &keypars) const
{    
    rotatedConfig result;

    auto xy1 = getXY(m_R1, keypars.phi1);
    auto xy2 = getXY(m_R2, keypars.phi2);

    result.rotang = getRotationAngles(xy1, xy2);
    result.xy1p = rotateXY(xy1, result.rotang);
    result.xy2p = rotateXY(xy2, result.rotang);
    result.y = result.xy2p.second - result.xy1p.second;

    return result;
}

std::pair<double, double> FPGATrackSimGenScanKeyLyrHelper::getRotatedHit(const std::pair<double, double>& rotang, const FPGATrackSimHit *hit) const
{
    auto xyh = getXY(hit->getR(), hit->getGPhi());
    return rotateXY(xyh, rotang);
}

FPGATrackSimGenScanKeyLyrHelper::KeyLyrPars FPGATrackSimGenScanKeyLyrHelper::trackParsToKeyPars(const FPGATrackSimTrackPars &pars) const
{
    KeyLyrPars retv;
    retv.z1 = FPGATrackSimGenScanGeomHelpers::zFromPars(m_R1, pars);
    retv.z2 = FPGATrackSimGenScanGeomHelpers::zFromPars(m_R2, pars);
    retv.phi1 = FPGATrackSimGenScanGeomHelpers::phiFromPars(m_R1, pars);
    retv.phi2 = FPGATrackSimGenScanGeomHelpers::phiFromPars(m_R2, pars);

    double Rinv = pars[FPGATrackSimTrackPars::IHIP] * (2.0 * FPGATrackSimGenScanGeomHelpers::CurvatureConstant);
    if (Rinv != 0)
    {
        double R = 1 / Rinv;
        auto xy1 = getXY(m_R1, retv.phi1);
        auto xy2 = getXY(m_R2, retv.phi2);
        double ysqr = (xy2.first - xy1.first) * (xy2.first - xy1.first) + (xy2.second - xy1.second) * (xy2.second - xy1.second);
        double sign = (R > 0) - (R < 0);
        retv.xm = -1 * sign * (std::abs(R) - sqrt(R * R - ysqr / 4.0));
    }
    else
    {
        retv.xm = 0;
    }

    return retv;
}

FPGATrackSimTrackPars FPGATrackSimGenScanKeyLyrHelper::keyParsToTrackPars(const KeyLyrPars &keypars) const
{

    FPGATrackSimTrackPars pars;
    pars[FPGATrackSimTrackPars::IZ0] = (keypars.z1 * m_R2 - keypars.z2 * m_R1) / (m_R2 - m_R1);
    pars[FPGATrackSimTrackPars::IETA] = FPGATrackSimGenScanGeomHelpers::EtaFromTheta(atan2(m_R2 - m_R1, keypars.z2 - keypars.z1));

    // This is the exact math which is a bit messy
    // if you want the math contact lipeles@sas.upenn.edu

    double xm = keypars.xm;

    auto rotated_coords = getRotatedConfig(keypars);
    auto xy1p = rotated_coords.xy1p;
    auto rotang = rotated_coords.rotang;
    auto y = rotated_coords.y;

    // reverse rotation
    auto revang = rotang;
    revang.second = -rotang.second;

    if (xm != 0)
    {
        double Rinv = 2 * xm / (xm * xm + (y / 2) * (y / 2));
        double d = (y * y / 4.0 - xm * xm) / (2.0 * xm);
        double sign = (xm > 0) - (xm < 0);

        pars[FPGATrackSimTrackPars::IHIP] = -1 * Rinv / (2.0 * FPGATrackSimGenScanGeomHelpers::CurvatureConstant);

        std::pair<double, double> xycp(-d + xy1p.first, y / 2.0 + xy1p.second);

        pars[FPGATrackSimTrackPars::ID0] = -1 * sign * (std::abs(1 / Rinv) - std::hypot(xycp.first, xycp.second));

        auto xyc = rotateXY(xycp, revang);

        pars[FPGATrackSimTrackPars::IPHI] = atan2(sign * -xyc.first, sign * xyc.second);
    }
    else
    {
        pars[FPGATrackSimTrackPars::IHIP] = 0.0;
        pars[FPGATrackSimTrackPars::ID0] = -1 * xy1p.first;
        pars[FPGATrackSimTrackPars::IPHI] = atan2(rotang.first, -rotang.second);
    }

    return pars;
}

double FPGATrackSimGenScanKeyLyrHelper::zExpected(KeyLyrPars keypars, double r) const
{
    return (keypars.z2 - keypars.z1) / (m_R2 - m_R1) * (r - m_R1) + keypars.z1;
}

// takes hit position and calculated what xm should be for that hit
double FPGATrackSimGenScanKeyLyrHelper::xmForHit(KeyLyrPars keypars, const FPGATrackSimHit *hit) const
{
    auto rotated_coords = getRotatedConfig(keypars);
    auto xy1p = rotated_coords.xy1p;
    auto rotang = rotated_coords.rotang;
    auto y = rotated_coords.y;

    auto xyhp = getRotatedHit(rotang, hit);
    double xh = xyhp.first - xy1p.first;
    double yh = xyhp.second - xy1p.second;

    // use taylor expanded xm calculation
    double sign = ((yh > 0) && (yh < y)) ? 1 : -1;
    if ((std::abs(yh) < std::abs(xh)) || (std::abs(y - yh) < std::abs(xh)))
    {
        return ((xh > 0) ? 1 : -1) * 100000;
    }

    double xm_taylor = sign * y * y / (y * y - 4 * xh * xh - 4 * (yh - y / 2.0) * (yh - y / 2.0)) * xh;

    return xm_taylor;
}

// Find shift from nominal track to hit in the "x" direction 
double FPGATrackSimGenScanKeyLyrHelper::deltaX(KeyLyrPars keypars, const FPGATrackSimHit *hit) const
{
    auto rotated_coords = getRotatedConfig(keypars);

    auto rotang = rotated_coords.rotang;

    auto xyhp = getRotatedHit(rotang, hit);

    double xh = xyhp.first-rotated_coords.xy1p.first;

    return xh - xExpected(keypars,hit);
}

// Find shift from nominal track to hit in the "x" direction 
double FPGATrackSimGenScanKeyLyrHelper::xExpected(KeyLyrPars keypars, const FPGATrackSimHit *hit) const
{
    auto rotated_coords = getRotatedConfig(keypars);

    auto rotang = rotated_coords.rotang;
    auto y = rotated_coords.y;

    auto xyhp = getRotatedHit(rotang, hit);

    double yh = xyhp.second-rotated_coords.xy1p.second;

    return keypars.xm * (keypars.xm * keypars.xm + yh * (y - yh)) / (keypars.xm * keypars.xm + (y / 2) * (y / 2));
}

double FPGATrackSimGenScanPhiSlicedKeyLyrBinning::phiResidual(const ParSet &parset, FPGATrackSimHit const *hit, [[maybe_unused]] bool debug) const
{
    return m_keylyrtool.deltaX(parSetToKeyPars(parset), hit);
}


