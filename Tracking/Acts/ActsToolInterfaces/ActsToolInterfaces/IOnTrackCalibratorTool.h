/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IONTRACKCALIBRATORTOOL_H
#define ACTSTOOLINTERFACES_IONTRACKCALIBRATORTOOL_H

#include "IOnBoundStateCalibratorTool.h"

#include <tuple>

namespace ActsTrk {

  // @TODO remove traj_t template argument ?
  template <typename cluster_t, std::size_t DIM, typename traj_t>
  class OnTrackCalibratorBase : public OnBoundStateCalibratorBase<cluster_t,DIM> {
  public:
     using BASE=OnBoundStateCalibratorBase<cluster_t,DIM>;
     using BASE::BASE;

     using Pos = xAOD::MeasVector<DIM>;
     using Cov = xAOD::MeasMatrix<DIM>;
     using TrackStateProxy = typename Acts::MultiTrajectory<traj_t>::TrackStateProxy;

     using OnTrackCalibrator = Acts::Delegate<
        void(const Acts::GeometryContext&,
             const Acts::CalibrationContext&,
             const cluster_t &,
             TrackStateProxy &)>;

     using BASE::connectCalibrator;
     virtual void connectOnTrackCalibrator(OnTrackCalibrator &calibrator) const = 0;

  };

  template <typename cluster_t, std::size_t DIM, typename traj_t>
  class IOnTrackCalibratorTool : virtual public ActsTrk::traits::Calibrator<cluster_t,DIM>::ToolInterface {
  public:
     virtual std::unique_ptr<OnTrackCalibratorBase<cluster_t, DIM, traj_t> > createOnTrackCalibrator(const EventContext &ctx) const = 0;
  };

} // namespace ActsTrk

#endif
