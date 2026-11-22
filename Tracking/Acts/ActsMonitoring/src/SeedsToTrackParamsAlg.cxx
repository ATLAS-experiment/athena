/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SeedsToTrackParamsAlg.h"

#include "TrkSurfaces/Surface.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"

namespace ActsTrk {

  StatusCode SeedsToTrackParamsAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name() << " ...");

    ATH_CHECK(m_ctxProvider.initialize());
    ATH_CHECK(m_geometryConvTool.retrieve());
    ATH_CHECK(m_paramEstimationTool.retrieve());

    ATH_CHECK(m_detEleCollKey.initialize());
    ATH_CHECK(m_inputSeedContainerKey.initialize());
    ATH_CHECK(m_outputTrackParamsCollectionKey.initialize());
    ATH_CHECK(m_outputSeedMeasurementsKey.initialize(!m_outputSeedMeasurementsKey.empty()));

    if (m_useTopSpRZboundary.size() != 2) {
      ATH_MSG_FATAL("useTopSpRZboundary must have 2 elements, but has " << m_useTopSpRZboundary.size());
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode SeedsToTrackParamsAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name() << " ... ");

    const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
    const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
    const Acts::CalibrationContext calContext = m_ctxProvider.getCalibrationContext(ctx);

    SG::ReadHandle<ActsTrk::SeedContainer> seedContainerHandle = SG::makeHandle(m_inputSeedContainerKey, ctx);
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> detEleHandle = SG::makeHandle(m_detEleCollKey, ctx);

    ATH_CHECK(seedContainerHandle.isValid());
    ATH_CHECK(detEleHandle.isValid());

    SG::WriteHandle<ActsTrk::BoundTrackParametersContainer> boundTrackParamsHandle = SG::makeHandle(m_outputTrackParamsCollectionKey, ctx);
    ATH_MSG_DEBUG("    \\__ Track Params Estimated `" << m_outputTrackParamsCollectionKey.key() << "` created ...");
    ATH_CHECK(boundTrackParamsHandle.record(std::make_unique<ActsTrk::BoundTrackParametersContainer>()));
    ActsTrk::BoundTrackParametersContainer *trackParams = boundTrackParamsHandle.ptr();

    const ActsTrk::SeedContainer &seeds = *seedContainerHandle;
    const InDetDD::SiDetectorElementCollection &detElements = *detEleHandle.cptr();

    // The measurements of each seed, index-aligned with the track parameters
    ActsTrk::SeedMeasurementsContainer *seedMeasurements = nullptr;
    if (!m_outputSeedMeasurementsKey.empty()) {
      SG::WriteHandle<ActsTrk::SeedMeasurementsContainer> seedMeasurementsHandle = SG::makeHandle(m_outputSeedMeasurementsKey, ctx);
      ATH_CHECK(seedMeasurementsHandle.record(std::make_unique<ActsTrk::SeedMeasurementsContainer>()));
      seedMeasurements = seedMeasurementsHandle.ptr();
      seedMeasurements->reserve(seeds.size(), 3 * seeds.size());
    }
    std::vector<const xAOD::UncalibratedMeasurement*> measurements;
    std::vector<const xAOD::UncalibratedMeasurement*> duplicateDetectionMeasurements;
    const ITrackParamsEstimationTool::SpacePointIndicesFun_t spacePointIndicesFun = m_paramEstimationTool->spacePointIndicesFun();

    for (std::size_t iseed = 0; iseed < seeds.size(); ++iseed) {
      ActsTrk::Seed seed = seeds[iseed];

      if (seedMeasurements) {
        const auto& sps = seed.sp();
        measurements.clear();
        for (const xAOD::SpacePoint *sp : sps) {
          measurements.insert(measurements.end(), sp->measurements().begin(), sp->measurements().end());
        }
        if (sps.size() <= 3) {
          seedMeasurements->push_back(measurements);
        } else {
          // As in the track finding, only the measurements of the three space points
          // used for the parameter estimation are used to detect duplicate seeds
          duplicateDetectionMeasurements.clear();
          for (std::size_t isp : spacePointIndicesFun(sps, searchStartSp(seed))) {
            const xAOD::SpacePoint *sp = sps.at(isp);
            duplicateDetectionMeasurements.insert(duplicateDetectionMeasurements.end(), sp->measurements().begin(), sp->measurements().end());
          }
          seedMeasurements->push_back(measurements, duplicateDetectionMeasurements);
        }
      }

      auto retrieveSurfaceFunction = 
        [this, &detElements] (const xAOD::SpacePoint& sp, std::size_t measIndex) -> const Acts::Surface& { 
          const InDetDD::SiDetectorElement* element = detElements.getDetectorElement(
                sp.elementIdList().at(measIndex));
          const Trk::Surface& atlas_surface = element->surface();
          return *m_geometryConvTool->convertSurfaceToActs(atlas_surface);
        };

      const auto& [optTrackParams, estimationStatus] =
        m_paramEstimationTool->estimateTrackParameters(
						       seed,
						       searchStartSp(seed),
						       tgContext,
						       mfContext,
						       calContext,
						       retrieveSurfaceFunction);

      if (!optTrackParams.has_value()) {
        ATH_MSG_DEBUG("Failed to estimate track parameters for seed " << iseed);
        trackParams->push_back(std::unique_ptr<Acts::BoundTrackParameters>());
        continue;
      }

      trackParams->push_back(std::make_unique<Acts::BoundTrackParameters>(std::move(*optTrackParams)));
    }

    return StatusCode::SUCCESS;
  }

  std::size_t SeedsToTrackParamsAlg::searchStartSp(const ActsTrk::Seed& seed) const {
    const xAOD::SpacePoint* bottom_sp = seed.sp().front();
    const bool reverse = m_autoReverseSearch && (bottom_sp->radius() > m_useTopSpRZboundary.value()[0] ||
                                                 std::abs(bottom_sp->z()) > m_useTopSpRZboundary.value()[1]);
    return reverse ? seed.sp().size() - 1 : 0;
  }

}
