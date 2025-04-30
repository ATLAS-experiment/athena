/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelDistortionData.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "CxxUtils/restrict.h"

#include "cmath"
namespace {

//constexpr binomial
//copied over from TMath but in constxpr
//fashion
constexpr double Binomial(int n, int k) {

  if (n < 0 || k < 0 || n < k) {
    return std::numeric_limits<double>::signaling_NaN();
  }
  if (k == 0 || n == k) {
    return 1;
  }

  int k1 = std::min(k, n - k);
  int k2 = n - k1;
  double fact = k2 + 1;
  for (double i = k1; i > 1.; --i) {
    fact *= (k2 + i) / i;
  }
  return fact;
}

constexpr int PolyDegree = 20;
constexpr int PolyNCoeff = PolyDegree + 1;

constexpr std::array<double, PolyNCoeff> BinomialCache() {
  constexpr double b0 = Binomial(20, 0);
  constexpr double b1 = Binomial(20, 1);
  constexpr double b2 = Binomial(20, 2);
  constexpr double b3 = Binomial(20, 3);
  constexpr double b4 = Binomial(20, 4);
  constexpr double b5 = Binomial(20, 5);
  constexpr double b6 = Binomial(20, 6);
  constexpr double b7 = Binomial(20, 7);
  constexpr double b8 = Binomial(20, 8);
  constexpr double b9 = Binomial(20, 9);
  constexpr double b10 = Binomial(20, 10);

  std::array<double, PolyNCoeff> res = {b0, b1, b2, b3,  b4, b5, b6,
                                        b7, b8, b9, b10, b9, b8, b7,
                                        b6, b5, b4, b3,  b2, b1, b0};

  return res;
}

// We just have a static const array in the anonymous
// as cache
constexpr std::array<double, PolyNCoeff> s_binomialCache = BinomialCache();

template <int N>
double bernstein_grundpolynom(const double t, const int i) {
  static_assert(s_binomialCache.size() > N,
                " Binomial cache must be larger than N");

  return s_binomialCache[i] * std::pow(t, i) * std::pow(1. - t, N - i);
}

double bernstein_bezier(const double u, const double v, const float *P) {
  double r = 0.;
  //So here we calculate the 21+21 polynomial values we need
  //for the inputs u , v for 0....n each (m==n)
  std::array<double,PolyNCoeff> bernstein_grundpolynomU{};
  std::array<double,PolyNCoeff> bernstein_grundpolynomV{};
  for (int i = 0; i < PolyNCoeff; ++i) {
    bernstein_grundpolynomU[i] = bernstein_grundpolynom<PolyDegree>(u, i);
    bernstein_grundpolynomV[i] = bernstein_grundpolynom<PolyDegree>(v, i);
  }

  for (int i = 0; i < PolyNCoeff; ++i) {

    //the one we passed u and  0 ....n
    const double bernstein_grundpolynom_i = bernstein_grundpolynomU[i];

    for (int j = 0; j < PolyNCoeff; ++j) {

      const int k = (i * (PolyDegree + 1)) + j;
      if (P[k] < -998.9){
        continue;
      }

      //the one we passed v and  0 ....n
      const double bernstein_grundpolynom_j = bernstein_grundpolynomV[j];
      r += bernstein_grundpolynom_i * bernstein_grundpolynom_j * P[k];
    }
  }
  return r;
}
}  // namespace

std::vector<float> PixelDistortionData::getDistortionMap(uint32_t hashID) const {
  int distosize;
  if (m_version < 2) distosize = 3;
  else distosize = 441;

  auto itr = m_distortionMap.find(hashID);
  if (itr!=m_distortionMap.end()) {
    return itr->second;
  }
  std::vector<float> map(distosize, 0.0);
  return map;
}

unsigned long long PixelDistortionData::getId(uint32_t hashID) const {
  auto search = m_ids.find(hashID);
  if (search != m_ids.end()) return search->second;

  return 0;
}

Amg::Vector2D PixelDistortionData::correction(uint32_t hashID, const Amg::Vector2D & locpos, const Amg::Vector3D & direction) const {
  double localphi = locpos[0];
  double localeta = locpos[1];
  const Amg::Vector2D nullCorrection(0.0,0.0);
  const unsigned long long ull_id = getId(hashID);

  // No corrections available for this module
  if (ull_id == 0 && m_version > 1) { return nullCorrection; }

  // This is needed to avoid rounding errors if the z component of the
  // direction vector is too small.
  if (std::fabs(direction.z())<1.*CLHEP::micrometer) { return nullCorrection; }

  // If a particle has a too shallow trajectory with respect to the
  // module direction, it is likely to be a secondary particle and no
  // shift should be applied.
  double invtanphi = direction.x()/direction.z();
  double invtaneta = direction.y()/direction.z();
  if (std::sqrt(invtanphi*invtanphi+invtaneta*invtaneta)>100.0) { return nullCorrection; }

  double localZ = 0;
  std::vector<float> map = getDistortionMap(hashID);

  // If old database versions, the corrections are from the pixel survey, otherwise corrections are from
  // insitu shape measurements.
  if (m_version < 2) {
     localZ = getSurveyZ(localeta, localphi, map.data());
  } else if (isOldParam(ull_id)) {
     localZ = getSurveyZ(localeta, localphi, map.data());
  } else {
     float modEtaSize, modPhiSize;

     // set module sizes in millimetre
     if (ull_id >= 0x200000000000000 && ull_id <= 0x20d980000000000) {
       if (isIBL3D(ull_id)) {
         modEtaSize = 19.75 * CLHEP::millimeter;
         modPhiSize = 16.75 * CLHEP::millimeter;
       } else {
         modEtaSize = 40.4 * CLHEP::millimeter;
         modPhiSize = 16.8 * CLHEP::millimeter;
       }
     } else {
       modEtaSize = 60.2 * CLHEP::millimeter;
       modPhiSize = 16.44 * CLHEP::millimeter;
     }

     localZ = getInSituZ(localeta, modEtaSize, localphi, modPhiSize, map.data());
  }

  double localetaCor = -localZ * invtaneta;
  double localphiCor = -localZ * invtanphi;

  // In earlies code version there was a bug in the sign of the correction.
  // In MC this was not seen as reco just corrects back what was done in digitization.
  // In order to maintain backward compatibilty in older MC we need to reproduce this wrong behaviour.
  if (getVersion()==0) { localphiCor = -localphiCor; }

  return Amg::Vector2D(localphiCor, localetaCor);
}

Amg::Vector2D PixelDistortionData::correctReconstruction(uint32_t hashID, const Amg::Vector2D & locpos, const Amg::Vector3D & direction) const {
  Amg::Vector2D newlocpos(locpos);
  newlocpos += correction(hashID, locpos, direction);
  return newlocpos;
}

Amg::Vector2D PixelDistortionData::correctSimulation(uint32_t hashID, const Amg::Vector2D & locpos, const Amg::Vector3D & direction) const {
  Amg::Vector2D newlocpos(locpos);
  newlocpos -= correction(hashID, locpos, direction);
  return newlocpos;
}

double PixelDistortionData::getInSituZ(const double localeta, const double eta_size, const double localphi, const double phi_size, const float *disto)
{
  double etaHalfRangeBB = eta_size * 10. / 21.;
  double phiHalfRangeBB = phi_size * 10. / 21.;
  double etaRangeBB = eta_size * 20. / 21.;
  double phiRangeBB = phi_size * 20. / 21.;
  double eta, phi;

  // map positions on interval [- edge, 1 + edge]
  // edge is the part outside of the Bezier-Bernstein range
  if (std::abs(localeta) - etaHalfRangeBB > 0) {
    if (localeta < 0)
      eta = (localeta + etaHalfRangeBB) / etaRangeBB;
    else
      eta = 1. + (localeta - etaHalfRangeBB) / etaRangeBB;
  } else {
    eta = localeta / etaRangeBB + 0.5;
  }
  if (std::abs(localphi) - phiHalfRangeBB > 0) {
    if (localphi < 0)
      phi = (localphi + phiHalfRangeBB) / phiRangeBB;
    else
      phi = 1. + (localphi - phiHalfRangeBB) / phiRangeBB;
  } else {
    phi = localphi / phiRangeBB + 0.5;
  }
  return bernstein_bezier(eta, phi, disto);
}

double PixelDistortionData::getSurveyZ(const double localeta, const double localphi, const float *disto)
{
  const double xFE = 22.0 * CLHEP::millimeter; // Distance between the 2 Front-End line, where bows have been measured
  const double yFE = 60.8 * CLHEP::millimeter; // Length of the active surface of each module

  double data0 = disto[0] / CLHEP::meter;           // curvature is in m-1
  double data1 = disto[1] / CLHEP::meter;           // curvature is in m-1
  double data2 = std::tan(0.5 * disto[2] * CLHEP::degree);   // twist angle in degree

  double twist1 = -data2;
  double twist2 = data2;
  double b1 = std::sqrt((1. + twist1*twist1) * (1. + twist1*twist1) * (1. + twist1*twist1));
  double b2 = std::sqrt((1. + twist2*twist2) * (1. + twist2*twist2) * (1. + twist2*twist2));
  double z1 = localeta * twist1 - 0.5 * b1 * localeta*localeta * data1;
  double z2 = localeta * twist2 - 0.5 * b2 * localeta*localeta * data0;
  double zoff1 = (b1 * yFE*yFE * data1) / 24.;
  double zoff2 = (b2 * yFE*yFE * data0) / 24.;
  z1 = z1 + zoff1;
  z2 = z2 + zoff2;

  return z1 + ((z2 - z1) / xFE) * (localphi + xFE / 2.);
}

bool PixelDistortionData::isOldParam(const unsigned long long ull_id)
{
  // Only pixel modules can have the old parametrisation
  if (ull_id < 0x240000000000000) return false;

  // For now: no new parametrisation for Pixel
  return true;
}

bool PixelDistortionData::isIBL3D(const unsigned long long ull_id)
{
  // Stave 1
  if (ull_id >= 0x200000000000000 && ull_id <= 0x200180000000000) return true;
  if (ull_id >= 0x200800000000000 && ull_id <= 0x200980000000000) return true;

  // Stave 2
  if (ull_id >= 0x201000000000000 && ull_id <= 0x201180000000000) return true;
  if (ull_id >= 0x201800000000000 && ull_id <= 0x201980000000000) return true;

  // Stave 3
  if (ull_id >= 0x202000000000000 && ull_id <= 0x202180000000000) return true;
  if (ull_id >= 0x202800000000000 && ull_id <= 0x202980000000000) return true;

  // Stave 4
  if (ull_id >= 0x203000000000000 && ull_id <= 0x203180000000000) return true;
  if (ull_id >= 0x203800000000000 && ull_id <= 0x203980000000000) return true;

  // Stave 5
  if (ull_id >= 0x204000000000000 && ull_id <= 0x204180000000000) return true;
  if (ull_id >= 0x204800000000000 && ull_id <= 0x204980000000000) return true;

  // Stave 6
  if (ull_id >= 0x205000000000000 && ull_id <= 0x205180000000000) return true;
  if (ull_id >= 0x205800000000000 && ull_id <= 0x205980000000000) return true;

  // Stave 7
  if (ull_id >= 0x206000000000000 && ull_id <= 0x206180000000000) return true;
  if (ull_id >= 0x206800000000000 && ull_id <= 0x206980000000000) return true;

  // Stave 8
  if (ull_id >= 0x207000000000000 && ull_id <= 0x207180000000000) return true;
  if (ull_id >= 0x207800000000000 && ull_id <= 0x207980000000000) return true;

  // Stave 9
  if (ull_id >= 0x208000000000000 && ull_id <= 0x208180000000000) return true;
  if (ull_id >= 0x208800000000000 && ull_id <= 0x208980000000000) return true;

  // Stave 10
  if (ull_id >= 0x209000000000000 && ull_id <= 0x209180000000000) return true;
  if (ull_id >= 0x209800000000000 && ull_id <= 0x209980000000000) return true;

  // Stave 11
  if (ull_id >= 0x20a000000000000 && ull_id <= 0x20a180000000000) return true;
  if (ull_id >= 0x20a800000000000 && ull_id <= 0x20a980000000000) return true;

  // Stave 12
  if (ull_id >= 0x20b000000000000 && ull_id <= 0x20b180000000000) return true;
  if (ull_id >= 0x20b800000000000 && ull_id <= 0x20b980000000000) return true;

  // Stave 13
  if (ull_id >= 0x20c000000000000 && ull_id <= 0x20c180000000000) return true;
  if (ull_id >= 0x20c800000000000 && ull_id <= 0x20c980000000000) return true;

  // Stave 14
  if (ull_id >= 0x20d000000000000 && ull_id <= 0x20d180000000000) return true;
  if (ull_id >= 0x20d800000000000 && ull_id <= 0x20d980000000000) return true;

  return false;
}

void PixelDistortionData::clear() {
  m_distortionMap.clear();
}

