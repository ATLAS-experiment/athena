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

    m_logger = makeActsAthenaLogger(this, "Acts");

    m_extrapolator = Extrapolator(Stepper(std::make_shared<ATLASMagneticFieldWrapper>()), Navigator(), logger().cloneWithSuffix("Prop"));

    return StatusCode::SUCCESS;
  }

  std::optional<Acts::BoundTrackParameters>
  TrackParamsEstimationTool::estimateTrackParameters(
						     const ActsTrk::Seed& seed,
						     bool useTopSp,
						     const Acts::GeometryContext& geoContext,
						     const Acts::MagneticFieldContext& magFieldContext,
						     std::function<const Acts::Surface&(const ActsTrk::Seed& seed, bool useTopSp)> retrieveSurface) const 
  {
    const auto& sp_collection = seed.sp();
    if ( sp_collection.size() < 3 ) return std::nullopt;
    const auto& bottom_sp = useTopSp ? sp_collection.back() : sp_collection.front();

    // Magnetic Field
    ATLASMagneticFieldWrapper magneticField;
    Acts::MagneticFieldProvider::Cache magFieldCache = magneticField.makeCache( magFieldContext );
    Acts::Vector3 bField = *magneticField.getField( Acts::Vector3(bottom_sp->x(), bottom_sp->y(), bottom_sp->z()),
                                                    magFieldCache );

    // Get the surface
    const Acts::Surface& surface = retrieveSurface(seed, useTopSp);

    return estimateTrackParameters(
				   seed,
				   useTopSp,
				   geoContext,
				   magFieldContext,
				   surface,
				   bField);
  }

  std::optional<Acts::BoundTrackParameters>
  TrackParamsEstimationTool::estimateTrackParameters(
						     const ActsTrk::Seed& seed,
						     bool useTopSp,
						     const Acts::GeometryContext& geoContext,
						     const Acts::MagneticFieldContext& magFieldContext,
						     const Acts::Surface& surface,
						     const Acts::Vector3& bField) const 
  {
    // Get SPs
    const auto& sp_collection = seed.sp();
    if ( sp_collection.size() < 3 ) return std::nullopt;

    // Compute free parameters
    Acts::FreeVector freeParams = useTopSp ?
      Acts::estimateTrackParamsFromSeed(std::ranges::views::reverse(sp_collection),
                                        bField) :
      Acts::estimateTrackParamsFromSeed(sp_collection,
                                        bField);

    if (useTopSp) {
      // reverse direction so momentum vector pointing outwards
      freeParams = Acts::reflectFreeParameters(freeParams);
    }

    // Convert free params to curvilinear params for extrapolation
    Acts::CurvilinearTrackParameters curvilinearParams(
      freeParams.segment<4>(Acts::eFreePos0),
      freeParams.segment<3>(Acts::eFreeDir0),
      freeParams[Acts::eFreeQOverP],
      std::nullopt,
      Acts::ParticleHypothesis::pion());

    // Extrapolate to surface
    Acts::PropagatorPlainOptions propOptions(geoContext, magFieldContext);
    propOptions.direction = Acts::Direction::fromScalarZeroAsPositive(
        surface.intersect(
          geoContext,
          freeParams.segment<3>(Acts::eFreePos0),
          freeParams.segment<3>(Acts::eFreeDir0)
        ).closest().pathLength());
    auto boundParamsResult = m_extrapolator->propagateToSurface(curvilinearParams, surface, propOptions);
    if (!boundParamsResult.ok()) {
      ATH_MSG_DEBUG("Extrapolation failed");
      return std::nullopt;
    }

    // Get extrapolated parameters
    Acts::BoundTrackParameters boundParams = *boundParamsResult;

    // Estimate covariance
    Acts::EstimateTrackParamCovarianceConfig covarianceEstimationConfig = {
      .initialSigmas = {m_sigmaLoc0, m_sigmaLoc1, m_sigmaPhi, m_sigmaTheta, m_sigmaQOverP, m_sigmaT0},
      .initialSigmaPtRel = m_initialSigmaPtRel,
      .initialVarInflation = Eigen::Map<const Acts::BoundVector>(m_initialVarInflation.value().data()),
      .noTimeVarInflation = 1.0,
    };
    boundParams.covariance() = Acts::estimateTrackParamCovariance(
      covarianceEstimationConfig,
      boundParams.parameters(),
      false);

    return boundParams;
  }

}
// namespace ActsTrk
