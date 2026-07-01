/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_STRIPCALIBRATOR_IMPL_H
#define ACTSTRACKRECONSTRUCTION_STRIPCALIBRATOR_IMPL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "ActsToolInterfaces/IStripOnTrackCalibratorTool.h"

namespace ActsTrk::detail {

  template <typename traj_t>
  class StripCalibrator :  public StripOnTrackCalibratorBase<traj_t>
  {
  public:
     using BASE=StripOnTrackCalibratorBase<traj_t>;

     struct Options {
        int m_errorStrategy;
        bool m_correctCovariance;
     };
     StripCalibrator(const Options &cfg)
        : m_options(cfg)
     {}
     using Pos = xAOD::MeasVector<1>;
     using Cov = xAOD::MeasMatrix<1>;
     using TrackStateProxy=BASE::TrackStateProxy;

     void calibrate(const Acts::GeometryContext&,
                    const Acts::CalibrationContext&,
                    const xAOD::StripCluster&,
                    TrackStateProxy&) const;

     std::tuple<Pos, Cov, unsigned int> calibrate(const Acts::GeometryContext&,
                                                  const Acts::CalibrationContext&,
                                                  const Acts::Surface&,
                                                  const xAOD::StripCluster&,
                                                  const Acts::BoundTrackParameters&) const;

     virtual void connectOnTrackCalibrator(BASE::OnTrackCalibrator &calibrator) const override;
     virtual void connectCalibrator(StripOnBoundStateCalibratorBase::Calibrator &calibrator) const override;
  protected:
    const InDetDD::SiDetectorElement& getDetectorElement(const Acts::Surface &surface) const;

    std::tuple<Pos,
               Cov,
               unsigned int>
    calibrate(const Acts::GeometryContext&,
              const Acts::CalibrationContext&,
              const xAOD::StripCluster&) const;

    std::optional<float> getCorrectedError(const xAOD::StripCluster& cluster) const;

    Options m_options;
  };

  template <typename traj_t>
    class StripCalibratorToolImpl
    : public extends<AthAlgTool, IStripOnTrackCalibratorTool<traj_t> > {
  public:
    using base_class = typename extends<AthAlgTool, IStripOnTrackCalibratorTool<traj_t>>::base_class;

    StripCalibratorToolImpl(const std::string& type,
                        const std::string& name,
                        const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual std::unique_ptr<StripOnBoundStateCalibratorBase > create(const EventContext &ctx) const override {
       return createOnTrackCalibrator(ctx);
    }
    virtual std::unique_ptr<StripOnTrackCalibratorBase<traj_t> > createOnTrackCalibrator(const EventContext &ctx) const override final {
       return std::make_unique<StripCalibrator<traj_t> >(createOptions(ctx));
    }

    virtual bool calibrateAfterMeasurementSelection() const override;

  private:
    typename StripCalibrator<traj_t>::Options createOptions(const EventContext &) const {
       return typename StripCalibrator<traj_t>::Options{
          .m_errorStrategy=m_errorStrategy,
          .m_correctCovariance=m_correctCovariance
       };
    }

    Gaudi::Property<bool> m_postCalibration{this, "CalibrateAfterMeasurementSelection", false};
    Gaudi::Property<bool> m_correctCovariance{this, "PerformCovarianceCalibration", true};
    Gaudi::Property<int> m_errorStrategy {this,"errorStrategy", 0, "Which error strategy to use for clusters on track: 0 - no correction, 1 - cluster size, 2 - from clustering tool"};

  };
  
} // namespace ActsTrk::detail

#include "src/detail/StripCalibratorToolImpl.icc"

#endif

