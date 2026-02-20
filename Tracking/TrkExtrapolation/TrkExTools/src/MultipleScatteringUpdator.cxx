/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MultipleScatteringUpdator.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk include
#include "TrkExTools/MultipleScatteringUpdator.h"
#include "TrkGeometry/MaterialProperties.h"
// CLHEP

#include "TrkExUtils/MaterialInteraction.h"
#include "AthenaKernel/RNGWrapper.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "CLHEP/Random/RandFlat.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/close_to_zero.h"

using CxxUtils::close_to_zero;

namespace{
// static doubles
constexpr double s_main_RutherfordScott = 13.6 * Gaudi::Units::MeV;
constexpr double s_log_RutherfordScott = 0.038;

constexpr double s_main_RossiGreisen = 17.5 * Gaudi::Units::MeV;
constexpr double s_log_RossiGreisen = 0.125;

// ============================= Gaussian mixture model =============
constexpr double s_gausMixSigma1_a0 = 8.471e-1;
constexpr double s_gausMixSigma1_a1 = 3.347e-2;
constexpr double s_gausMixSigma1_a2 = -1.843e-3;

constexpr double s_gausMixEpsilon_a0 = 4.841e-2;
constexpr double s_gausMixEpsilon_a1 = 6.348e-3;
constexpr double s_gausMixEpsilon_a2 = 6.096e-4;

constexpr double s_gausMixEpsilon_b0 = -1.908e-2;
constexpr double s_gausMixEpsilon_b1 = 1.106e-1;
constexpr double s_gausMixEpsilon_b2 = -5.729e-3;
}


// constructor
Trk::MultipleScatteringUpdator::MultipleScatteringUpdator(const std::string &t, const std::string &n,
                                                          const IInterface *p) :
  AthAlgTool(t, n, p) {
  declareInterface<IMultipleScatteringUpdator>(this);
}

// destructor
Trk::MultipleScatteringUpdator::~MultipleScatteringUpdator() = default;

// Athena standard methods
// initialize
StatusCode
Trk::MultipleScatteringUpdator::initialize() {
  // if Gaussian mixture is required
  if (m_gaussianMixture) {
    // get the random generator service
    if (m_rndGenSvc.retrieve().isFailure()) {
      ATH_MSG_WARNING("Could not retrieve " << m_rndGenSvc << " -> switching Gaussian mixture model off.");
      m_gaussianMixture = false;
    } else {
      ATH_MSG_VERBOSE("Successfully retrieved " << m_rndGenSvc);
    }

    // Get own engine with own seeds:
    m_rngWrapper = m_gaussianMixture ? m_rndGenSvc->getEngine(this, m_randomEngineName) : nullptr;
    if (!m_rngWrapper) {
      ATH_MSG_WARNING(
        "Could not get random engine '" << m_randomEngineName << "' -> switching Gaussian mixture model off.");
      m_gaussianMixture = false;
    }
  }
  if (m_gaussianMixture) {
    ATH_MSG_VERBOSE("Gaussian mixture model = ON ... optimised = " << m_optGaussianMixtureG4);
  } else {
    ATH_MSG_VERBOSE("Gaussian mixture model = OFF");
  }

  return StatusCode::SUCCESS;
}

double
Trk::MultipleScatteringUpdator::sigmaSquare(const MaterialProperties &mat,
                                            double p,
                                            double pathcorrection,
                                            ParticleHypothesis particle,
                                            double) const {
  if (mat.thicknessInX0() <= 0. || particle == Trk::geantino) {
    return 0.;
  }

  // make sure the path correction is positive to avoid a floating point exception
  pathcorrection *= pathcorrection < 0. ? (-1.) : (1);

  // scale the path length to the radiation length
  double t = pathcorrection * mat.thicknessInX0();

  // kinematics (relativistic)
  double m = Trk::ParticleMasses::mass[particle];
  double E = sqrt(p * p + m * m);
  double beta = p / E;

  double sigma2(0.);

  double sigma = Trk::MaterialInteraction::sigmaMS(t, p, beta);
  sigma2 = sigma * sigma;

  if (m_useTrkUtils && particle != Trk::electron) {
    return sigma2;
  }

// Code below will not be used if the parameterization of TrkUtils is used


  if (particle != Trk::electron) {
    if (auto denom = beta * p; not close_to_zero(denom)){
      // the highland formula
      sigma2 = s_main_RutherfordScott / denom;
    }

    if (m_log_include) {
      sigma2 *= (1. + s_log_RutherfordScott * std::log(t));
    }

    sigma2 *= (sigma2 * t);
  }else {
    // Electron multiple scattering effects - see Highland NIM 129 (1975) p497-499
    // (Highland extension to the Rossi-Greisen formulation)
    // NOTE: The formula can be extended by replacing the momentum^2 term with pi * pf
    sigma2 = s_main_RossiGreisen / (beta * p);
    sigma2 *= (sigma2 * t);

    if (m_log_include) {
      double factor = 1. + s_log_RossiGreisen * std::log10(10. * t);
      factor *= factor;
      sigma2 *= factor;
    }
  }

  // use the Gaussian mixture model
  if (m_gaussianMixture) {
    // Reseed the RNG if needed.
    const EventContext& ctx = Gaudi::Hive::currentContext();
    if (m_rngWrapper->evtSeeded(ctx) != ctx.evt()) {
      // Ok, the wrappers are unique to this algorithm and a given slot,
      // so cannot be accessed concurrently.
      ATHRNG::RNGWrapper* wrapper_nc ATLAS_THREAD_SAFE = m_rngWrapper;
      wrapper_nc->setSeed (this->name(), ctx);
    }
    CLHEP::HepRandomEngine* engine = m_rngWrapper->getEngine (ctx);
    // d_0'
    double dprime{};
    if (auto denom = beta * beta; not close_to_zero(denom)){
      dprime = t / denom;
    } else {
      ATH_MSG_WARNING("Trk::MultipleScatteringUpdator::sigmaSquare: beta close to zero");
      return 0.;
    }
    double log_dprime = std::log(dprime);
    // d_0''
    double log_dprimeprime = std::log(std::pow(mat.averageZ(), 2.0 / 3.0) * dprime);
    // get epsilon
    double epsilon = log_dprimeprime < 0.5 ?
                     s_gausMixEpsilon_a0 + s_gausMixEpsilon_a1 * log_dprimeprime + s_gausMixEpsilon_a2 *
                     log_dprimeprime * log_dprimeprime :
                     s_gausMixEpsilon_b0 + s_gausMixEpsilon_b1 * log_dprimeprime + s_gausMixEpsilon_b2 *
                     log_dprimeprime * log_dprimeprime;
    // the standard sigma
    double sigma1square = s_gausMixSigma1_a0 + s_gausMixSigma1_a1 * log_dprime + s_gausMixSigma1_a2 * log_dprime *
                          log_dprime;
    // G4 optimised / native double Gaussian model
    if (!m_optGaussianMixtureG4) {
      if (auto denom = p * p;not close_to_zero(denom)){
        sigma2 = 225. * dprime / (p * p);
      } else {
        ATH_MSG_WARNING("Trk::MultipleScatteringUpdator::sigmaSquare: p close to zero");
      }
    }
    // throw the random number core/tail
    if (CLHEP::RandFlat::shoot(engine) < epsilon) {
      sigma2 *= (1. - (1. - epsilon) * sigma1square) / epsilon;
    }
  }

  return sigma2;
}
