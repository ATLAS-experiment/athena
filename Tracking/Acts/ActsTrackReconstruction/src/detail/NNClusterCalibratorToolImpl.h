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
#include "SiClusterizationTool/OnnxNNCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "src/detail/NNPixelClusterCalibratorHelpers.h"

namespace ActsTrk::detail {

template <typename calib_data_t, typename traj_t>
class NNClusterCalibrator;

/// @brief Options for the NN clustering calibrator that is kind of Analogue
/// calibrator
template <typename calib_data_t, typename traj_t>
struct NNClusterCalibratorOptions {
  int m_minClusterSizeForNN;  ///! minimum number of hits to run NN
  const OnnxNNCollection* m_models =
      nullptr;  ///! set of models for inference (non owning pointer)
  std::unique_ptr<AnalogueClusteringCalibrator<calib_data_t, traj_t>>
      m_fallback;
  bool isValid() const { return m_models != nullptr and m_fallback; }
};

/// @brief the NN clustering calibrator
template <typename calib_data_t, typename traj_t>
class NNClusterCalibrator
    : public PixelClusterCalibratorBase<
          NNClusterCalibrator<calib_data_t, traj_t>, traj_t> {
 public:
  using BASE =
      PixelClusterCalibratorBase<NNClusterCalibrator<calib_data_t, traj_t>,
                                 traj_t>;
  friend BASE;
  // using BaseOptions = BASE::Options;
  using Options = NNClusterCalibratorOptions<calib_data_t, traj_t>;

  NNClusterCalibrator(PixelClusterCalibratorOptionsBase&& base_options,
                      Options&& options)
      : BASE(std::move(base_options)), m_options(std::move(options)) {}

 protected:
  std::tuple<typename NNClusterCalibrator<calib_data_t, traj_t>::BASE::Pos,
             typename NNClusterCalibrator<calib_data_t, traj_t>::BASE::Cov,
             unsigned int>
  calibrate(const EventContext& ctx, const Acts::GeometryContext& gctx,
            const Acts::CalibrationContext& cctx,
            const xAOD::PixelCluster& cluster,
            const InDetDD::SiDetectorElement& detElement,
            const std::pair<float, float>& angles) const;

 private:
  /// @brief returns spans of the cluster
  /// @param cluster pixel
  /// @return xmin, xmax, ymin, ymax
  std::tuple<int, int, int, int> clusterIndexRanges(
      const xAOD::PixelCluster& cluster) const;

  /// @brief Calculated charge weighted cluster position
  /// @param cluster pixel cluster to process
  /// @param pixelID decoder decoder
  /// @param design is geometry of the module
  /// @return call that that is the center of the cluster
  InDetDD::SiCellId weightedClusterCenter(
      const xAOD::PixelCluster& cluster,
      const InDetDD::PixelModuleDesign* design) const;

  /// @brief Fills payload for NN from cluster
  /// @param nn the NN input payload to fill
  /// @param cluster pixel cluster to process
  /// Presumably, this function should also take track state proxy (which
  /// may require this to become a template) and InDetDD::SiDetectorElement
  /// and overall would presumably fit better another place
  void fillClusterData(NNinput& nn, const xAOD::PixelCluster& cluster,
                       const InDetDD::SiDetectorElement& detElement) const;

  /// @brief obtain probabilities of number of particles using numbers NN
  /// @param  the network input
  /// @return vector of size 3 with probabilities
  std::vector<float> predictNumberOfClusters(NNinput& nn) const;

  /// @brief run position inference
  PositionNNoutput predictPositions(NNinput& nn, int number) const;

  /// @brief returns network appropriate for the number of sub-clusters
  Ort::Session& selectPositionNetwork(int number) const;

  /// @brief scale
  typename NNClusterCalibrator<calib_data_t, traj_t>::BASE::Pos computeOutputPostion(const PositionNNoutput& positions, int bestIndex, const NNinput& input) const;
  typename NNClusterCalibrator<calib_data_t, traj_t>::BASE::Cov computeOutputCovariance(const PositionNNoutput& positions, int bestIndex, const NNinput& input) const;

  NNClusterCalibratorOptions<calib_data_t, traj_t> m_options;
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
        BASE::createBaseOptions(ctx), createOptions(ctx));
  }

 private:

  /**
   * @brief Create a Options object
   * This is needed to configure the per-event calibrator instances
   * @return NNClusterCalibratorOptions
   */
  NNClusterCalibratorOptions<calib_data_t, traj_t> createOptions(
      const EventContext& ctx) const;

  Gaudi::Property<int> m_minClusterSizeForNN{
      this, "minClusterSizeForNN", 0,
      "how big the cluster needs to be to apply NN to it"};

  SG::ReadCondHandleKey<OnnxNNCollection> m_readKeyONNX {
      this, "NnCollectionONNXReadKey", "PixelClusterNNONNX",
      "The conditions key for ONNX-based pixel cluster NNs"};

  // to bo added here is tool to interface with NN inference
};

}  // namespace ActsTrk::detail

#include "src/detail/NNClusterCalibratorToolImpl.icc"

#endif  // ACTSTRACKRECONSTRUCTION_NNCLUSTERINGTOOLIMPL_H
