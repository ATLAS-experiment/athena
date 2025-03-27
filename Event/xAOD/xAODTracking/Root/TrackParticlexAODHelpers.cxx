/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "AthContainers/AuxElement.h"
#include <stdexcept>

namespace xAOD {
  
  namespace TrackingHelpers {
  void checkTPAndDefiningParamCov(const xAOD::TrackParticle *tp) {
    if (!tp) {
      throw std::runtime_error("Invalid TrackParticle pointer.");
    }
    SG::AuxElement::ConstAccessor< std::vector<float> > accDiag( "definingParametersCovMatrixDiag" );
    SG::AuxElement::ConstAccessor< std::vector<float> > accOffDiag( "definingParametersCovMatrixOffDiag" );
    if( !(accDiag.isAvailable( *tp ) && accOffDiag.isAvailable( *tp )) ) {
      throw std::runtime_error("TrackParticle without covariance matrix for the defining parameters.");
    }
  }

  void checkTPAndDefiningParamCovDiag(const xAOD::TrackParticle *tp) {
    if (!tp) {
      throw std::runtime_error("Invalid TrackParticle pointer.");
    }
    SG::AuxElement::ConstAccessor< std::vector<float> > accDiag( "definingParametersCovMatrixDiag" );
    if( !(accDiag.isAvailable( *tp )) ) {
      throw std::runtime_error("TrackParticle without diagonal covariance matrix elements for the defining parameters.");
    }
  }
  }

  bool TrackingHelpers::hasValidCov(const xAOD::TrackParticle *tp) {
    if (!tp) return false;
    if( tp->definingParametersCovMatrixFilled() != xAOD::FullCovMatrixAvailable ) return false;
    return true;
  }

  namespace TrackingHelpers {
    inline double d0significance(const xAOD::TrackParticle *tp, double d0_uncert_beam_spot_2) {
      checkTPAndDefiningParamCovDiag(tp);
      double d0 = tp->d0();
      // elements in definingParametersCovMatrixDiagVec should be : sigma_d0^2, sigma_z0^2
      double sigma_d0 = tp->definingParametersCovMatrixDiagVec().at(0);
      if (sigma_d0<=0.) {
        throw std::runtime_error("TrackParticle with zero or negative d0 uncertainty.");
      }
      return d0/sqrt(sigma_d0+d0_uncert_beam_spot_2);
    }
  }

  double TrackingHelpers::d0significance(const xAOD::TrackParticle *tp) {
    return d0significance(tp,0.);
  }

  double TrackingHelpers::d0significance(const xAOD::TrackParticle *tp, double beam_sigma_x, double beam_sigma_y, double beam_sigma_xy) {
    if (!checkBeamSpotSigma(beam_sigma_x, beam_sigma_y, beam_sigma_xy)) {
      throw std::runtime_error("Beamspot covariance matrix is invalid.");
    }
    return d0significance(tp,d0UncertaintyBeamSpot2(tp->phi(),beam_sigma_x, beam_sigma_y, beam_sigma_xy));
  }

  double TrackingHelpers::z0significance(const xAOD::TrackParticle *tp, const xAOD::Vertex *vx) {
    checkTPAndDefiningParamCovDiag(tp);
    double z0 = tp->z0() + tp->vz();
    if (vx) {
        if (!checkPVReference(tp,vx)) {
           throw std::runtime_error("Given primary vertex does not fulfil the requirements i.e. does not exist, or is too far away from the beam axis.");
        }
        z0 -= vx->z();
    }
    // elements in definingParametersCovMatrixVec should be : sigma_d0^2, sigma_z0^2
    double sigma_z0 = tp->definingParametersCovMatrixDiagVec().at(1);
    if (sigma_z0<=0.) {
      throw std::runtime_error("TrackParticle with zero or negative z0 uncertainty.");
    }
    return z0/sqrt(sigma_z0);
  }

  double TrackingHelpers::z0sinthetasignificance(const xAOD::TrackParticle *tp, const xAOD::Vertex *vx) {
    checkTPAndDefiningParamCovDiag(tp);
    double z0 = tp->z0() + tp->vz();
    double sintheta = sin(tp->theta());
    // elements in definingParametersCovMatrixVec should be : sigma_d0^2, sigma_z0^2
    double sigma_z0 = tp->definingParametersCovMatrixDiagVec().at(1);
    if (vx) z0 -= vx->z();
    double costheta = cos(tp->theta());
    double sigma_theta = tp->definingParametersCovMatrixDiagVec().at(3);
    double sigma_z0sintheta = sigma_z0 * sintheta * sintheta + z0 * z0 * costheta * costheta * sigma_theta;
    if (sigma_z0sintheta <= 0.) {
      throw std::runtime_error("TrackParticle with zero or negative z0sintheta uncertainty.");
    }
    sigma_z0sintheta = sqrt(sigma_z0sintheta);
    return z0 * sintheta / sigma_z0sintheta;
        
  }

  double TrackingHelpers::pTErr2(const xAOD::TrackParticle *tp) {
    checkTPAndDefiningParamCov(tp);
    if (!hasValidCovQoverP(tp)) {
      throw std::runtime_error("TrackParticle without covariance matrix for defining parameters or the covariance matrix is wrong dimensionality.");
    }
    if (std::abs(tp->qOverP())<0) {
      throw std::runtime_error("q/p is zero");
    }
    return pTErr2Unsafe(tp);
  }


}

