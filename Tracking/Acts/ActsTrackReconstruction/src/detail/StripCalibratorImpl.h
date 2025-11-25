/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_STRIPCALIBRATOR_IMPL_H
#define ACTSTRACKRECONSTRUCTION_STRIPCALIBRATOR_IMPL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"
#include "src/detail/OnTrackCalibrator.h"


namespace ActsTrk::detail {

  template <typename traj_t>
    class StripCalibratorImpl
    : public extends<AthAlgTool, IOnTrackCalibratorTool<traj_t>> {
  public:
    using base_class = typename extends<AthAlgTool, IOnTrackCalibratorTool<traj_t>>::base_class;
    using Pos = typename OnTrackCalibrator<traj_t>::StripPos;
    using Cov = typename OnTrackCalibrator<traj_t>::StripCov;
    using TrackStateProxy = typename OnTrackCalibrator<traj_t>::TrackStateProxy;

    StripCalibratorImpl(const std::string& type,
                        const std::string& name,
                        const IInterface* parent);

    virtual StatusCode initialize() override;

    std::pair<Pos, Cov> calibrate(const Acts::GeometryContext&,
                                  const Acts::CalibrationContext&,
                                  const xAOD::StripCluster&,
                                  const TrackStateProxy&) const;

    std::pair<Pos, Cov> calibrate(const Acts::GeometryContext&,
                                  const Acts::CalibrationContext&,
                                  const xAOD::StripCluster&,
                                  const Acts::BoundTrackParameters&) const;

    virtual void connect(OnTrackCalibrator<traj_t>& calibrator) const override;

    virtual void connectStripCalibrator(IOnBoundStateCalibratorTool::StripCalibrator& calibrator) const override;

    virtual bool calibrateAfterMeasurementSelection() const override;

  private:

    const InDetDD::SiDetectorElement& getDetectorElement(xAOD::DetectorIDHashType id) const;
    
    std::pair<typename StripCalibratorImpl<traj_t>::Pos,
              typename StripCalibratorImpl<traj_t>::Cov>
    calibrate(const Acts::GeometryContext&,
              const Acts::CalibrationContext&,
              const xAOD::StripCluster&,
              const InDetDD::SiDetectorElement&) const;

    std::optional<float> getCorrectedError(const xAOD::StripCluster& cluster) const;

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_stripDetEleCollKey {this, "DetEleCollKey", "",
      "Key of SiDetectorElementCollection for Strip"
    };

    Gaudi::Property<bool> m_postCalibration{this, "CalibrateAfterMeasurementSelection", false};
    Gaudi::Property<bool> m_correctCovariance{this, "PerformCovarianceCalibration", true};
    Gaudi::Property<int> m_errorStrategy {this,"errorStrategy", 0, "Which error strategy to use for clusters on track: 0 - no correction, 1 - cluster size, 2 - from clustering tool"};

  };
  
} // namespace ActsTrk::detail

#include "src/detail/StripCalibratorImpl.icc"

#endif

