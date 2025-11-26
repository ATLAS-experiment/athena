/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ANALOGUECLUSTERINGTOOL_IMPL_H
#define ACTSTRACKRECONSTRUCTION_ANALOGUECLUSTERINGTOOL_IMPL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "PixelConditionsData/ITkPixelOfflineCalibData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"
#include "src/detail/OnTrackCalibrator.h"

namespace ActsTrk::detail {

  template <typename calib_data_t, typename traj_t>
  class AnalogueClusteringToolImpl
    : public extends<AthAlgTool, IOnTrackCalibratorTool<traj_t>> {
  public:
    using base_class = typename extends<AthAlgTool, IOnTrackCalibratorTool<traj_t>>::base_class;
    using Pos = typename OnTrackCalibrator<traj_t>::PixelPos;
    using Cov = typename OnTrackCalibrator<traj_t>::PixelCov;
    using TrackStateProxy = typename OnTrackCalibrator<traj_t>::TrackStateProxy;
    
    AnalogueClusteringToolImpl(const std::string& type,
                               const std::string& name,
                               const IInterface* parent);
    
    virtual StatusCode initialize() override;
    
    std::pair<Pos, Cov> calibrate(const Acts::GeometryContext&,
                                  const Acts::CalibrationContext&,
                                  const xAOD::PixelCluster&,
                                  const TrackStateProxy&) const;
    
    std::pair<Pos, Cov> calibrate(const Acts::GeometryContext&,
                                  const Acts::CalibrationContext&,
                                  const xAOD::PixelCluster&,
                                  const Acts::BoundTrackParameters&) const;
    
    virtual void connect(OnTrackCalibrator<traj_t>& calibrator) const override;
    
    virtual void connectPixelCalibrator(IOnBoundStateCalibratorTool::PixelCalibrator& calibrator) const override;
    
    virtual bool calibrateAfterMeasurementSelection() const override;
    
  private:
    
    using error_data_t = typename std::remove_pointer_t<decltype(std::declval<calib_data_t>().getClusterErrorData())>;
    
    const InDetDD::SiDetectorElement& getDetectorElement(xAOD::DetectorIDHashType id) const;
    
    std::pair<typename AnalogueClusteringToolImpl<calib_data_t, traj_t>::Pos,
              typename AnalogueClusteringToolImpl<calib_data_t, traj_t>::Cov>
    calibrate(const Acts::GeometryContext& gctx,
              const Acts::CalibrationContext& cctx,
              const xAOD::PixelCluster& cluster,
              const InDetDD::SiDetectorElement& detElement,
              const std::pair<float, float>& angles) const;
    
    std::pair<float, float>
    anglesOfIncidence(const InDetDD::SiDetectorElement& element,
                      const Acts::Vector3& direction) const;
    
    
    std::pair<float, float> getCentroid(const xAOD::PixelCluster& cluster,
                                        const InDetDD::SiDetectorElement& element) const;
    
    const error_data_t* getErrorData() const;
    
    std::pair<std::optional<float>, std::optional<float>>
    getCorrectedPosition(const xAOD::PixelCluster& cluster,
                         const error_data_t& errorData,
                         const InDetDD::SiDetectorElement& element,
                         const std::pair<float, float>& angles) const;
    
    std::pair<std::optional<float>, std::optional<float>>
    getCorrectedError(const error_data_t& errorData,
                      const InDetDD::SiDetectorElement& element,
                      const std::pair<float, float>& angles,
                      const xAOD::PixelCluster& cluster) const;
    
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey {this, "DetEleCollKey", "",
      "Key of SiDetectorElementCollection for Pixel"
    };
    
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
    
    
    const PixelID* m_pixelid {nullptr};
  };
  
} // namespace ActsTrk::detail

#include "src/detail/AnalogueClusteringToolImpl.icc"

#endif
