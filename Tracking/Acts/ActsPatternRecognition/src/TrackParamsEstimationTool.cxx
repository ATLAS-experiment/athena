/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackParamsEstimationTool.h"
#include "xAODInDetMeasurement/SpacePoint.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"
#include "Acts/EventData/TransformationHelpers.hpp"

#include <algorithm>
#include <ranges>

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
    ATH_MSG_DEBUG( "   " << m_bFieldMode );
    ATH_MSG_DEBUG( "   " << m_firstSp );

    m_logger = makeActsAthenaLogger(this, "Acts");

    m_extrapolator = Extrapolator(Stepper(std::make_shared<ATLASMagneticFieldWrapper>()), Navigator(), logger().cloneWithSuffix("Prop"));

    m_spacePointIndicesFun = spacePointIndicesFun();

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
    const xAOD::SpacePoint* bottom_sp = (useTopSp && m_bFieldMode != 2) ? sp_collection.back() : sp_collection.front();

    // Magnetic Field
    ATLASMagneticFieldWrapper magneticField;
    Acts::MagneticFieldProvider::Cache magFieldCache = magneticField.makeCache( magFieldContext );
    Acts::Vector3 bField = *magneticField.getField( Acts::Vector3(bottom_sp->x(), bottom_sp->y(), bottom_sp->z()),
                                                    magFieldCache );
    if (m_bFieldMode == 1) {
        bField[0] = 0.0;
        bField[1] = 0.0;
    }

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
    const std::size_t nSp = sp_collection.size();
    if (nSp < 3) return std::nullopt;

    // Function to extract the values from sp_collection
    auto sp_collection_extract = std::views::transform([&sp_collection, useTopSp](std::size_t i) {
      return sp_collection.at(useTopSp ? sp_collection.size() - i - 1 : i);
    });

    // Compute free parameters
    Acts::FreeVector freeParams = Acts::estimateTrackParamsFromSeed(m_spacePointIndicesFun(nSp) | sp_collection_extract, bField);

    if (m_useLongSeeds == 1 && nSp > 3ul) {
      auto spacePointIndicesFun2 = [](std::size_t nSp) -> std::array<std::size_t, 3> {
        return {0, nSp / 2ul, nSp - 1};
      };
      Acts::FreeVector freeParams2 = Acts::estimateTrackParamsFromSeed(spacePointIndicesFun2(nSp) | sp_collection_extract, bField);
      ATH_MSG_DEBUG("update seed p = " << 1.0 / freeParams[Acts::eFreeQOverP] << " to " << 1.0 / freeParams2[Acts::eFreeQOverP]);
      freeParams[Acts::eFreeQOverP] = freeParams2[Acts::eFreeQOverP];
    }

    if (useTopSp) {
      // reverse direction so momentum vector pointing outwards
      freeParams = Acts::reflectFreeParameters(freeParams);
    }

    // Convert free params to curvilinear params for extrapolation
    Acts::BoundTrackParameters curvilinearParams = Acts::BoundTrackParameters::createCurvilinear(
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

  // Function to return which 3 SPs of a seed to use
  ITrackParamsEstimationTool::SpacePointIndicesFun_t TrackParamsEstimationTool::spacePointIndicesFun() const {
    if (m_useLongSeeds == 2) {
      return [](std::size_t nSp) -> std::array<std::size_t, 3> {
        if (nSp > 3ul)
          return {0, nSp / 2ul, nSp - 1};
        else
          return {0, 1, 2};
      };
    } else if (m_firstSp > 0ul) {
      std::size_t firstSp = m_firstSp;
      return [firstSp](std::size_t nSp) -> std::array<std::size_t, 3> {
        if (nSp > 3ul) {
          std::size_t first = std::min(firstSp, nSp - 3ul);
          return {first, first + 1, first + 2};
        } else
          return {0, 1, 2};
      };
    } else {
      return [](std::size_t) -> std::array<std::size_t, 3> {
        return {0, 1, 2};
      };
    }
  };

}
// namespace ActsTrk
