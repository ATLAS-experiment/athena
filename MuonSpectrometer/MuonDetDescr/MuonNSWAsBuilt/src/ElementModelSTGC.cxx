/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonNSWAsBuilt/ElementModelSTGC.h"
#include <stdexcept>
#include <utility>

using namespace NswAsBuilt;

ElementModelSTGC::ElementModelSTGC(double lenX, double lenY, Amg::Vector3D defo0)
  : m_lenY(lenY),m_lenX(lenX),
  m_defo0(std::move(defo0))
{
}

/**
 * Transform a set of vectors expressed in local frame, stored in a matrix
 */
void ElementModelSTGC::transform(const ParameterVector& parvec, VectorSetRef local) const {

  if (!parvec.transformCacheValid) {
    throw std::runtime_error("Should call Element::cacheTransforms() first");
  }

  // Apply the deformation component
  
  // old-style (reference) implementation: not optimized, provided for comparison purposes
  // for (int i=0; i< local.cols(); ++i) {
  //   applyDeformation(parvec, local.col(i));
  // }


  // Eigen-style implementation: optimized
  applyDeformation2(parvec, local);

  // Apply the rigid component
  local = (parvec.transformCache * local).eval(); // Needs eval to avoid aliasing (?)
}

/**
 * Cache the rigid component of this deformation model
 * Add rotation and offset obtained from the sTGC fit 
 */
void ElementModelSTGC::cacheTransform(ParameterVector& parvec) const {
  const auto& pars = parvec.parameters;
  parvec.transformCache
    = Eigen::Translation3d(pars[X], pars[Y], pars[Z])
    * Eigen::AngleAxisd(pars[THZ], Eigen::Vector3d::UnitZ())
    * Eigen::AngleAxisd(pars[THY], Eigen::Vector3d::UnitY())
    * Eigen::AngleAxisd(pars[THX], Eigen::Vector3d::UnitX());
  parvec.transformCacheValid = true;
}

/*
 * Helper methods to convert parameter index to string representation
 */
ElementModelSTGC::ipar_t ElementModelSTGC::getParameterIndex(const std::string& parname) const {
  if (parname == "x") return parameter_t::X;
  if (parname == "y") return parameter_t::Y;
  if (parname == "z") return parameter_t::Z;
  if (parname == "thx") return parameter_t::THX;
  if (parname == "thy") return parameter_t::THY;
  if (parname == "thz") return parameter_t::THZ;
  if (parname == "rot") return parameter_t::ROT;
  if (parname == "off") return parameter_t::OFF;
  if (parname == "scl") return parameter_t::SCL;
  if (parname == "npar") return parameter_t::NPAR;
  throw std::runtime_error("Invalid parameter name "+parname);
}

std::string ElementModelSTGC::getParameterName(ipar_t ipar) const {
  switch (ipar) {
    case X: return "x";
    case Y: return "y";
    case Z: return "z";
    case THX: return "thx";
    case THY: return "thy";
    case THZ: return "thz";
    case ROT: return "rot";
    case OFF: return "off";
    case SCL: return "scl";
    case NPAR: return "npar";
    default: throw std::runtime_error("Invalid parameter");
  }
}

Amg::Vector3D ElementModelSTGC::stgcOffset(double off) {
// Offset extracted from combined fit of X-ray data and by CMM/Faro measurement at construction sites
  return off * Amg::Vector3D::UnitY();
}

Amg::Vector3D ElementModelSTGC::stgcRotation(double rot, const Amg::Vector3D& d0) {
// Rotation extracted from combined fit of X-ray data and by CMM/Faro measurement at construction sites
  return -rot * d0.cross(Amg::Vector3D::UnitZ());
}

Amg::Vector3D ElementModelSTGC::stgcScale(double scl, const Amg::Vector3D& d0) const {
// Scale measured by CMM/Faro at construction sites
  return d0.array() * Eigen::Array3d{0., scl/m_lenY, 0.};
}


Amg::Vector3D ElementModelSTGC::stgcNonPar(double npar, const Amg::Vector3D& d0) const {
// Non-parallelism measured by CMM/Faro at construction sites
  double delta = npar*d0[0]*d0[1]/(m_lenX*m_lenY);
  return Amg::Vector3D(0., delta, 0.);
}

void ElementModelSTGC::applyDeformation(const ParameterVector& parvec, Eigen::Ref<Amg::Vector3D> local) const {
  // Applies the deformation to the set of local vectors given as argument
  // This old-style implementation is the reference implementation
  Amg::Vector3D d0 = local - m_defo0;
  local = local
    + stgcOffset(parvec[OFF])
    + stgcRotation(parvec[ROT], d0)
    + stgcScale(parvec[SCL], d0)
    + stgcNonPar(parvec[NPAR], d0)
    ;
}

void ElementModelSTGC::applyDeformation2(const ParameterVector& parvec, VectorSetRef local) const {
  // Applies the deformation to the set of local vectors given as argument
  // This implementation uses Eigen-style algebra (does the same as the method applyDeformation above, but benefits from Eigen's optimizations)

  // Temporaries allocated on the stack
  // d0 = local - defo0
  VectorSet d0 = local.colwise() - m_defo0;

  double off = parvec[OFF];
  double rot = parvec[ROT];
  double scl = parvec[SCL]/m_lenY;
  double npar = parvec[NPAR]/(m_lenX*m_lenY);

  // OFF:
  local.array().colwise() += Eigen::Array3d{0. ,off, 0.};
 
  // ROT
  local.topRows<2>().array() += d0.topRows<2>().array().colwise().reverse().colwise() * Eigen::Array2d{0., rot};

  // SCL:
  local.array() += d0.array().colwise() * Eigen::Array3d{0., scl, 0.};

  // NPAR:
  local.topRows<2>().array() +=  (d0.topRows<2>().array().colwise().reverse() * d0.topRows<2>().array()).colwise() * Eigen::Array2d{0., npar};
}

