/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SeedsToTrackParamsAlg.h"

#include "TrkSurfaces/Surface.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"

namespace ActsTrk {

  SeedsToTrackParamsAlg::SeedsToTrackParamsAlg(const std::string& name, 
								   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) 
  {}

  StatusCode SeedsToTrackParamsAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name() << " ...");

    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_extrapolationTool.retrieve());
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    ATH_CHECK(m_paramEstimationTool.retrieve());

    ATH_CHECK(m_detEleCollKey.initialize());
    ATH_CHECK(m_inputSeedContainerKey.initialize());
    ATH_CHECK(m_outputTrackParamsCollectionKey.initialize());

    return AthReentrantAlgorithm::initialize();
  }

  StatusCode SeedsToTrackParamsAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name() << " ... ");

    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
    Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);

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

    for (std::size_t iseed = 0; iseed < seeds.size(); ++iseed) {
      const ActsTrk::Seed& seed = *seeds[iseed];

      bool useTopSp = false;

      auto retrieveSurfaceFunction = 
        [this, &detElements, useTopSp] (const ActsTrk::Seed& seed) -> const Acts::Surface& { 
          const xAOD::SpacePoint* sp = useTopSp ? seed.sp().back() : seed.sp().front();
          const InDetDD::SiDetectorElement* element = detElements.getDetectorElement(
                useTopSp ? sp->elementIdList().back() : sp->elementIdList().front());
          const Trk::Surface& atlas_surface = element->surface();
          return this->m_ATLASConverterTool->trkSurfaceToActsSurface(atlas_surface);
        };

      std::optional<Acts::BoundTrackParameters> optTrackParams =
        m_paramEstimationTool->estimateTrackParameters(ctx,
						       seed,
						       tgContext,
						       mfContext,
						       retrieveSurfaceFunction,
                   useTopSp);

      if (!optTrackParams.has_value()) {
        ATH_MSG_DEBUG("Failed to estimate track parameters for seed " << iseed);
        continue;
      }

      trackParams->push_back(std::make_unique<Acts::BoundTrackParameters>(std::move(*optTrackParams)));
    }

    return StatusCode::SUCCESS;
  }

}
