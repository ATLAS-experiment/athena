/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackToTrackParticleCnvTool.h"

#include "xAODMeasurementBase/MeasurementDefs.h"
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

#include "src/detail/CurvilinearCovarianceHelper.h"
#include "src/detail/HitSummaryDataUtils.h"
#include "src/detail/ExpectedHitUtils.h"

#include <Acts/Definitions/TrackParametrization.hpp>
#include <tuple>

namespace {

   template <typename T, class T_SquareMatrix>
   inline void lowerTriangleToVector(const T_SquareMatrix& covMatrix,
                                     std::vector<T>& vec, unsigned int n_rows_max) {
      assert( covMatrix.rows() == covMatrix.cols());
      vec.clear();
      unsigned int n_rows = std::min(n_rows_max, static_cast<unsigned int>(covMatrix.rows()));
      vec.reserve((n_rows+1)*n_rows/2);
      for (unsigned int i = 0; i < n_rows; ++i) {
         for (unsigned int j = 0; j <= i; ++j) {
            vec.emplace_back(covMatrix(i, j));
         }
      }
   }

   template <typename T, class T_SquareMatrix>
   inline void lowerTriangleToVectorScaleLastRow(const T_SquareMatrix& covMatrix,
                                                 std::vector<T>& vec, unsigned int n_rows_max,
                                                 typename T_SquareMatrix::Scalar last_element_scale) {
      assert( covMatrix.rows() == covMatrix.cols());
      vec.clear();
      unsigned int n_rows = std::min(n_rows_max, static_cast<unsigned int>(covMatrix.rows()));
      vec.reserve((n_rows+1)*n_rows/2);
      for (unsigned int i = 0; i < n_rows; ++i) {
         for (unsigned int j = 0; j <= i; ++j) {
            vec.emplace_back(covMatrix(i, j));
         }
      }
      typename std::vector<T>::iterator cov_iter = vec.end();
      --cov_iter;
      *cov_iter *= last_element_scale;
      for (unsigned int i=0; i<n_rows_max; ++i) {
         *cov_iter *= last_element_scale;
         --cov_iter;
      }
   }

   void setSummaryValue(xAOD::TrackParticle& track_particle, uint8_t value, xAOD::SummaryType summary_type) {
      uint8_t tmp = value;
      track_particle.setSummaryValue(tmp, summary_type);
   }

   std::array<unsigned short, ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::nTypes)> makeMeasurementToSummaryTypeMap() {
      std::array<unsigned short, ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::nTypes)> ret;
      for (unsigned short& elm : ret) {
         elm = xAOD::numberOfTrackSummaryTypes;
      }
      ret.at(ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::PixelClusterType)) = xAOD::numberOfPixelHits;
      ret.at(ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::StripClusterType)) = xAOD::numberOfSCTHits;
      return ret;
   }
}

namespace ActsTrk {

   xAOD::ParticleHypothesis ActsTrk::TrackToTrackParticleCnvTool::convertParticleHypothesis(Acts::PdgParticle abs_pdg_id) {
     static const std::array map {
       std::pair{Acts::eElectron, xAOD::electron},
       std::pair{Acts::eMuon,     xAOD::muon},
       std::pair{Acts::ePionPlus, xAOD::pion},
       std::pair{Acts::eProton,   xAOD::proton},
       std::pair{Acts::ePionZero, xAOD::pi0},
       std::pair{Acts::eNeutron,  xAOD::neutron},
       std::pair{Acts::eGamma,    xAOD::photon},
     };
     auto iter = std::find_if(
       map.begin(), map.end(),
       [abs_pdg_id](const auto& elm) {
         return abs_pdg_id == elm.first;
       });
     return (iter != map.end() ? iter->second : xAOD::noHypothesis);
   }

   TrackToTrackParticleCnvTool::TrackToTrackParticleCnvTool(const std::string& type,
                                                            const std::string& name,
                                                            const IInterface* parent)
      : base_class(type, name, parent)
   {
   }

   StatusCode TrackToTrackParticleCnvTool::initialize()
   {
      ATH_CHECK( m_trackingGeometryTool.retrieve() );
      ATH_CHECK( m_extrapolationTool.retrieve() );
      ATH_CHECK( m_fieldCacheCondObjInputKey.initialize() );
      ATH_CHECK( m_siDetEleCollKey.initialize() );

      if (m_siDetEleCollToMeasurementType.size() == m_siDetEleCollKey.size()) {
         unsigned int collection_idx = 0;
         for (int type : m_siDetEleCollToMeasurementType) {
            if (type < 1 || type > 2) {
               ATH_MSG_ERROR("Invalid measurement type (" << type << ") given for collection " << collection_idx << " : "
                             << m_siDetEleCollKey[collection_idx].key()
                             << ". Expected 1 for pixel, 2 for strips.");
               return StatusCode::FAILURE;
            }
            ++collection_idx;
         }
      } else {
         ATH_MSG_ERROR("Expected exactly one value in SiDetEleCollToMeasurementType per SiDetectorElementCollection. But got "
                       << m_siDetEleCollToMeasurementType.size() << " instead of " << m_siDetEleCollKey.size() << ".");
         return StatusCode::FAILURE;
      }

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

   StatusCode TrackToTrackParticleCnvTool::convert(
      xAOD::TrackParticle& track_particle,
      const EventContext& ctx,
      const ActsTrk::TrackContainer::ConstTrackProxy& track,
      const Acts::PerigeeSurface* perigeeSurface,
      const InDet::BeamSpotData* beamspot_data) const
   {
      using namespace Acts::UnitLiterals;

      SG::ReadCondHandle<AtlasFieldCacheCondObj> fieldHandle = SG::makeHandle(m_fieldCacheCondObjInputKey, ctx);
      ATH_CHECK(fieldHandle.isValid());
      const AtlasFieldCacheCondObj* field_cond_data = fieldHandle.cptr();
      MagField::AtlasFieldCache fieldCache;
      field_cond_data->getInitializedCache(fieldCache);

      const GeometryContext& gctx = m_trackingGeometryTool->getNominalGeometryContext();

      std::array<const InDetDD::SiDetectorElementCollection*, ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::nTypes)> siDetEleColl{};
      for (unsigned int idx = 0; idx < m_siDetEleCollToMeasurementType.size(); ++idx) {
         SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> detHandle = SG::makeHandle(m_siDetEleCollKey[idx], ctx);
         ATH_CHECK(detHandle.isValid());
         siDetEleColl[m_siDetEleCollToMeasurementType[idx]] = detHandle.cptr();
      }

      static const std::array<unsigned short, ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::nTypes)>
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
         if (perigeeSurface == nullptr) {
            return track.createParametersAtReference();
         } else {
            return parametersAtPerigee(ctx, track, *perigeeSurface);
         }
      }();

      Acts::BoundVector boundParams = perigeeParam.parameters();
      track_particle.setDefiningParameters(boundParams[Acts::eBoundLoc0],
                                           boundParams[Acts::eBoundLoc1],
                                           boundParams[Acts::eBoundPhi],
                                           boundParams[Acts::eBoundTheta],
                                           boundParams[Acts::eBoundQOverP] * 1_MeV);

      if (perigeeParam.covariance().has_value()) {
         lowerTriangleToVectorScaleLastRow(perigeeParam.covariance().value(), tmp_cov_vector, 5, 1_MeV);
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
      track_particle.setParticleHypothesis(convertParticleHypothesis(hypothesis.absolutePdg()));
      constexpr float inv_1_MeV = 1 / 1_MeV;

      std::array<std::array<uint8_t, ActsTrk::detail::to_underlying(ActsTrk::detail::HitCategory::N)>,
                 ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::nTypes)> specialHitCounts{};

      ActsTrk::detail::SumOfValues chi2_stat;
      gatherTrackSummaryData(track,
                             siDetEleColl,
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
                      specialHitCounts[ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::PixelClusterType)][ActsTrk::detail::HitCategory::Hole],
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
                      specialHitCounts[ActsTrk::detail::to_underlying(xAOD::UncalibMeasType::StripClusterType)][ActsTrk::detail::HitCategory::Hole],
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
               lowerTriangleToVector(curvilinear_cov, tmp_cov_vector, 5);
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

      return StatusCode::SUCCESS;
   }

   Acts::BoundTrackParameters TrackToTrackParticleCnvTool::parametersAtPerigee(
      const EventContext& ctx,
      const ActsTrk::TrackContainer::ConstTrackProxy& track,
      const Acts::PerigeeSurface& perigee_surface) const
   {
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
