/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CorePixelSpacePointFormationTool.h"

#include "Acts/SpacePointFormation/PixelSpacePointBuilder.hpp"
#include "Acts/Surfaces/Surface.hpp"

#include "GaudiKernel/ThreadLocalContext.h"

#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

#include <algorithm>

namespace ActsTrk {

    StatusCode CorePixelSpacePointFormationTool::initialize()
    {
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        m_surfaceAccessor = ActsTrk::detail::xAODUncalibMeasSurfAcc(m_trackingGeometrySvc.get());

        return StatusCode::SUCCESS;
    }

    StatusCode
    CorePixelSpacePointFormationTool::producePixelSpacePoint(const Acts::GeometryContext& gctx,
                                                             const xAOD::PixelCluster& cluster,
                                                             xAOD::SpacePoint& sp,
                                                             const InDetDD::SiDetectorElement& /*element*/) const
    {
      const xAOD::DetectorIDHashType idHash = cluster.identifierHash();
      const Acts::Vector3 globalPosition = cluster.globalPosition().cast<double>();
      const Acts::SquareMatrix2 localCov = cluster.localCovariance<2>().cast<double>();

      // Clusters come grouped by module: look up the surface only when the module changes.
      auto lookupReferenceFrame = [this, &gctx, &cluster, idHash](Acts::RotationMatrix3& rot) -> StatusCode {
        const Acts::Surface* surface = m_surfaceAccessor.get(&cluster);
        if (surface == nullptr) {
          ATH_MSG_FATAL("No ACTS surface for pixel cluster with identifier hash " << idHash);
          return StatusCode::FAILURE;
        }
        // Position and direction are unused by plane and disc surfaces
        rot = surface->referenceFrame(gctx, Acts::Vector3::Zero(), Acts::Vector3::Zero());
        return StatusCode::SUCCESS;
      };

      // Returned in the order (z, r)
      Acts::Vector2 variance;
      if (m_useSurfaceCache) {
        const EventContext& ctx = Gaudi::Hive::currentContext();
        SurfaceCache& cache = *m_surfaceCache.get(ctx);
        if (cache.idHash != idHash or cache.evt != ctx.evt()) {
          // Key updated only on success, so a failed look-up leaves no stale frame.
          ATH_CHECK( lookupReferenceFrame(cache.rotLocalToGlobal) );
          cache.idHash = idHash;
          cache.evt = ctx.evt();
        }
        variance = Acts::PixelSpacePointBuilder::computeCovarianceZR(cache.rotLocalToGlobal,
                                                                     globalPosition, localCov).diagonal();
      } else {
        Acts::RotationMatrix3 rotLocalToGlobal;
        ATH_CHECK( lookupReferenceFrame(rotLocalToGlobal) );
        variance = Acts::PixelSpacePointBuilder::computeCovarianceZR(rotLocalToGlobal,
                                                                     globalPosition, localCov).diagonal();
      }

      float cov_z = static_cast<float>(variance[0]);
      float cov_r = static_cast<float>(variance[1]);

      if (m_useMaxVariance) {
        cov_z = std::min(cov_z, m_maxVarianceZ.value());
        cov_r = std::min(cov_r, m_maxVarianceR.value());
      }

      sp.setSpacePoint(idHash,
                       cluster.globalPosition(),
                       cov_r,
                       cov_z,
                       std::vector< const xAOD::UncalibratedMeasurement* >({&cluster}));

      return StatusCode::SUCCESS;
    }
}
