/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackToTrackParticleCnvTool.h"

#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Propagator/detail/JacobianEngine.hpp"
#include "ActsInterop/Logger.h"

#include "MagFieldElements/AtlasFieldCache.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "ActsEvent/ParticleHypothesisEncoding.h"
#include "src/detail/CurvilinearCovarianceHelper.h"
#include "src/detail/HitSummaryDataUtils.h"
#include "ActsEvent/ExpectedHitUtils.h"
#include "MuonTrackEvent/HitSummary.h"

#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Utilities/Helpers.hpp>
#include <Acts/Utilities/MathHelpers.hpp>
#include <Acts/Definitions/Tolerance.hpp>
#include <tuple>

namespace {
   constexpr float toFloat(const double x) {

      if (Acts::abs(x) < Acts::s_epsilon) {
         return 0.f;
      }
      constexpr double min = 3.*static_cast<double>(std::numeric_limits<float>::min());
      constexpr double max = static_cast<double>(std::numeric_limits<float>::max());
      const double clampedX = std::copysign(std::clamp(Acts::abs(x), min, max), x);

      return static_cast<float>(clampedX);
   }
    template <int nRowsMax, int nMatSize>
   inline void lowerTriangleToVector(const Acts::SquareMatrix<nMatSize>& covMatrix,
                                     std::vector<float>& vec) {
      assert( covMatrix.rows() == covMatrix.cols());
      static_assert(nRowsMax > 0);
      static_assert(nMatSize > 0);
      constexpr int nRows = std::min(nRowsMax, nMatSize);
      vec.clear();
      vec.reserve(Acts::sumUpToN(nRows));
      for (int i = 0; i < nRows; ++i) {
         for (int j = 0; j <= i; ++j) {
            vec.emplace_back(toFloat(covMatrix(i, j)));
         }
      }
   }

   template <int nRowsMax, int nMatSize>
   inline void lowerTriangleToVectorScaleLastRow(const Acts::SquareMatrix<nMatSize>& covMatrix,
                                                 std::vector<float>& vec,
                                                 const double last_element_scale) {
      vec.clear();
      static_assert(nRowsMax > 0);
      static_assert(nMatSize > 0);
      constexpr int nRows = std::min(nRowsMax, nMatSize);
      vec.reserve(Acts::sumUpToN(nRows));
      for (int i = 0; i < nRows; ++i) {
         for (int j = 0; j <= i; ++j) {
            const double covVal = covMatrix(i,j) * 
               ( i == Acts::eBoundQOverP || j == Acts::eBoundQOverP ? 
                              last_element_scale : 1.);
            vec.emplace_back(toFloat(covVal));
         }
      }
   }

   void setSummaryValue(xAOD::TrackParticle& track_particle, uint8_t value, xAOD::SummaryType summary_type) {
      uint8_t tmp = value;
      track_particle.setSummaryValue(tmp, summary_type);
   }

   std::array<unsigned short, Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)> makeMeasurementToSummaryTypeMap() {
      std::array<unsigned short, Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)> ret;
      for (unsigned short& elm : ret) {
         elm = xAOD::numberOfTrackSummaryTypes;
      }
      ret.at(Acts::toUnderlying(xAOD::UncalibMeasType::PixelClusterType)) = xAOD::numberOfPixelHits;
      ret.at(Acts::toUnderlying(xAOD::UncalibMeasType::StripClusterType)) = xAOD::numberOfSCTHits;
      return ret;
   }
}

namespace ActsTrk {

   StatusCode TrackToTrackParticleCnvTool::initialize()
   {
      ATH_CHECK( m_trackingGeometryTool.retrieve() );
      ATH_CHECK( m_extrapolationTool.retrieve() );
      ATH_CHECK( m_fieldCacheCondObjInputKey.initialize() );
      ATH_CHECK( m_muonSummaryTool.retrieve(EnableTool{!m_muonSummaryTool.empty()}));

      // propagator for conversion to curvilinear parameters
      {
         auto logger = makeActsAthenaLogger(this, "Prop");
         Navigator::Config cfg{m_trackingGeometryTool->trackingGeometry()};
         cfg.resolvePassive = false;
         cfg.resolveMaterial = true;
         cfg.resolveSensitive = true;
         auto navigtor_logger = logger->cloneWithSuffix("Navigator");
         m_propagator = std::make_unique<Propagator>(Stepper(std::make_shared<ATLASMagneticFieldWrapper>()),
                                                     Navigator(cfg, std::move(navigtor_logger)),
                                                     std::move(logger));
      }

      return StatusCode::SUCCESS;
   }

   StatusCode TrackToTrackParticleCnvTool::convert(xAOD::TrackParticle& track_particle,
                                                   const EventContext& ctx,
                                                   const ActsTrk::TrackContainer::ConstTrackProxy& track,
                                                   const Acts::Surface& perigeeSurface,
                                                   const InDet::BeamSpotData* beamspot_data) const {
      using namespace Acts::UnitLiterals;

      const AtlasFieldCacheCondObj* field_cond_data{nullptr};
      ATH_CHECK(SG::get(field_cond_data, m_fieldCacheCondObjInputKey, ctx));
      MagField::AtlasFieldCache fieldCache;
      field_cond_data->getInitializedCache(fieldCache);

      if (m_muonSummaryTool.isEnabled()) {
         m_muonSummaryTool->copySummary(m_muonSummaryTool->makeSummary(ctx, track),
                                        track_particle);
      }
      const GeometryContext& gctx = m_trackingGeometryTool->getGeometryContext(ctx);

      static const std::array<unsigned short, Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)>
         measurementToSummaryType ATLAS_THREAD_SAFE (makeMeasurementToSummaryTypeMap());

      // re-used temporaries
      std::vector<float> tmp_cov_vector;
      std::vector<ActsTrk::TrackStateBackend::ConstTrackStateProxy::IndexType> tmp_param_state_idx;
      tmp_param_state_idx.reserve(30);
      Amg::Vector3D magnFieldVect;
      std::vector<std::vector<float>> parametersVec;
      ActsTrk::detail::HitSummaryData hitInfo;

      // convert defining parameters
      Acts::BoundTrackParameters perigeeParam = [&] {
         if (&perigeeSurface == &track.referenceSurface()) {
            return track.createParametersAtReference();
         } else {
            return parametersAtPerigee(ctx, track, perigeeSurface);
         }
      }();

      Acts::BoundVector boundParams = perigeeParam.parameters();
      track_particle.setDefiningParameters(boundParams[Acts::eBoundLoc0],
                                           boundParams[Acts::eBoundLoc1],
                                           boundParams[Acts::eBoundPhi],
                                           boundParams[Acts::eBoundTheta],
                                           boundParams[Acts::eBoundQOverP] * 1_MeV);

      if (perigeeParam.covariance().has_value()) {
         lowerTriangleToVectorScaleLastRow<5>(perigeeParam.covariance().value(), tmp_cov_vector, 1_MeV);
         track_particle.setDefiningParametersCovMatrixVec(tmp_cov_vector);
      }

      // optional beam tilt
      if (beamspot_data) {
         track_particle.setBeamlineTiltX(beamspot_data->beamTilt(0));
         track_particle.setBeamlineTiltY(beamspot_data->beamTilt(1));
      }

      // fit info, quality
      track_particle.setFitQuality(track.chi2(), track.nDoF());
      track_particle.setPatternRecognitionInfo(m_patternRecognitionInfo.value());
      track_particle.setTrackFitter(static_cast<xAOD::TrackFitter>(m_trackFitter.value()));

      const Acts::ParticleHypothesis& hypothesis = track.particleHypothesis();
      track_particle.setParticleHypothesis(ParticleHypothesis::convert(hypothesis));
      constexpr float inv_1_MeV = 1 / 1_MeV;

      std::array<std::array<uint8_t, Acts::toUnderlying(ActsTrk::detail::HitCategory::N)>,
                 Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)> specialHitCounts{};

      ActsTrk::detail::SumOfValues chi2_stat;
      gatherTrackSummaryData(track,
                             measurementToSummaryType,
                             chi2_stat,
                             hitInfo,
                             tmp_param_state_idx,
                             specialHitCounts);

      // pixel summaries
      std::array<std::tuple<uint8_t, uint8_t, uint8_t, bool>, 4> copy_summary {
         std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelTotal),
                         static_cast<uint8_t>(xAOD::numberOfContribPixelLayers),
                         static_cast<uint8_t>(xAOD::numberOfPixelHits),
                         false),

         std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat),
                         static_cast<uint8_t>(xAOD::numberOfContribPixelBarrelFlatLayers),
                         static_cast<uint8_t>(xAOD::numberOfPixelBarrelFlatHits),
                         true),

         std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelBarrelInclined),
                         static_cast<uint8_t>(xAOD::numberOfContribPixelBarrelInclinedLayers),
                         static_cast<uint8_t>(xAOD::numberOfPixelBarrelInclinedHits),
                         true),

         std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelEndcap),
                         static_cast<uint8_t>(xAOD::numberOfContribPixelEndcap),
                         static_cast<uint8_t>(xAOD::numberOfPixelEndcapHits),
                         true) };

      for (auto [src_region, dest_xaod_summary_layer, dest_xaod_summary_hits, add_outlier] : copy_summary) {
         setSummaryValue(track_particle,
                         hitInfo.contributingLayers(static_cast<ActsTrk::detail::HitSummaryData::DetectorRegion>(src_region)),
                         static_cast<xAOD::SummaryType>(dest_xaod_summary_layer));
         setSummaryValue(track_particle,
                         hitInfo.contributingHits(static_cast<ActsTrk::detail::HitSummaryData::DetectorRegion>(src_region))
                         + (add_outlier
                            ? hitInfo.contributingOutlierHits(static_cast<ActsTrk::detail::HitSummaryData::DetectorRegion>(src_region))
                            : 0),
                         static_cast<xAOD::SummaryType>(dest_xaod_summary_hits));
      }
      setSummaryValue(track_particle,
                      hitInfo.sum<ActsTrk::detail::HitSummaryData::Hit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 0)
                      + hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap, 0),
                      xAOD::numberOfInnermostPixelLayerEndcapHits);
      setSummaryValue(track_particle,
                      hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap, 0),
                      xAOD::numberOfInnermostPixelLayerEndcapOutliers);
      setSummaryValue(track_particle,
                      hitInfo.sum<ActsTrk::detail::HitSummaryData::Hit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 1)
                      + hitInfo.sum<ActsTrk::detail::HitSummaryData::Hit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 2)
                      + hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap, 1)
                      + hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap, 2),
                      xAOD::numberOfNextToInnermostPixelLayerEndcapHits);
      setSummaryValue(track_particle,
                      hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap, 1)
                      + hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelEndcap, 2),
                      xAOD::numberOfNextToInnermostPixelLayerEndcapOutliers);
      setSummaryValue(track_particle,
                      hitInfo.contributingOutlierHits(ActsTrk::detail::HitSummaryData::pixelTotal),
                      xAOD::numberOfPixelOutliers);
      setSummaryValue(track_particle,
                      specialHitCounts[Acts::toUnderlying(xAOD::UncalibMeasType::PixelClusterType)][Acts::toUnderlying(ActsTrk::detail::HitCategory::Hole)],
                      xAOD::numberOfPixelHoles);
      setSummaryValue(track_particle,
                      hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 0),
                      xAOD::numberOfInnermostPixelLayerSharedEndcapHits);
      setSummaryValue(track_particle,
                      hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 1)
                      + hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 2),
                      xAOD::numberOfNextToInnermostPixelLayerSharedEndcapHits);
      setSummaryValue(track_particle,
                      hitInfo.contributingSharedHits(ActsTrk::detail::HitSummaryData::pixelTotal),
                      xAOD::numberOfPixelSharedHits);

      // expected layer pattern
      std::array<unsigned int, 4> expect_layer_pattern{};
      if (detail::ExpectedLayerPatternHelper::exists(track.container())) {
         expect_layer_pattern = detail::ExpectedLayerPatternHelper::get(track);
      } else {
         expect_layer_pattern = (m_computeExpectedLayerPattern.value()
                                 && (!m_expectIfPixelContributes.value()
                                     || hitInfo.contributingLayers(ActsTrk::detail::HitSummaryData::pixelTotal))
                                 ? detail::expectedLayerPattern(ctx,
                                                                *m_extrapolationTool,
                                                                perigeeParam,
                                                                m_pixelExpectLayerPathLimitInMM.value() * Acts::UnitConstants::mm)
                                 : std::array<unsigned int, 4>{0u, 0u, 0u, 0u});
      }

      // @TODO consider end-caps for inner most pixel hits ?
      setSummaryValue(track_particle,
                      static_cast<uint8_t>((expect_layer_pattern[0] & (1<<0)) != 0),
                      xAOD::expectInnermostPixelLayerHit);
      setSummaryValue(track_particle,
                      static_cast<uint8_t>((expect_layer_pattern[0] & (1<<1)) != 0),
                      xAOD::expectNextToInnermostPixelLayerHit);
      setSummaryValue(track_particle,
                      static_cast<unsigned int>(hitInfo.sum<ActsTrk::detail::HitSummaryData::Hit>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat, 0)),
                      xAOD::numberOfInnermostPixelLayerHits);
      setSummaryValue(track_particle,
                      static_cast<unsigned int>(hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat, 0)),
                      xAOD::numberOfInnermostPixelLayerOutliers);
      setSummaryValue(track_particle,
                      static_cast<unsigned int>(hitInfo.sum<ActsTrk::detail::HitSummaryData::Hit>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat, 1)),
                      xAOD::numberOfNextToInnermostPixelLayerHits);
      setSummaryValue(track_particle,
                      static_cast<unsigned int>(hitInfo.sum<ActsTrk::detail::HitSummaryData::Outlier>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat, 1)),
                      xAOD::numberOfNextToInnermostPixelLayerOutliers);
      setSummaryValue(track_particle,
                      static_cast<unsigned int>(hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat, 0)),
                      xAOD::numberOfInnermostPixelLayerSharedHits);
      setSummaryValue(track_particle,
                      static_cast<unsigned int>(hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat, 1)),
                      xAOD::numberOfNextToInnermostPixelLayerSharedHits);

      // Strip summaries
      setSummaryValue(track_particle,
                      hitInfo.contributingHits(ActsTrk::detail::HitSummaryData::stripTotal),
                      xAOD::numberOfSCTHits);
      setSummaryValue(track_particle,
                      hitInfo.contributingOutlierHits(ActsTrk::detail::HitSummaryData::stripTotal),
                      xAOD::numberOfSCTOutliers);
      setSummaryValue(track_particle,
                      hitInfo.contributingSharedHits(ActsTrk::detail::HitSummaryData::stripTotal),
                      xAOD::numberOfSCTSharedHits);
      setSummaryValue(track_particle,
                      specialHitCounts[Acts::toUnderlying(xAOD::UncalibMeasType::StripClusterType)][Acts::toUnderlying(ActsTrk::detail::HitCategory::Hole)],
                      xAOD::numberOfSCTHoles);

      double biased_chi2_variance = chi2_stat.biasedVariance();
      setSummaryValue(track_particle,
                      static_cast<uint8_t>(biased_chi2_variance > 0.
                                           ? std::min(static_cast<unsigned int>(std::sqrt(biased_chi2_variance) * 100), 255u)
                                           : 0u),
                      xAOD::standardDeviationOfChi2OS);

      setSummaryValue(track_particle,
                      hitInfo.contributingOutlierHits(ActsTrk::detail::HitSummaryData::pixelTotal)
                      + hitInfo.contributingOutlierHits(ActsTrk::detail::HitSummaryData::stripTotal),
                      xAOD::numberOfOutliersOnTrack);

      // @TODO select states for which parameters are stored
      if (m_firstAndLastParamOnly && tmp_param_state_idx.size() > 2) {
         tmp_param_state_idx[1] = tmp_param_state_idx.back();
         tmp_param_state_idx.erase(tmp_param_state_idx.begin() + 2, tmp_param_state_idx.end());
      }

      // store track parameters and covariances for selected states
      parametersVec.clear();
      parametersVec.reserve(tmp_param_state_idx.size());

      for (std::vector<ActsTrk::TrackStateBackend::ConstTrackStateProxy::IndexType>::const_reverse_iterator
              idx_iter = tmp_param_state_idx.rbegin();
           idx_iter != tmp_param_state_idx.rend();
           ++idx_iter) {
         ActsTrk::TrackStateBackend::ConstTrackStateProxy
            state = track.container().trackStateContainer().getTrackState(*idx_iter);
         const Acts::BoundTrackParameters actsParam = track.createParametersFromState(state);

         Acts::Vector3 position = actsParam.position(gctx.context());
         Acts::Vector3 momentum = actsParam.momentum();

         // scaling from Acts momentum units (GeV) to Athena Units (MeV)
         for (unsigned int i = 0; i < momentum.rows(); ++i) {
            momentum(i) *= inv_1_MeV;
         }

         if (actsParam.covariance()) {
            Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
            Acts::GeometryContext tgContext = gctx.context();

            magnFieldVect.setZero();
            fieldCache.getField(position.data(), magnFieldVect.data());
            // scaling from Athena magnetic field units kT to Acts units T
            {
               using namespace Acts::UnitLiterals;
               magnFieldVect *= 1000_T;
            }

            auto curvilinear_cov_result = ActsTrk::detail::convertActsBoundCovToCurvilinearParam(tgContext, actsParam, magnFieldVect, hypothesis);
            if (curvilinear_cov_result.has_value()) {
               Acts::BoundMatrix& curvilinear_cov = curvilinear_cov_result.value();

               // convert q/p components from GeV (Acts) to MeV (Athena)
               for (unsigned int col_i = 0; col_i < 4; ++col_i) {
                  curvilinear_cov(col_i, 4) *= 1_MeV;
                  curvilinear_cov(4, col_i) *= 1_MeV;
               }
               curvilinear_cov(4, 4) *= (1_MeV * 1_MeV);

               std::size_t param_idx = parametersVec.size();
               // only use the 5x5 sub-matrix of the full covariance matrix
               lowerTriangleToVector<5>(curvilinear_cov, tmp_cov_vector);
               if (tmp_cov_vector.size() != 15) {
                  ATH_MSG_ERROR("Invalid size of lower triangle cov " << tmp_cov_vector.size() << " != 15"
                                << " input matrix : " << curvilinear_cov.rows() << " x " << curvilinear_cov.cols());
               }
               track_particle.setTrackParameterCovarianceMatrix(param_idx, tmp_cov_vector);
            }
         }
         parametersVec.emplace_back(std::vector<float>{
            static_cast<float>(position[0]), static_cast<float>(position[1]), static_cast<float>(position[2]),
            static_cast<float>(momentum[0]), static_cast<float>(momentum[1]), static_cast<float>(momentum[2]) });
      }
      for (const std::vector<float>& param : parametersVec) {
         if (param.size() != 6) {
            ATH_MSG_ERROR("Invalid size of param element " << param.size() << " != 6");
         }
      }

      track_particle.setTrackParameters(parametersVec);
      if( !parametersVec.empty() ) {
         track_particle.setParameterPosition(0, xAOD::ParameterPosition::FirstMeasurement);
         track_particle.setParameterPosition(parametersVec.size()-1, xAOD::ParameterPosition::LastMeasurement);
      }


      return StatusCode::SUCCESS;
   }

   Acts::BoundTrackParameters TrackToTrackParticleCnvTool::parametersAtPerigee(const EventContext& ctx,
                                                                               const ActsTrk::TrackContainer::ConstTrackProxy& track,
                                                                               const Acts::Surface& perigee_surface) const {
      const Acts::BoundTrackParameters trackParam = track.createParametersAtReference();

      Acts::Result<Acts::BoundTrackParameters>
         perigeeParam = m_extrapolationTool->propagate(ctx,
                                                       trackParam,
                                                       perigee_surface,
                                                       Acts::Direction::Backward(),
                                                       m_paramExtrapolationParLimit.value());
      if (!perigeeParam.ok()) {
         ATH_MSG_WARNING("Failed to extrapolate to perigee, started from \n" << trackParam << " " << trackParam.referenceSurface().name());
         return trackParam;
      }

      return perigeeParam.value();
   }

}
