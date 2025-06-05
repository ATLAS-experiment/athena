/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_TRACKCONTAINERHANDLESHELPER_H
#define ACTSTRKEVENT_TRACKCONTAINERHANDLESHELPER_H

#include <memory>

#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/TrackSummaryContainer.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "GaudiKernel/StatusCode.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTracking/TrackStateContainer.h"
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackMeasurementContainer.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackSurfaceContainer.h"
#include "xAODTracking/TrackSummaryContainer.h"

namespace ActsTrk {

/**
 * Parse TrackContainer name to get the prefix for backends
 * The name has to contain XYZTracks, the XYZ is returned
 */
std::string prefixFromTrackContainerName(const std::string& tracks);

class MutableTrackContainerHandlesHelper {
 public:
  
 /** @brief Constructor taking the pointer to the class holding the object used to declare the 
  *         data dependency from the WriteHandleKeys to the AvalancheScheduler. The object should be
  *         defined in the header like
  *             ActsTrk::MutableTrackContainerHandlesHelper m_trackHelper{this};
  */ 
 template <class PropOwner> 
      MutableTrackContainerHandlesHelper(PropOwner* owner);

  /**
   * Sets up the handles
   * @arg prefix - common prefix for all the names
   * the value would typically be taken from configurable string property
   */
  StatusCode initialize(const std::string& prefix);

  /**
   * produces ActsTrk::ConstTrackContainer with all backends stored in SG
   * @arg tc - MutableTrackContainer the source (will  be disassembled
   * after the operation)
   * @arg - geoContext - geometry context, needed in surfaces conversion
   * @arg evtContext - event context (needed for SG operations)
   */
  std::unique_ptr<ActsTrk::TrackContainer> moveToConst(
      ActsTrk::MutableTrackContainer&& tc,
      const Acts::GeometryContext& geoContext,
      const EventContext& evtContext) const;

 private:
  // MTJ part
  SG::WriteHandleKey<xAOD::TrackStateContainer> m_statesKey;
  SG::WriteHandleKey<xAOD::TrackParametersContainer> m_parametersKey;
  SG::WriteHandleKey<xAOD::TrackJacobianContainer> m_jacobiansKey;
  SG::WriteHandleKey<xAOD::TrackMeasurementContainer> m_measurementsKey;
  SG::WriteHandleKey<xAOD::TrackSurfaceContainer> m_surfacesKey;
  SG::WriteHandleKey<ActsTrk::MultiTrajectory> m_mtjKey;

  /**
   * helper to record MTJ
   */
  std::unique_ptr<ActsTrk::MultiTrajectory> moveToConst(
      ActsTrk::MutableMultiTrajectory&& mmtj,
      const EventContext& context) const;

  // TrackContainer part
  SG::WriteHandleKey<xAOD::TrackSummaryContainer> m_xAODTrackSummaryKey;
  SG::WriteHandleKey<xAOD::TrackSurfaceContainer> m_trackSurfacesKey;
  SG::WriteHandleKey<ActsTrk::TrackSummaryContainer> m_trackSummaryKey;
};

class ConstTrackContainerHandlesHelper {
 public:
  /**
   * Sets up the handles
   * @arg prefix - common prefix for all the names
   * the value would typically be taken from configurable string property
   */
  StatusCode initialize(const std::string& prefix);

   /** @brief Constructor taking the pointer to the class holding the object used to declare the 
  *         data dependency from the WriteHandleKeys to the AvalancheScheduler. The object should be
  *         defined in the header like
  *             ActsTrk::ConstTrackContainerHandlesHelper m_trackHelper{this};
  */ 
 template <class PropOwner> 
      ConstTrackContainerHandlesHelper(PropOwner* owner);

  std::unique_ptr<ActsTrk::TrackContainer> build(
      const Acts::TrackingGeometry* geo,
      const Acts::GeometryContext& geoContext,
      const EventContext& context) const;

 private:
  // restore the pointer to uncalibrated measurements from element links
  void restoreUncalibMeasurementPtr(xAOD::TrackStateAuxContainer &statesLink) const;
  // MTJ part
  SG::ReadHandleKey<xAOD::TrackStateContainer> m_statesKey;
  SG::ReadHandleKey<xAOD::TrackParametersContainer> m_parametersKey;
  SG::ReadHandleKey<xAOD::TrackJacobianContainer> m_jacobiansKey;
  SG::ReadHandleKey<xAOD::TrackMeasurementContainer> m_measurementsKey;
  SG::ReadHandleKey<xAOD::TrackSurfaceContainer> m_surfacesKey;

  SG::WriteHandleKey<ActsTrk::MultiTrajectory> m_mtjKey;
  // build for MTJ part
  std::unique_ptr<ActsTrk::MultiTrajectory> buildMtj(
      const Acts::TrackingGeometry* geo,
      const Acts::GeometryContext& geoContext,
      const EventContext& context) const;
  // TrackContainer part

  // track and its backend
  SG::ReadHandleKey<xAOD::TrackSummaryContainer> m_xAODTrackSummaryKey;
  SG::ReadHandleKey<xAOD::TrackSurfaceContainer> m_trackSurfacesKey;
  SG::WriteHandleKey<ActsTrk::TrackSummaryContainer> m_trackSummaryKey;
};

}  // namespace ActsTrk
#include "ActsEvent/TrackContainerHandlesHelper.ixx"
#endif
