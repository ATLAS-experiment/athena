/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_COREPIXELSPACEPOINTFORMATIONTOOL_H
#define ACTSTRK_DATAPREPARATION_COREPIXELSPACEPOINTFORMATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IPixelSpacePointFormationTool.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

#include "AthenaKernel/SlotSpecificObj.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/EventContext.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

#include <limits>
#include <string>

namespace ActsTrk {

  /// @class CorePixelSpacePointFormationTool
  /// Pixel space point formation using Acts::PixelSpacePointBuilder for the
  /// space point covariance. The position is still the cluster global position.
  ///
  /// Not equivalent to PixelSpacePointFormationTool: ACTS projects the cluster
  /// local covariance to (z, r) with no inflation by the cluster width.

  class CorePixelSpacePointFormationTool : public extends<AthAlgTool, ActsTrk::IPixelSpacePointFormationTool> {
  public:
    using base_class::base_class;
    virtual ~CorePixelSpacePointFormationTool() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode producePixelSpacePoint(const EventContext& ctx,
					      const Acts::GeometryContext& gctx,
					      const xAOD::PixelCluster& cluster,
					      xAOD::SpacePoint& sp,
					      const InDetDD::SiDetectorElement& element) const override;

    virtual bool usesGeometryContext() const override { return true; }

  private:

    ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

    ActsTrk::detail::xAODUncalibMeasSurfAcc m_surfaceAccessor{};

    /// Reference frame of the last module seen, keyed on the identifier hash and
    /// stamped with the event so an alignment change cannot go stale.
    struct SurfaceCache {
      Acts::RotationMatrix3 rotLocalToGlobal{Acts::RotationMatrix3::Identity()};
      xAOD::DetectorIDHashType idHash{std::numeric_limits<xAOD::DetectorIDHashType>::max()};
      EventContext::ContextEvt_t evt{EventContext::INVALID_CONTEXT_EVT};
    };
    mutable SG::SlotSpecificObj<SurfaceCache> m_surfaceCache ATLAS_THREAD_SAFE;

    Gaudi::Property<bool> m_useSurfaceCache{this, "UseSurfaceCache", true,
      "Reuse the surface reference frame across the clusters of a module"};

  };

}

#endif
