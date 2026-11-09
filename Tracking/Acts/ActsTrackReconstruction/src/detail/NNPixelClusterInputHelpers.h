/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRACKRECONSTRUCTION_NNPIXELCLUSTERINPUTHELPERS_H
#define ACTSTRACKRECONSTRUCTION_NNPIXELCLUSTERINPUTHELPERS_H

#include <onnxruntime_cxx_api.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <tuple>
#include <vector>

#include "Acts/Utilities/Zip.hpp"
#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "src/detail/NNPixelClusterCalibratorHelpers.h"
#include "xAODInDetMeasurement/PixelCluster.h"

/// Helpers to fill the NN input from a pixel cluster and to run the number
/// network, outside the NN cluster calibrator so that other components can
/// use them.
namespace ActsTrk::detail::NNPixelCluster {

/// @brief returns spans of the cluster
/// @return xmin, xmax, ymin, ymax
inline std::tuple<int, int, int, int> clusterIndexRanges(
    const PixelID& pixelID, const xAOD::PixelCluster& cluster) {
  int xMin = std::numeric_limits<int>::max();
  int xMax = std::numeric_limits<int>::min();
  int yMin = std::numeric_limits<int>::max();
  int yMax = std::numeric_limits<int>::min();
  for (const auto& rdoID : cluster.rdoList()) {
    const Identifier singlePixelID(rdoID);
    const int x = static_cast<int>(pixelID.phi_index(singlePixelID));
    const int y = static_cast<int>(pixelID.eta_index(singlePixelID));
    xMin = std::min(x, xMin);
    xMax = std::max(x, xMax);
    yMin = std::min(y, yMin);
    yMax = std::max(y, yMax);
  }
  return {xMin, xMax, yMin, yMax};
}

/// @brief Calculated charge weighted cluster position
/// @return cell that is the center of the cluster
inline InDetDD::SiCellId weightedClusterCenter(
    const PixelID& pixelID, const xAOD::PixelCluster& cluster,
    const InDetDD::PixelModuleDesign* design) {

  // obtain weighted position
  double sum = 0;
  InDetDD::SiLocalPosition position(0, 0);
  auto idvals = Acts::zip(cluster.rdoList(), cluster.chargeList());
  for (const auto [rdoID, charge] : idvals) {
    const Identifier singlePixelID(rdoID);
    const int x = static_cast<int>(pixelID.phi_index(singlePixelID));
    const int y = static_cast<int>(pixelID.eta_index(singlePixelID));

    InDetDD::SiLocalPosition siLocalPosition(
        design->positionFromColumnRow(y, x));
    position += siLocalPosition * charge;
    sum += charge;
  }
  position /= sum;

  return design->cellIdOfPosition(position);
}

/// @brief Fills payload for NN from cluster (charges, pitches, layer),
/// the incidence angles have to be set by the caller.
inline void fillClusterData(const PixelID& pixelID, NNinput& nn,
                            const xAOD::PixelCluster& cluster,
                            const InDetDD::SiDetectorElement& detElement) {

  const Identifier moduleID(
      static_cast<Identifier::value_type>(cluster.identifier()));
  nn.set(NNinput::Index::layer, (float)pixelID.layer_disk(moduleID));
  nn.set(NNinput::Index::bec, (float)pixelID.barrel_ec(moduleID));

  // need to decide how to center the cluster in 7 x 7 matrix
  // this implementation will center it w/o taking into account the charge
  // (will be revisited)
  // reminder: row <-> phi <-> x, column <-> eta <-> y
  auto [xMin, xMax, yMin, yMax] = clusterIndexRanges(pixelID, cluster);

  // this is a particular way of truncating cluster to 7x7
  // TODO test if picking the center would work better for large clusters
  xMax = std::clamp(xMax, xMin,
                    xMin + static_cast<int>(NNinput::Index::windowSize));
  yMax = std::clamp(yMax, yMin,
                    yMin + static_cast<int>(NNinput::Index::windowSize));

  const InDetDD::PixelModuleDesign* design(
      static_cast<const InDetDD::PixelModuleDesign*>(&detElement.design()));

  nn.centerCell = weightedClusterCenter(pixelID, cluster, design);
  nn.centerPosition = design->localPositionOfCell(nn.centerCell);
  const int xCenter = nn.centerCell.phiIndex();
  const int yCenter = nn.centerCell.etaIndex();

  auto idvals = Acts::zip(cluster.rdoList(), cluster.chargeList());
  for (const auto [rdoID, charge] : idvals) {
    const Identifier singlePixelID(rdoID);
    const int x = static_cast<int>(pixelID.phi_index(singlePixelID));
    const int y = static_cast<int>(pixelID.eta_index(singlePixelID));

    const int xNN = NNinput::toNNinputIndex(x, xCenter);
    const int yNN = NNinput::toNNinputIndex(y, yCenter);

    if (NNinput::inWindow(xNN, yNN)) {
      nn.setPixelCharge(xNN, yNN, charge);
    } else {
      // this happens, we need to understand frequency and impact
    }
  }

  for (int i = 0; i < static_cast<int>(NNinput::Index::windowSize); ++i) {
    nn.setPixelXPitch(i, 0.1);
    nn.setPixelYPitch(i, 0.1);
    const int x = NNinput::toModuleIndex(i, xCenter);
    const int y = NNinput::toModuleIndex(i, yCenter);
    const InDetDD::SiCellId cell(x, y);
    if (cell.isValid())
      nn.setPixelXPitch(i, design->parameters(cell).width().xPhi());
  }
}

/// @brief obtain probabilities of number of particles using numbers NN
/// @return array with probabilities for 1, 2 and 3 or more particles
inline std::array<float, 3> predictNumberOfClusters(Ort::Session& session,
                                                    NNinput& nn) {
  auto inputTypeInfo = session.GetInputTypeInfo(0);
  auto tensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();
  const int64_t expectedDim = tensorInfo.GetShape()[1];

  // TODO investigate streamlining this
  // this is copy paste from NnClusterizationFactory
  // seems to be a lot of preps to be repeated for each invocation
  // maybe it can be spared?
  Ort::MemoryInfo memInfo =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  std::vector<int64_t> inputShape = {1, expectedDim};
  Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
      memInfo, nn.payload().data(), nn.payload().size(), inputShape.data(),
      inputShape.size());

  Ort::AllocatorWithDefaultOptions allocator;
  auto inputName = session.GetInputNameAllocated(0, allocator);
  auto outputName = session.GetOutputNameAllocated(0, allocator);
  std::array<const char*, 1> inputNames({inputName.get()});
  std::array<const char*, 1> outputNames({outputName.get()});

  // Run inference
  auto outputTensors = session.Run(Ort::RunOptions{nullptr}, inputNames.data(),
                                   &inputTensor, 1, outputNames.data(), 1);

  // Extract output
  const float* outputData = outputTensors[0].GetTensorData<float>();
  return std::array<float, 3>({*outputData, *(outputData+1),  *(outputData+2)});
}

}  // namespace ActsTrk::detail::NNPixelCluster

#endif
