/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"


#include "SeedToTrackCnvAlg.h"

namespace ActsTrk {

StatusCode SeedToTrackCnvAlg::initialize()
{
  ATH_CHECK(m_seedContainerKey.initialize());
  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
  ATH_CHECK(m_actsTrackParamsKey.initialize());
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  m_surfAcc = detail::xAODUncalibMeasSurfAcc{m_trackingGeometryTool.get()};
  if (m_seedContainerKey.size() != m_actsTrackParamsKey.size()) {
    ATH_MSG_ERROR("Seed and Parameter containers have different sizes: "
      << m_seedContainerKey.size() << " for seeds and "
      << m_actsTrackParamsKey.size() << " for the parameters");
    return StatusCode::FAILURE;
  }


  return StatusCode::SUCCESS;
}


StatusCode SeedToTrackCnvAlg::execute(const EventContext& context) const {
  Acts::VectorTrackContainer trackBackend;
  Acts::VectorMultiTrajectory trackStateBackend;
  ActsTrk::MutableTrackContainer tracksContainer( std::move(trackBackend),
                                                  std::move(trackStateBackend) );

  Acts::GeometryContext gctx = m_trackingGeometryTool->getGeometryContext(context).context();
  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_trackingGeometryTool->trackingGeometry();

  for (std::size_t i(0); i<m_seedContainerKey.size(); ++i) {
    ATH_MSG_DEBUG("Retrieving Seed Collection with key: " << m_seedContainerKey.at(i).key());
    ATH_MSG_DEBUG("Retrieving Track Parameter Estimation Collection with key: " << m_actsTrackParamsKey.at(i).key());
    SG::ReadHandle<ActsTrk::SeedContainer> seedsHandle = SG::makeHandle(m_seedContainerKey.at(i), context);
    SG::ReadHandle<ActsTrk::BoundTrackParametersContainer> parameterHandle = SG::makeHandle(m_actsTrackParamsKey.at(i), context);
    ATH_CHECK(seedsHandle.isValid());
    ATH_CHECK(parameterHandle.isValid());

    for (std::size_t seedIndex = 0 ;  seedIndex < seedsHandle->size() ;++seedIndex){
      ActsTrk::Seed seed = seedsHandle->at(seedIndex);
      const Acts::BoundTrackParameters* paramsPointer = parameterHandle->at(seedIndex);

      if (paramsPointer == nullptr) {
        ATH_MSG_DEBUG("Track Parameters is nullptr");
        continue;
      }
      
      auto actsTrack =  tracksContainer.makeTrack();
      auto& trackStateContainer = tracksContainer.trackStateContainer();
      
      actsTrack.parameters() = paramsPointer->parameters();
      actsTrack.covariance() = (*paramsPointer->covariance());
      actsTrack.setReferenceSurface(paramsPointer->referenceSurface().getSharedPtr());
      std::size_t tsosPreviousIndex = Acts::kTrackIndexInvalid;
      for (const xAOD::SpacePoint_v1* spacepoint: seed.sp()) {
          const auto& measurements = spacepoint->measurements();
          for (const xAOD::UncalibratedMeasurement *umeas : measurements) {
            const Acts::Surface *surf = m_surfAcc.get(umeas);
            assert(surf);
            auto actsTSOS = trackStateContainer.getTrackState(trackStateContainer.addTrackState(Acts::TrackStatePropMask::None, tsosPreviousIndex));
            actsTSOS.setReferenceSurface(surf->getSharedPtr());
            actsTSOS.setUncalibratedSourceLink(detail::xAODUncalibMeasCalibrator::pack(umeas));
            actsTrack.tipIndex() = actsTSOS.index();
            tsosPreviousIndex = actsTrack.tipIndex();
          }
      }
    } 

  }

  Acts::ConstVectorTrackContainer ctrackBackend( std::move(tracksContainer.container()) );
  Acts::ConstVectorMultiTrajectory ctrackStateBackend( std::move(tracksContainer.trackStateContainer()) );
  std::unique_ptr< ActsTrk::TrackContainer > ctracksContainer = std::make_unique< ActsTrk::TrackContainer >( std::move(ctrackBackend),
                                                                                                             std::move(ctrackStateBackend) );
  
  SG::WriteHandle<ActsTrk::TrackContainer> trackContainerHandle = SG::makeHandle(m_trackContainerKey, context);
  ATH_MSG_DEBUG("Tracks Container `" << m_trackContainerKey.key() << "` created ...");
  ATH_MSG_DEBUG("Created container with size: " << ctracksContainer->size());
  ATH_CHECK(trackContainerHandle.record(std::move(ctracksContainer)));
  if (!trackContainerHandle.isValid())
    {
      ATH_MSG_FATAL("Failed to write TrackContainer with key " << m_trackContainerKey.key());
      return StatusCode::FAILURE;
    }
  return StatusCode::SUCCESS;
}

} // namespace ActsTrk

