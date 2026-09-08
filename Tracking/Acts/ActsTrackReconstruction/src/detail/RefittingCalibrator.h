/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_REFITTINGCALIBRATOR_H
#define ACTSTRACKRECONSTRUCTION_REFITTINGCALIBRATOR_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "ActsEvent/TrackContainer.h"

#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "ActsCalibrators/TrkMeasurementCalibrator.h"
#include "ActsCalibrators/TrkPrepRawDataCalibrator.h"

#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibrators/TrkPrepRawDataSurfaceAcc.h"
#include "ActsCalibrators/TrkMeasSurfaceAccessor.h"

namespace ActsTrk::detail {
    /** @brief */
    class RefittingCalibrator {
      public:
        RefittingCalibrator(const ActsTrk::IGeometryRealmConvTool* convTool,
                            const Trk::IRIO_OnTrackCreator* rotCreator);
        
        using MutableTrackStateProxy = ActsTrk::MutableTrackStateBackend::TrackStateProxy;
        using ConstTrackStateProxy = ActsTrk::MutableTrackStateBackend::ConstTrackStateProxy;

        void calibrate(const Acts::GeometryContext& gctx,
                       const Acts::CalibrationContext& cctx,
                       const Acts::SourceLink& sourceLink,
                       MutableTrackStateProxy trackState) const;

        template <auto Callable, typename Type>
        /** @copydoc xAODUncalibMeasCalibrator::connect */
        void connect(const xAOD::UncalibMeasType type, const Type* instance) {
          m_xAODCalibrator.connect<Callable>(type, instance);
        }
      private:
        TrkPrepRawDataCalibrator m_prdCalibrator{};
        TrkMeasurementCalibrator m_measCalibrator{};
        xAODUncalibMeasCalibrator m_xAODCalibrator{};
       
    };

    class RefittingSurfaceAccesor {
        public:
            RefittingSurfaceAccesor(const IGeometryRealmConvTool* trkConvTool,
                                    const ActsTrk::ITrackingGeometrySvc* trackGeoSvc); 
             /** @brief Operator called by the Acts API to fetch the surface. */
            const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const;
          
        private:
          xAODUncalibMeasSurfAcc m_xAODAcc{};
          TrkPrepRawDataSurfaceAcc m_prdAcc{};
          TrkMeasSurfaceAccessor m_rotAcc{};

    };

}  // namespace ActsTrk::detail

#endif  // ACTSTRACKRECONSTRUCTION_REFITTINGCALIBRATOR_H
