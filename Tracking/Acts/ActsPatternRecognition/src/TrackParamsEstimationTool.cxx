/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackParamsEstimationTool.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"
#include "Acts/EventData/TransformationHelpers.hpp"

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
    ATH_MSG_DEBUG( "   " << m_sigmaLoc0 );
    ATH_MSG_DEBUG( "   " << m_sigmaLoc1 );
    ATH_MSG_DEBUG( "   " << m_sigmaPhi );
    ATH_MSG_DEBUG( "   " << m_sigmaTheta );
    ATH_MSG_DEBUG( "   " << m_sigmaQOverP );
    ATH_MSG_DEBUG( "   " << m_sigmaT0 );
    ATH_MSG_DEBUG( "   " << m_initialVarInflation );
    ATH_MSG_DEBUG( "   " << m_useTopSp );

    // Make the logger And Propagate to ACTS routines
    m_logger = makeActsAthenaLogger(this, "Acts");
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
    const auto& bottom_sp = m_useTopSp ? sp_collection.back() : sp_collection.front();

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
				   bField);
  }

  std::optional<Acts::BoundTrackParameters>
  TrackParamsEstimationTool::estimateTrackParameters(const EventContext& /*ctx*/,
						     const ActsTrk::Seed& seed,
						     const Acts::GeometryContext& geoContext,
						     const Acts::Surface& surface,
						     const Acts::Vector3& bField) const 
  {
    // Get SPs
    const auto& sp_collection = seed.sp();
    if ( sp_collection.size() < 3 ) return std::nullopt;
    
    // Compute Bound parameters at surface
    auto params_result = m_useTopSp ?
      Acts::estimateTrackParamsFromSeed(geoContext,
                                        std::ranges::views::reverse(sp_collection),
                                        surface,
                                        bField) :
      Acts::estimateTrackParamsFromSeed(geoContext,
                                        sp_collection,
                                        surface,
                                        bField);
    
    if ( not params_result.ok() ) {
      return std::nullopt;
    }

    auto& params = params_result.value();

    if (m_useTopSp) {
      // reverse direction so momentum vector pointing outwards
      params = Acts::reflectBoundParameters(params);
    }

    Acts::EstimateTrackParamCovarianceConfig covarianceEstimationConfig = {
      .initialSigmas = {m_sigmaLoc0, m_sigmaLoc1, m_sigmaPhi, m_sigmaTheta, m_sigmaQOverP, m_sigmaT0},
      .initialSigmaPtRel = m_initialSigmaPtRel,
      .initialVarInflation = Eigen::Map<const Acts::BoundVector>(m_initialVarInflation.value().data()),
      .noTimeVarInflation = 1.0,
    };
    Acts::BoundMatrix covariance = Acts::estimateTrackParamCovariance(covarianceEstimationConfig, params, false);

    // Create BoundTrackParameters
    return Acts::BoundTrackParameters(surface.getSharedPtr(),
                                      params,
                                      covariance,
                                      Acts::ParticleHypothesis::pion());
  }
  
}
// namespace ActsTrk
