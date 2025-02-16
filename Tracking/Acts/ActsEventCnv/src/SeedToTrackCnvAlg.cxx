/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <vector>
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"
#include "Acts/Definitions/Algebra.hpp"
#include "xAODMeasurementBase/MeasurementDefs.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "StoreGate/WriteHandle.h"


#include "SeedToTrackCnvAlg.h"

namespace ActsTrk {

SeedToTrackCnvAlg::SeedToTrackCnvAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode SeedToTrackCnvAlg::initialize()
{
  ATH_CHECK(m_seedContainerKey.initialize());
  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
  ATH_CHECK(m_actsTrackParamsKey.initialize());
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_detectorElementToGeometryIdMapKey.initialize());

  return StatusCode::SUCCESS;
}


StatusCode SeedToTrackCnvAlg::execute(const EventContext& context) const
{
  ActsTrk::MutableTrackContainer tracksContainer;

  Acts::GeometryContext gctx = m_trackingGeometryTool->getGeometryContext(context).context();
  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_trackingGeometryTool->trackingGeometry();
  ATH_CHECK(trackingGeometry.get() != nullptr);

  ATH_MSG_DEBUG("Retrieving Seed Collection with key: " << m_seedContainerKey.key());
  ATH_MSG_DEBUG("Retrieving Track Parameter Estimation Collection with key: " << m_actsTrackParamsKey.key());
  SG::ReadHandle<ActsTrk::SeedContainer> seedsHandle = SG::makeHandle(m_seedContainerKey, context);
  SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> parameterHandle = SG::makeHandle(m_actsTrackParamsKey, context);

  SG::ReadCondHandle<ActsTrk::DetectorElementToActsGeometryIdMap>
     detectorElementToGeometryIdMap{m_detectorElementToGeometryIdMapKey, context};
  ATH_CHECK(detectorElementToGeometryIdMap.isValid());

  ATH_CHECK(seedsHandle.isValid());
  ATH_CHECK(parameterHandle.isValid());
  for (std::size_t seedIndex = 0 ;  seedIndex < seedsHandle->size() ;++seedIndex){
    const ActsTrk::Seed* seedPointer = seedsHandle->at(seedIndex);
    const Acts::BoundTrackParameters* paramsPointer = parameterHandle->at(seedIndex);

    auto actsTrack =  tracksContainer.makeTrack();
    ActsTrk::MutableMultiTrajectory& trackStateContainer = tracksContainer.trackStateContainer();

    actsTrack.parameters() = paramsPointer->parameters();
    actsTrack.covariance() = (*paramsPointer->covariance());
    actsTrack.setReferenceSurface(paramsPointer->referenceSurface().getSharedPtr());
    size_t tsosPreviousIndex = Acts::MultiTrajectoryTraits::kInvalid;
    for (const xAOD::SpacePoint_v1* spacepoint: seedPointer->sp()) {
      const auto& measurements = spacepoint->measurements();
      for (const xAOD::UncalibratedMeasurement *umeas : measurements) {
            ActsTrk::ATLASUncalibSourceLink el(makeATLASUncalibSourceLink(umeas));
            const Acts::Surface *surf = ActsTrk::getSurfaceOfMeasurement(*trackingGeometry,**detectorElementToGeometryIdMap,*umeas);
            ATH_CHECK( surf && surf->getSharedPtr().get() != nullptr);
            auto actsTSOS = trackStateContainer.getTrackState(trackStateContainer.addTrackState(Acts::TrackStatePropMask::None, tsosPreviousIndex));
            actsTSOS.setReferenceSurface(surf->getSharedPtr());
            actsTSOS.setUncalibratedSourceLink(Acts::SourceLink(el));
            actsTrack.tipIndex() = actsTSOS.index();
            tsosPreviousIndex = actsTrack.tipIndex();
      }
    }
  } 
  
  std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer = m_tracksBackendHandlesHelper.moveToConst(std::move(tracksContainer), 
  m_trackingGeometryTool->getGeometryContext(context).context(), context);

  SG::WriteHandle<ActsTrk::TrackContainer> trackContainerHandle = SG::makeHandle(m_trackContainerKey, context);
  ATH_MSG_DEBUG("Tracks Container `" << m_trackContainerKey.key() << "` created ...");
  ATH_MSG_DEBUG("Created container with size: " << constTracksContainer->size());
  ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));
  if (!trackContainerHandle.isValid())
    {
      ATH_MSG_FATAL("Failed to write TrackContainer with key " << m_trackContainerKey.key());
      return StatusCode::FAILURE;
    }
  return StatusCode::SUCCESS;
}

} // namespace ActsTrk

