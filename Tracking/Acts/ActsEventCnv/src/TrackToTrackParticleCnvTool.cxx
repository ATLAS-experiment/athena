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

      if (std::abs(x) < Acts::s_epsilon) {
         return 0.f;
      }
      constexpr double min = 3.*static_cast<double>(std::numeric_limits<float>::min());
      constexpr double max = static_cast<double>(std::numeric_limits<float>::max());
      const double clampedX = std::copysign(std::clamp(std::abs(x), min, max), x);

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
      ret.at(Acts::toUnderlying(xAOD::UncalibMeasType::HGTDClusterType))  = xAOD::numberOfHGTDHits;
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

      if (m_hgtdDecorationLevel>0) {
         static const SG::Accessor<float> perigeeTime("time");
         perigeeTime(track_particle) = ActsTrk::timeToAthena(boundParams[Acts::eBoundTime]);
      }

      if (perigeeParam.covariance().has_value()) {
         lowerTriangleToVectorScaleLastRow<5>(perigeeParam.covariance().value(), tmp_cov_vector, 1_MeV);
         track_particle.setDefiningParametersCovMatrixVec(tmp_cov_vector);
         if (m_hgtdDecorationLevel>0) {
            static const SG::Accessor<float> perigeeTimeResolution("timeResolution");
            perigeeTimeResolution(track_particle) = ActsTrk::timeToAthena(perigeeParam.covariance().value()(Acts::eBoundTime,Acts::eBoundTime));
         }
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

      ActsTrk::detail::TimeInfo time_info;
      ActsTrk::detail::SumOfValues chi2_stat;
      gatherTrackSummaryData(track,
                             measurementToSummaryType,
                             chi2_stat,
                             hitInfo,
                             tmp_param_state_idx,
                             specialHitCounts,
                             time_info);

      // pixel summaries
      static constexpr std::array<std::tuple<uint8_t, uint8_t, uint8_t, bool>, 5> copy_summary {
         std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelTotal),
                         static_cast<uint8_t>(xAOD::numberOfContribPixelLayers),
                         static_cast<uint8_t>(xAOD::numberOfPixelHits),
                         false),
         std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelBarrel),
                         static_cast<uint8_t>(xAOD::numberOfContribPixelBarrelLayers),
                         static_cast<uint8_t>(xAOD::numberOfPixelBarrelHits),
                         true),
         std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelEndcap),
                         static_cast<uint8_t>(xAOD::numberOfContribPixelEndcap),
                         static_cast<uint8_t>(xAOD::numberOfPixelEndcapHits),
                         true),
         std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelBarrelFlat),
                         static_cast<uint8_t>(xAOD::numberOfContribPixelBarrelFlatLayers),
                         static_cast<uint8_t>(xAOD::numberOfPixelBarrelFlatHits),
                         true),
         std::make_tuple(static_cast<uint8_t>(ActsTrk::detail::HitSummaryData::pixelBarrelInclined),
                         static_cast<uint8_t>(xAOD::numberOfContribPixelBarrelInclinedLayers),
                         static_cast<uint8_t>(xAOD::numberOfPixelBarrelInclinedHits),
                         true)
      };

      // if not adding expert level decorations only set the total
      for (auto [src_region, dest_xaod_summary_layer, dest_xaod_summary_hits, add_outlier] : std::span(copy_summary.begin(),
                                                                                                       m_itkDecorationLevel>=s_expertLevel
                                                                                                       ? copy_summary.end()
                                                                                                       : copy_summary.begin()+1)) {
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
                      specialHitCounts[Acts::toUnderlying(xAOD::UncalibMeasType::PixelClusterType)][Acts::toUnderlying(ActsTrk::detail::HitCategory::Hole)],
                      xAOD::numberOfPixelHoles);
      setSummaryValue(track_particle,
                      hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 0),
                      xAOD::numberOfInnermostPixelLayerSharedEndcapHits);
      setSummaryValue(track_particle,
                      hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 1)
                      + hitInfo.sum<ActsTrk::detail::HitSummaryData::SharedHit>(ActsTrk::detail::HitSummaryData::pixelEndcap, 2),
                      xAOD::numberOfNextToInnermostPixelLayerSharedEndcapHits);

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

      // Strip, HGTD and seom pixel summaries
      std::array<std::tuple<ActsTrk::detail::HitSummaryData::DetectorRegion,
                            ActsTrk::detail::HitSummaryData::CountType,
                            xAOD::SummaryType> ,8 > copy_summary_types = {
         // pixel _hits_ are copied above
         std::make_tuple(ActsTrk::detail::HitSummaryData::pixelTotal,ActsTrk::detail::HitSummaryData::CountType::Outlier,xAOD::numberOfPixelOutliers),
         std::make_tuple(ActsTrk::detail::HitSummaryData::pixelTotal, ActsTrk::detail::HitSummaryData::CountType::SharedHit, xAOD::numberOfPixelSharedHits),
         std::make_tuple(ActsTrk::detail::HitSummaryData::stripTotal,ActsTrk::detail::HitSummaryData::CountType::Hit,xAOD::numberOfSCTHits),
         std::make_tuple(ActsTrk::detail::HitSummaryData::stripTotal,ActsTrk::detail::HitSummaryData::CountType::Outlier,xAOD::numberOfSCTOutliers),
         std::make_tuple(ActsTrk::detail::HitSummaryData::stripTotal,ActsTrk::detail::HitSummaryData::CountType::SharedHit,xAOD::numberOfSCTSharedHits),
         std::make_tuple(ActsTrk::detail::HitSummaryData::hgtdTotal,ActsTrk::detail::HitSummaryData::CountType::Hit,xAOD::numberOfHGTDHits),
         std::make_tuple(ActsTrk::detail::HitSummaryData::hgtdTotal,ActsTrk::detail::HitSummaryData::CountType::Outlier,xAOD::numberOfHGTDOutliers),
         std::make_tuple(ActsTrk::detail::HitSummaryData::hgtdTotal,ActsTrk::detail::HitSummaryData::CountType::SharedHit,xAOD::numberOfHGTDSharedHits)
      };

      for (auto [region,count_type,dest_summary_type] : std::span(copy_summary_types.begin(),
                                                                  copy_summary_types.begin()+(m_hgtdDecorationLevel>0
                                                                                              ? copy_summary_types.size()
                                                                                              : copy_summary_types.size()-3) )) {
         setSummaryValue(track_particle,hitInfo.contributingHits(region, count_type),dest_summary_type);
      }
      setSummaryValue(track_particle,
                      specialHitCounts[Acts::toUnderlying(xAOD::UncalibMeasType::StripClusterType)][Acts::toUnderlying(ActsTrk::detail::HitCategory::Hole)],
                      xAOD::numberOfSCTHoles);
      if (m_hgtdDecorationLevel>0) {
        setSummaryValue(
              track_particle,
              specialHitCounts[Acts::toUnderlying(xAOD::UncalibMeasType::HGTDClusterType)][Acts::toUnderlying(ActsTrk::detail::HitCategory::Hole)],
              xAOD::numberOfHGTDHoles);
      }

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

      if (m_hgtdDecorationLevel>0) {
         static const SG::Accessor<uint8_t> hasValidTime("hasValidTime");
         static const SG::Accessor<uint32_t> hgtdSummary("HGTDSummaryinfo");
         using HitSummaryData=ActsTrk::detail::HitSummaryData;
         unsigned int n_hgtd_hits = hitInfo.contributingHits(static_cast<HitSummaryData::DetectorRegion>(HitSummaryData::hgtdTotal));
         unsigned int n_hgtd_outliers = hitInfo.contributingOutlierHits(static_cast<HitSummaryData::DetectorRegion>(HitSummaryData::hgtdTotal));
         hasValidTime(track_particle) = n_hgtd_hits > 2 || n_hgtd_hits>n_hgtd_outliers;
         unsigned int hgtd_hit_pattern = (n_hgtd_hits>0u
                                          ? hitInfo.layerPattern(static_cast<HitSummaryData::DetectorRegion>(HitSummaryData::hgtdTotal),
                                                                 true /* include outlier */)
                                          : 0u);
         hgtdSummary(track_particle) = hgtd_hit_pattern;
         if (m_hgtdDecorationLevel>=s_expertLevel) {
            static const SG::Accessor<float> meanTime("HGTDMeanTime");
            static const SG::Accessor<float> timeResolution("HGTDMeanTimeResolution");
            static const SG::Accessor<float> hgtdChi2("HGTDChi2");
            meanTime(track_particle) = time_info.mean;
            timeResolution(track_particle) = time_info.resolution;
            hgtdChi2(track_particle) = static_cast<float>(time_info.chi2);
         }
      }

      // @TODO select states for which parameters are stored
      if (m_firstAndLastParamOnly && tmp_param_state_idx.size() > 2) {
         tmp_param_state_idx[1] = tmp_param_state_idx.back();
         tmp_param_state_idx.erase(tmp_param_state_idx.begin() + 2, tmp_param_state_idx.end());
      }

      // store track parameters and covariances for selected states
      parametersVec.clear();
      parametersVec.reserve(tmp_param_state_idx.size());

      // Check if this is a seed track (TSOS mask = None, no Predicted/Filtered/Calibrated)
      // For seed tracks, perigee parameters are already set from SeedsToTrackParamsAlg, skip per-TSOS loop
      bool isSeedTrack = tmp_param_state_idx.empty() ? false
         : track.container().trackStateContainer().getTrackState(tmp_param_state_idx.front()).getMask() == Acts::TrackStatePropMask::None;

      if (isSeedTrack) {
         ATH_MSG_DEBUG("Seed track detected, skipping per-TSOS parameter extraction");
      }
      else {
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
      }  // end else (isSeedTrack)
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
