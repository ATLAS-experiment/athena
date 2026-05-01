/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ANALOGUECLUSTERINGTOOL_IMPL_H
#define ACTSTRACKRECONSTRUCTION_ANALOGUECLUSTERINGTOOL_IMPL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelConditionsData/ITkPixelOfflineCalibData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

#include "ActsToolInterfaces/IPixelOnTrackCalibratorTool.h"

namespace ActsTrk::detail {

  template <typename calib_data_t, typename traj_t>
  class AnalogueClusteringCalibrator :  public PixelOnTrackCalibratorBase<traj_t>
  {
  public:
     using OnTrackCalibrator=OnTrackCalibratorBase<xAOD::PixelCluster,2,traj_t>::OnTrackCalibrator;
     using TrackStateProxy = OnTrackCalibratorBase<xAOD::PixelCluster,2,traj_t>::TrackStateProxy;
     using Calibrator=OnBoundStateCalibratorBase<xAOD::PixelCluster,2>::Calibrator;
     using error_data_t = typename std::remove_pointer_t<decltype(std::declval<calib_data_t>().getClusterErrorData())>;
     struct Options {
        const ISiLorentzAngleTool *m_lorentzAngleTool;
        const PixelID* m_pixelID;
        const error_data_t *m_errorData;
        double m_calibratedCovarianceLowerBound;
        int m_thickness;
        int m_errorStrategy;
        bool m_correctCovariance;
        bool m_useWeightedPos;
     };
     AnalogueClusteringCalibrator(const Options &cfg)
        : m_options(cfg)
     {}
     using Pos = xAOD::MeasVector<2>;
     using Cov = xAOD::MeasMatrix<2>;
     std::pair<Pos, Cov> calibrate(const Acts::GeometryContext&,
                                   const Acts::CalibrationContext&,
                                   const xAOD::PixelCluster&,
                                   const TrackStateProxy&) const;

     std::pair<Pos, Cov> calibrate(const Acts::GeometryContext&,
                                   const Acts::CalibrationContext&,
                                   const Acts::Surface&,
                                   const xAOD::PixelCluster&,
                                   const Acts::BoundTrackParameters&) const;

     virtual void connectOnTrackCalibrator(OnTrackCalibrator& calibrator) const override;
     virtual void connectCalibrator(Calibrator& calibrator) const override;

  protected:
     const error_data_t* getErrorData() const {
        return m_options.m_errorData;
     }
     const InDetDD::SiDetectorElement& getDetectorElement(const Acts::Surface &surface) const;

     std::pair<typename AnalogueClusteringCalibrator<calib_data_t, traj_t>::Pos,
               typename AnalogueClusteringCalibrator<calib_data_t, traj_t>::Cov>
     calibrate(const Acts::GeometryContext& gctx,
               const Acts::CalibrationContext& cctx,
               const xAOD::PixelCluster& cluster,
               const InDetDD::SiDetectorElement& detElement,
               const std::pair<float, float>& angles) const;

     std::pair<float, float>
     anglesOfIncidence(const EventContext& ctx,
                       const Acts::GeometryContext& gctx,
                       const Acts::Surface &surface,
                       const InDetDD::SiDetectorElement& element,
                       const Acts::Vector3& direction) const;

     std::pair<float, float> getCentroid(const EventContext& ctx,
                                         const xAOD::PixelCluster& cluster,
                                         const InDetDD::SiDetectorElement& element) const;

     std::pair<std::optional<float>, std::optional<float>>
     getCorrectedPosition(const EventContext& ctx,
                          const xAOD::PixelCluster& cluster,
                          const error_data_t& errorData,
                          const InDetDD::SiDetectorElement& element,
                          const std::pair<float, float>& angles) const;

     std::pair<std::optional<float>, std::optional<float>>
     getCorrectedError(const error_data_t& errorData,
                       const InDetDD::SiDetectorElement& element,
                       const std::pair<float, float>& angles,
                       const xAOD::PixelCluster& cluster) const;

  private:
     Options m_options;
  };

  template <typename calib_data_t, typename traj_t>
  class AnalogueClusteringToolImpl
     : public extends<AthAlgTool, IPixelOnTrackCalibratorTool<traj_t>> {
  public:
    using base_class = typename extends<AthAlgTool, IPixelOnTrackCalibratorTool<traj_t>>::base_class;
    
    AnalogueClusteringToolImpl(const std::string& type,
                               const std::string& name,
                               const IInterface* parent);
    
    virtual StatusCode initialize() override;
    
    virtual std::unique_ptr<PixelOnBoundStateCalibratorBase >  create(const EventContext &ctx) const override {
       return createOnTrackCalibrator(ctx);
    }
    virtual std::unique_ptr<PixelOnTrackCalibratorBase<traj_t> > createOnTrackCalibrator(const EventContext &ctx) const override final {
       return std::make_unique<AnalogueClusteringCalibrator<calib_data_t,traj_t> >(createOptions(ctx));
    }

    virtual bool calibrateAfterMeasurementSelection() const override;

  private:
    const typename AnalogueClusteringCalibrator<calib_data_t,traj_t>::error_data_t* getErrorData(const EventContext &ctx) const;

    typename AnalogueClusteringCalibrator<calib_data_t,traj_t>::Options createOptions(const EventContext &ctx) const {
       return typename AnalogueClusteringCalibrator<calib_data_t,traj_t>::Options{
          .m_lorentzAngleTool=&(*m_lorentzAngleTool),
          .m_pixelID=m_pixelID,
          .m_errorData=getErrorData(ctx),
          .m_calibratedCovarianceLowerBound=m_calibratedCovarianceLowerBound,
          .m_thickness=m_thickness,
          .m_errorStrategy=m_errorStrategy,
          .m_correctCovariance=m_correctCovariance,
          .m_useWeightedPos=m_useWeightedPos
       };
    }

    SG::ReadCondHandleKey<calib_data_t> m_clusterErrorKey {this, "PixelOfflineCalibData", "ITkPixelOfflineCalibData",
      "Calibration data for pixel clusters"
    };

    ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool {this, "PixelLorentzAngleTool", "",
      "Tool to retreive Lorentz angle"
    };

    // in micrometers
    Gaudi::Property<int> m_thickness {this, "PixelThickness", 250};
    Gaudi::Property<bool> m_postCalibration{this, "CalibrateAfterMeasurementSelection", false};
    Gaudi::Property<bool> m_correctCovariance{this, "PerformCovarianceCalibration", true};
    Gaudi::Property<double> m_calibratedCovarianceLowerBound {this, "CalibratedCovarianceLowerBound", 0.};
    Gaudi::Property<bool> m_useWeightedPos {this, "UseWeightedPosition", false}; // if pixel cluster use weighted local position

    Gaudi::Property<int> m_errorStrategy {this, "errorStrategy", 1, "Which error strategy to use for clusters on track: 0 - calibrated, 1 - cluster pitch, to be used only if broadClusters is used during clustering"}; 

    const PixelID *m_pixelID{};
  };
  
} // namespace ActsTrk::detail

#include "src/detail/AnalogueClusteringToolImpl.icc"

#endif
