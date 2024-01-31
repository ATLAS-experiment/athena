/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRK_CURVILINEARCOVARIANCEHELPER_H
#define ACTSTRK_CURVILINEARCOVARIANCEHELPER_H 1

#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Propagator/detail/JacobianEngine.hpp"

namespace ActsTrk {
   /** Helper function to compute dt/ds
    * Helper function to compute the derivative of the time as function of the path length
    */
   inline double computeDtDs(const Acts::ParticleHypothesis &hypothesis, double qop) {
      return std::hypot(1.,hypothesis.mass() / hypothesis.extractMomentum(qop));
   }

   /** Compute the path length derivatives for the free/bound to curvilinear paramter transform.
    * @param direction the direction of trajectory at the location in question
    * @param qop q/p of the particle at the location in question in Acts units
    * @param bfield the magnetic field at the location in question in Acts units
    * @param particle_hypothesis the particle hypothesis e.g. Acts::ParticleHypothesis::pion()
    * @return path length derivatives ( dr(...)/ds, dt/ds, dr(...)/ds2, d qop/ds [== 0] )
    */
   inline Acts::FreeToPathMatrix computeFreeToPathDerivatives( const Acts::Vector3 &direction,
                                                               double qop,
                                                               const Acts::Vector3 &bfield,
                                                               const Acts::ParticleHypothesis &particle_hypothesis)
   {
      Acts::FreeToPathMatrix path_length_deriv;
      static_assert(path_length_deriv.cols() == 8);  // ensure that all elements are initialized
      path_length_deriv.segment<3>(Acts::eFreePos0) = direction;
      path_length_deriv(0,Acts::eFreeTime) =  computeDtDs(particle_hypothesis,qop);
      path_length_deriv.segment<3>(Acts::eFreeDir0) = (qop  * direction.cross(bfield)).transpose();
      path_length_deriv(0,Acts::eFreeQOverP) = 0.;
      return path_length_deriv;
   }


   /** Convert the covariance of the given Acts track parameters into curvilinear parameterisation.
    * @param tgContext the current geometry context
    * @param param the Acts track parameters
    * @param magnFieldVect the vector of the magnetic field at the position of the track parameters in Acts units
    * @param particle_hypothesis the particle hypothesis e.g. Acts::ParticleHypothesis::pion()
    * Usage:
    * <pre>
    *   <code>
    *      using namespace Acts::UnitLiterals;
    *      MagField::AtlasFieldCache fieldCache;
    *      ...
    *      Amg::Vector3D magnFieldVect;
    *      fieldCache.getField(param.position(tgContext).data(), magnFieldVect.data());
    *      magnFieldVect *= 1000_T;
    *      auto cov = convertActsBoundCovToCurvilinearParam(tgContext, param, magnFieldVect);
    *      if (cov.has_value()) { ... }
    *   </code>
    * </pre>
    */
   inline std::optional<Acts::BoundMatrix> convertActsBoundCovToCurvilinearParam(const Acts::GeometryContext &tgContext,
                                                                                 const Acts::BoundTrackParameters &param,
                                                                                 const Acts::Vector3 &magnFieldVect,
                                                                                 const Acts::ParticleHypothesis &particle_hypothesis)
   {
      if (param.covariance().has_value()) {

         Acts::Vector3 direction(param.direction());
         Acts::BoundMatrix
            b2c = Acts::detail::boundToCurvilinearTransportJacobian(direction, // magnFieldVect.normalized(),
                                                                    param.referenceSurface().boundToFreeJacobian(tgContext,
                                                                                                                 param.parameters()),
                                                                    Acts::FreeMatrix::Identity(),
                                                                    computeFreeToPathDerivatives(direction,
                                                                                                 param.parameters()[Acts::eBoundQOverP],
                                                                                                 magnFieldVect,
                                                                                                 particle_hypothesis));

         return b2c * param.covariance().value() * b2c.transpose();
      }
      else {
         return {};
      }
   }
}
#endif
