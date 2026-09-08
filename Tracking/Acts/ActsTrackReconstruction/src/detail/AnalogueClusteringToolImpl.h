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
#include "PixelClusterCalibrationToolBase.h"

namespace ActsTrk::detail {

   template <typename calib_data_t, typename traj_t>
   class AnalogueClusteringCalibrator;

   /// @brief Options for the analogue clustering calibrator
   template <typename calib_data_t>
   struct AnalogueClusteringCalibratorOptions {
        using error_data_t = typename std::remove_pointer_t<decltype(std::declval<calib_data_t>().getClusterErrorData())>;
        const error_data_t *m_errorData;
        double m_calibratedCovarianceLowerBound;
        int m_errorStrategy;
        bool m_correctCovariance;
   };

   /// @brief the Analogue clustering calibrator
  template <typename calib_data_t, typename traj_t>
  class AnalogueClusteringCalibrator
     : public PixelClusterCalibratorBase<AnalogueClusteringCalibrator<calib_data_t,traj_t>, traj_t>
  {
  public:
     using BASE=PixelClusterCalibratorBase<AnalogueClusteringCalibrator<calib_data_t,traj_t>, traj_t>;
     friend BASE;
     using Options = AnalogueClusteringCalibratorOptions<calib_data_t>;

     using error_data_t = AnalogueClusteringCalibratorOptions<calib_data_t>::error_data_t;
     AnalogueClusteringCalibrator(PixelClusterCalibratorOptionsBase &&base_options,
                                  Options &&options)
        : BASE(std::move(base_options)),
          m_options(std::move(options))
     {}

     const error_data_t* getErrorData() const {
        return m_options.m_errorData;
     }

     std::tuple<typename AnalogueClusteringCalibrator<calib_data_t, traj_t>::BASE::Pos,
                typename AnalogueClusteringCalibrator<calib_data_t, traj_t>::BASE::Cov,
                unsigned int>
     calibrate(const EventContext& ctx,
               const Acts::GeometryContext& gctx,
               const Acts::CalibrationContext& cctx,
               const xAOD::PixelCluster& cluster,
               const InDetDD::SiDetectorElement& detElement,
               const std::pair<float, float>& angles) const;

   protected:
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

     AnalogueClusteringCalibratorOptions<calib_data_t> m_options;
  };

   /// @brief the tool to create the analogue clustering calibrator.
  template <typename calib_data_t, typename traj_t>
  class AnalogueClusteringToolImpl
     : public PixelClusterCalibrationToolBase<traj_t> {
  public:
    using BASE = PixelClusterCalibrationToolBase<traj_t>;
    using BASE::BASE;
    
    virtual StatusCode initialize() override;
    
    virtual std::unique_ptr<PixelOnBoundStateCalibratorBase >  create(const EventContext &ctx) const override {
       return createOnTrackCalibrator(ctx);
    }

    virtual std::unique_ptr<PixelOnTrackCalibratorBase<traj_t> > createOnTrackCalibrator(const EventContext &ctx) const override {
       return std::make_unique<AnalogueClusteringCalibrator<calib_data_t,traj_t> >(this->createBaseOptions(ctx),
                                                                                   this->createOptions(ctx));
    }

  protected:
    const typename AnalogueClusteringCalibrator<calib_data_t,traj_t>::error_data_t* getErrorData(const EventContext &ctx) const;

    AnalogueClusteringCalibratorOptions<calib_data_t> createOptions(const EventContext &ctx) const {
       // @TODO cannot use designators because there is a method which creates the options for the base class
       AnalogueClusteringCalibratorOptions<calib_data_t> options{
          .m_errorData=getErrorData(ctx),
          .m_calibratedCovarianceLowerBound=m_calibratedCovarianceLowerBound,
          .m_errorStrategy=m_errorStrategy,
          .m_correctCovariance=m_correctCovariance};
       return options;
    }

    SG::ReadCondHandleKey<calib_data_t> m_clusterErrorKey {this, "PixelOfflineCalibData", "ITkPixelOfflineCalibData",
      "Calibration data for pixel clusters"
    };

    // in micrometers
    Gaudi::Property<bool> m_correctCovariance{this, "PerformCovarianceCalibration", true};
    Gaudi::Property<double> m_calibratedCovarianceLowerBound {this, "CalibratedCovarianceLowerBound", 0.};

    Gaudi::Property<int> m_errorStrategy {this, "errorStrategy", 1, "Which error strategy to use for clusters on track: 0 - calibrated, 1 - cluster pitch, to be used only if broadClusters is used during clustering"}; 
  };
  
} // namespace ActsTrk::detail

#include "src/detail/AnalogueClusteringToolImpl.icc"

#endif
