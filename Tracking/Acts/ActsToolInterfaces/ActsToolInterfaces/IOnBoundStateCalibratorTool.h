/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTOOLINTERFACES_IONBOUNDSTATECALIBRATORTOOL_H
#define ACTSTOOLINTERFACES_IONBOUNDSTATECALIBRATORTOOL_H

#include <GaudiKernel/IAlgTool.h>

#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"

namespace ActsTrk {

   class ClusterCalibratorBase {
   public:
      virtual ~ClusterCalibratorBase() {}
   };

   /// @brief Base class of a InDet calibrator object
   ///
   /// the calibrator will be used to produce the calibrated position and uncertainty of a cluster
   template <typename cluster_t, std::size_t DIM>
   class OnBoundStateCalibratorBase : public ClusterCalibratorBase {
   public:
      using ClusterType = cluster_t;
      static constexpr std::size_t ClusterDIM = DIM;
      using Pos = xAOD::MeasVector<DIM>;
      using Cov = xAOD::MeasMatrix<DIM>;
      using Calibrator = Acts::Delegate<
         std::pair<Pos, Cov>(const Acts::GeometryContext&,
                             const Acts::CalibrationContext&,
                             const Acts::Surface&,
                             const cluster_t &,
                             const Acts::BoundTrackParameters &)>;

      /// Connect this calibrator to the provided delegate.
      virtual void connectCalibrator(Calibrator &calibrator) const =0;
   };

   /// @brief interface of a tool to create a calibrator for a certain cluster type.
   template <typename cluster_t, std::size_t DIM>
   class IOnBoundStateCalibratorTool : virtual public IAlgTool {
   public:
      /// Create a calibrator object for the given event.
      virtual std::unique_ptr<OnBoundStateCalibratorBase<cluster_t, DIM>> create(const EventContext &ctx) const = 0;

      /// @return true if the calibration should only be applied after measurement selection e.g.
      ///     because it is already too slow to apply it already during measurement selection.
      virtual bool calibrateAfterMeasurementSelection() const =0;
   };

   namespace traits {
      /// Helper struct to get the correct types for a certain cluster
      template <typename T, std::size_t DIM>
      struct Calibrator {
         using ToolInterface = IOnBoundStateCalibratorTool<T,DIM>;
         // @TODO remove:
         //         using Calibrator = OnBoundStateCalibratorBase<T,DIM>;
      };
   }

} // namespace ActsTrk

#endif
