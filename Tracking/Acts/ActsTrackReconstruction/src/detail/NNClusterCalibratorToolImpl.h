/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_NNCLUSTERCALIBRATORTOOLIMPL_H
#define ACTSTRACKRECONSTRUCTION_NNCLUSTERCALIBRATORTOOLIMPL_H

#include "ActsToolInterfaces/IPixelOnTrackCalibratorTool.h"
#include "AnalogueClusteringToolImpl.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelClusterCalibrationToolBase.h"
#include "PixelConditionsData/ITkPixelOfflineCalibData.h"
#include "StoreGate/ReadCondHandleKey.h"

namespace ActsTrk::detail {

template <typename calib_data_t, typename traj_t>
class NNClusterCalibrator;

/// @brief Options for the NN clustering calibrator that is kind of Analogue
/// calibrator

struct NNClusterCalibratorOptions {
  int m_minClusterSizeForNN;  ///! minimum number of hits to run NN
};

/// @brief the NN clustering calibrator
template <typename calib_data_t, typename traj_t>
class NNClusterCalibrator
    : public AnalogueClusteringCalibrator<calib_data_t, traj_t> {
 public:
  using BASE = AnalogueClusteringCalibrator<calib_data_t, traj_t>;
  friend BASE;
  using error_data_t = BASE::error_data_t;
  using BaseOptions = BASE::Options;
  using Options = NNClusterCalibratorOptions;

  NNClusterCalibrator(PixelClusterCalibratorOptionsBase&& base_options,
                         BaseOptions&& direct_base_options, Options&& options)
      : BASE(std::move(base_options), std::move(direct_base_options)), 
        m_options(std::move(options)) {}

 protected:
  std::tuple<
      typename NNClusterCalibrator<calib_data_t, traj_t>::BASE::Pos,
      typename NNClusterCalibrator<calib_data_t, traj_t>::BASE::Cov, 
      unsigned int>
  calibrate(const EventContext& ctx,
            const Acts::GeometryContext& gctx,
            const Acts::CalibrationContext& cctx,
            const xAOD::PixelCluster& cluster,
            const InDetDD::SiDetectorElement& detElement,
            const std::pair<float, float>& angles) const;

  std::pair<std::optional<float>, std::optional<float>> getCorrectedPosition(
      const EventContext& ctx, 
      const xAOD::PixelCluster& cluster,
      const error_data_t& errorData, 
      const InDetDD::SiDetectorElement& element,
      const std::pair<float, float>& angles) const;

  std::pair<std::optional<float>, std::optional<float>> getCorrectedError(
      const error_data_t& errorData, 
      const InDetDD::SiDetectorElement& element,
      const std::pair<float, float>& angles,
      const xAOD::PixelCluster& cluster) const;

  NNClusterCalibratorOptions m_options;
};

/// @brief the tool to create NN calibrator.
template <typename calib_data_t, typename traj_t>
class NNClusterCalibratorToolImpl
    : public AnalogueClusteringToolImpl<calib_data_t, traj_t> {
 public:
  using BASE = AnalogueClusteringToolImpl<calib_data_t, traj_t>;
  using BASE::BASE;

  virtual StatusCode initialize() override;

  virtual std::unique_ptr<PixelOnBoundStateCalibratorBase> create(
      const EventContext& ctx) const override {
    return createOnTrackCalibrator(ctx);
  }
  /***
   * @brief produces configured calibrator object for this event
   * 
   * The structure is such because the calibrator is used many times per event
   * and it needs event context for IOV potentially
   */
  
  virtual std::unique_ptr<PixelOnTrackCalibratorBase<traj_t>>
  createOnTrackCalibrator(const EventContext& ctx) const override {
    return std::make_unique<NNClusterCalibrator<calib_data_t, traj_t>>(
        BASE::createBaseOptions(ctx), BASE::createOptions(ctx),
        createOptions(ctx));
  }

 private:
  const typename NNClusterCalibrator<calib_data_t, traj_t>::error_data_t*
  getErrorData(const EventContext& ctx) const;

  /**
   * @brief Create a Options object
   * This is needed to configure the per-event calibrator instances
   * @return NNClusterCalibratorOptions 
   */
  NNClusterCalibratorOptions createOptions(const EventContext&) const {
    return {
      .m_minClusterSizeForNN = m_minClusterSizeForNN
    };
  }

  Gaudi::Property<int> m_minClusterSizeForNN {this, "minClusterSizeForNN", 3, "how big the cluster needs to be to apply NN to it"}; 

  // to bo added here is tool to interface with NN inference
};

}  // namespace ActsTrk::detail

#include "src/detail/NNClusterCalibratorToolImpl.icc"

#endif  // ACTSTRACKRECONSTRUCTION_NNCLUSTERINGTOOLIMPL_H
