/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackParamsEstimationTool.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"

namespace ActsTrk {
  TrackParamsEstimationTool::TrackParamsEstimationTool(const std::string& type,
						       const std::string& name,
						       const IInterface* parent)
    : base_class(type, name, parent)  
  {}

  StatusCode TrackParamsEstimationTool::initialize() 
  {
    ATH_MSG_INFO( "Initializing " << name() << "..." );

    ATH_MSG_DEBUG( "Properties Summary:" );
    ATH_MSG_DEBUG( "   " << m_bFieldMin );
    ATH_MSG_DEBUG( "   " << m_sigmaLoc0 );
    ATH_MSG_DEBUG( "   " << m_sigmaLoc1 );
    ATH_MSG_DEBUG( "   " << m_sigmaPhi );
    ATH_MSG_DEBUG( "   " << m_sigmaTheta );
    ATH_MSG_DEBUG( "   " << m_sigmaQOverP );
    ATH_MSG_DEBUG( "   " << m_sigmaT0 );
    ATH_MSG_DEBUG( "   " << m_initialVarInflation );

    return StatusCode::SUCCESS;
  }

  std::optional<Acts::BoundTrackParameters>
  TrackParamsEstimationTool::estimateTrackParameters(const EventContext& ctx,
						     const ActsTrk::Seed& seed,
						     const Acts::GeometryContext& geoContext,
						     const Acts::MagneticFieldContext& magFieldContext,
						     std::function<const Acts::Surface&(const ActsTrk::Seed&)> retrieveSurface) const 
  {
    const auto& sp_collection = seed.sp();
    if ( sp_collection.size() < 3 ) return std::nullopt;
    const auto& bottom_sp = sp_collection.front();

    // Magnetic Field
    ATLASMagneticFieldWrapper magneticField;
    Acts::MagneticFieldProvider::Cache magFieldCache = magneticField.makeCache( magFieldContext );
    Acts::Vector3 bField = *magneticField.getField( Acts::Vector3(bottom_sp->x(), bottom_sp->y(), bottom_sp->z()),
                                                    magFieldCache );

    // Get the surface
    const Acts::Surface& surface = retrieveSurface(seed);

    return estimateTrackParameters(ctx,
				   seed,
				   geoContext,
				   surface,
				   bField,
				   m_bFieldMin);
  }

  std::optional<Acts::BoundTrackParameters>
  TrackParamsEstimationTool::estimateTrackParameters(const EventContext& /*ctx*/,
						     const ActsTrk::Seed& seed,
						     const Acts::GeometryContext& geoContext,
						     const Acts::Surface& surface,
						     const Acts::Vector3& bField,
						     double bFieldMin) const 
  {
    // Get SPs
    const auto& sp_collection = seed.sp();
    if ( sp_collection.size() < 3 ) return std::nullopt;

    // Compute Bound parameters at surface
    std::optional<Acts::BoundVector> params_opt =
      Acts::estimateTrackParamsFromSeed(geoContext,
                                        sp_collection.begin(),
                                        sp_collection.end(),
                                        surface,
                                        bField,
                                        bFieldMin);

    if ( not params_opt.has_value() )
      return std::nullopt;

    const auto& params = params_opt.value();

    Acts::BoundVector initialSigmas = {m_sigmaLoc0, m_sigmaLoc1, m_sigmaPhi,
        m_sigmaTheta, m_sigmaQOverP, m_sigmaT0};
    Acts::BoundMatrix covariance = Acts::BoundMatrix::Zero();

    for (std::size_t i = Acts::eBoundLoc0; i < Acts::eBoundSize; ++i) {
      double variance = initialSigmas[i] * initialSigmas[i];

      if (i == Acts::eBoundQOverP) {
        // note that we rely on the fact that sigma theta is already computed
        double varianceTheta = covariance(Acts::eBoundTheta, Acts::eBoundTheta);

        // transverse momentum contribution
        variance +=
            std::pow(m_initialSigmaPtRel * params[Acts::eBoundQOverP], 2);

        // theta contribution
        variance +=
            varianceTheta * std::pow(params[Acts::eBoundQOverP] /
                                        std::tan(params[Acts::eBoundTheta]),
                                    2);
      }

      // Inflate the initial covariance
      variance *= m_initialVarInflation[i];

      covariance(i, i) = variance;
    }

    // Create BoundTrackParameters
    return Acts::BoundTrackParameters(surface.getSharedPtr(),
                                      params,
                                      covariance,
                                      Acts::ParticleHypothesis::pion());
  }
  
}
// namespace ActsTrk
