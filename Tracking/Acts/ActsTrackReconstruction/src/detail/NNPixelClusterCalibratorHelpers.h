/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef NNPIXELCLUSTERCALIBRATORHELPERS_H
#define NNPIXELCLUSTERCALIBRATORHELPERS_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>

#include "ReadoutGeometryBase/SiCellId.h"
#include "ReadoutGeometryBase/SiLocalPosition.h"

namespace ActsTrk {
class NNinput {
 public:
  using payload_t = float;
  using index_t = int;
  enum class Index : index_t {
    chargeOffset = 0,
    windowSize = 7,
    center = (windowSize - 1) / 2,  // this should be index of center point
    halfSize = center,
    chargeEnd = windowSize * windowSize,
    pitchYOffset = chargeEnd,
    pitchXOffset = pitchYOffset + windowSize,
    layer = pitchXOffset + windowSize,
    bec = layer + 1,
    phi = bec + 1,
    theta = phi + 1,
    totalSize = theta + 1
  };
  NNinput() : m_payload(static_cast<index_t>(Index::totalSize), {}) {}

  std::vector<payload_t>& payload() { return m_payload; }

  const std::vector<payload_t>& payload() const { return m_payload; }

  float totCharge() const {
    return std::accumulate(
        std::begin(m_payload) + static_cast<index_t>(Index::chargeOffset),
        std::begin(m_payload) + static_cast<index_t>(Index::chargeEnd), 0.0f);
  }

  void setPixelCharge(index_t x, index_t y, payload_t charge) {
    checkRange(x, "x of coordinate of the charge");
    checkRange(y, "x of coordinate of the charge");

    m_payload[static_cast<index_t>(Index::chargeOffset) +
              x * static_cast<index_t>(Index::windowSize) + y] = charge;
  }

  void setPixelXPitch(index_t i, payload_t pitch) {
    checkRange(i, "x pitch");
    m_payload[static_cast<index_t>(Index::pitchXOffset) + i] = pitch;
  }

  payload_t getPixelXPitch(index_t i) const {
    return m_payload[static_cast<index_t>(Index::pitchXOffset) + i];
  }

  void setPixelYPitch(index_t i, payload_t pitch) {
    checkRange(i, "y pitch");
    m_payload[static_cast<index_t>(Index::pitchYOffset) + i] = pitch;
  }

  void set(Index i, payload_t v) { m_payload[static_cast<index_t>(i)] = v; }

  /// @brief Translate from module indices to NNinput indices
  /// e.g. if the cluster is of size 3 and indices of pixels are 23,24,25
  // they will become 2 3 4
  /// @param pixelIndex
  /// @param centerIndex
  /// @return
  static int toNNinputIndex(index_t pixelIndex, index_t centerIndex) {
    return pixelIndex - centerIndex +
           static_cast<NNinput::index_t>(Index::center);
  }
  /// @brief Reverse operation to toNNInputIndex
  /// @param nnIndex pixel index in NN repsentation
  /// @param centerIndex cluster center
  /// @return pixel index in module
  static int toModuleIndex(index_t nnIndex, index_t centerIndex) {
    return nnIndex + centerIndex - static_cast<NNinput::index_t>(Index::center);
  }

  static bool inWindow(index_t x, index_t y) {
    return x >= 0 && x < static_cast<index_t>(Index::windowSize) && y >= 0 &&
           y < static_cast<index_t>(Index::windowSize);
  }

  /// @brief translates position returned from the network to mm
  /// The position returned from the network is given in the "index" space
  /// while it needs to be be in mm.
  /// Simplifying, the returned position need to be multiplied by pitch
  /// however, due to uneven pitch size the translation is slightly trickier.
  /// The code is reverse engineered from: https://pixsplit.docs.cern.ch/export/
  /// and associated python implementation
  /// @param coord - position in nn coordinates
  /// @param pitchOffset - either pitchXOffset or pitchYOffset - place where
  /// pitches vectors are stored
  /// @return position shift in mm
  float indexCoordToRealCoord(float coord, Index pitchOffset) {
    std::span<payload_t, static_cast<index_t>(Index::windowSize)> pitches(
        m_payload.begin() + static_cast<index_t>(pitchOffset),
        m_payload.begin() + static_cast<index_t>(pitchOffset) +
            static_cast<index_t>(Index::windowSize));
    const std::size_t windowSize = static_cast<std::size_t>(Index::windowSize);

    std::array<double, static_cast<std::size_t>(Index::windowSize)>
        cumulative{};
    double acc = 0.0;
    for (std::size_t j = 0; j < windowSize; ++j) {
      acc += pitches[j];
      cumulative[j] = acc;
    }

    const std::size_t centerIdx = static_cast<std::size_t>(Index::center);
    const double centerValue = cumulative[centerIdx];
    for (double& value : cumulative) {
      value -= centerValue;
    }

    const double coordsIdx = coord + static_cast<double>(Index::center);
    const double floorValue = std::floor(coordsIdx);
    const double frac = floorValue - coordsIdx;
    const long intIdx = static_cast<long>(floorValue);
    const long clippedIdx = std::clamp(intIdx, static_cast<long>(0),
                                       static_cast<long>(windowSize) - 1);
    const double base = cumulative[clippedIdx];
    const double pitchAtIdx = pitches[clippedIdx];
    const double centerOfCluster = pitchOffset == Index::pitchXOffset
                                       ? this->centerPosition.xPhi()
                                       : this->centerPosition.xEta();
    return base + std::abs(frac) * pitchAtIdx + centerOfCluster;
  }

  /// @brief translate RMS that is output from NN to detector units (mm)
  /// Conversion is done  by integrating the actual pitches,
  /// so non-uniform pitch (ITk long/end pixels, 25 um modules) is handled.
  /// size is m_sizeX in phi (x), m_sizeY in eta (y).
  /// this code is adopted from NNClusterizationFactory
  /// @param prec precision returned by NN
  /// @param pitchOffset either pitchXOffset or pitchYOffset
  /// @return
  float precisionToRealCoord(float prec, Index pitchOffset) {
    std::span<payload_t, static_cast<index_t>(Index::windowSize)> pitches(
        m_payload.begin() + static_cast<index_t>(pitchOffset),
        m_payload.begin() + static_cast<index_t>(pitchOffset) +
            static_cast<index_t>(Index::windowSize));

    const float p = ((prec > 0) ? std::sqrt(1.0f / prec) : 0.01f) +
                    static_cast<float>(Index::center);
    float pitchPos = -100;     // unset position
    float pitchCenter = -100;  // unset position
    float pitchActual = 0;     // integral of pitch sizes
    for (index_t i = 0; i < static_cast<index_t>(Index::windowSize); i++) {
      if (p >= i and p <= (i + 1))
        pitchPos = pitchActual + (p - i + 0.5) * pitches[i];
      if (i == static_cast<index_t>(Index::center))
        pitchCenter = pitchActual + 0.5 * pitches[i];
      pitchActual += pitches[i];
    }
    return std::abs(pitchPos - pitchCenter);
  }

  ///! to avoid need to calculate them once more time when
  ///! translating from index biases to local module coordinates
  InDetDD::SiCellId centerCell;
  InDetDD::SiLocalPosition centerPosition;

 private:
  std::vector<payload_t> m_payload;  ///! storage for NN input

  /// validates index arguments passed to setters
  void checkRange(index_t i, const std::string& context) const {
    if (i >= static_cast<index_t>(Index::windowSize))
      throw std::domain_error(
          "Filling the NNinput for pixel calibration, the value should be "
          "within 0-6, while it "
          "is " +
          std::to_string(i) + " " + context);
  }
};

struct NumberNNoutput {
  using payload_t = NNinput::payload_t;  ///! payload from NN

  /***
   * @brief given number network output finds how many clusters there is
   * the numbers network resurns probabilities and we decide picking
   * highest probability outcome (position of highest probability +1 == nnumber
   * of clusters)
   * TODO check if in case of small differences in prob, we should not preffer
   * smaller number of clusters
   */
  static unsigned int maxProbIndex(const std::array<payload_t, 3>& prob) {
    return std::ranges::distance(std::begin(prob),
                                 std::ranges::max_element(prob));
  }
};

/***
 * @brief helper for decoding position networks output
 * Each position set contains 4 numbers x,y, and precision
 * Clusters are counted from 0,1,2 (max)
 * Data encoded according to:
 * https://pixsplit.docs.cern.ch/architecture/#positionregressor
 */
class PositionNNoutput {
 public:
  using payload_t = NNinput::payload_t;
  using index_t = NNinput::index_t;
  enum class Index : index_t {
    outputUnitSize = 5,
    xOffset = 1,
    yOffset = 2,
    xPrecOffset = 3,
    yPrecOffset = 4
  };

  PositionNNoutput(const float* begin, const float* end)
      : m_payload(begin, end) {}
  PositionNNoutput(int /*nPos*/)
      : m_payload(static_cast<index_t>(Index::outputUnitSize)) {}

  std::vector<payload_t>& payload() { return m_payload; }

  /// @brief returns number of (sub)clusters
  /// Effectively it depends on the position network used
  inline index_t numberOfSubClusters() {
    return m_payload.size() / static_cast<index_t>(Index::outputUnitSize);
  }

  /// @brief x position (in nn coordinates)
  /// @param cluster index of cluster (not checked for validity)
  /// @return position
  payload_t x(index_t cluster) {
    return m_payload[cluster * static_cast<index_t>(Index::outputUnitSize) +
                     static_cast<index_t>(Index::xOffset)];
  }

  /// @see x
  payload_t y(index_t cluster) {
    return m_payload[cluster * static_cast<index_t>(Index::outputUnitSize) +
                     static_cast<index_t>(Index::yOffset)];
  }

  /// @brief precision in nn coordinates
  payload_t xprec(index_t cluster) {
    return m_payload[cluster * static_cast<index_t>(Index::outputUnitSize) +
                     static_cast<index_t>(Index::xPrecOffset)];
  }

  payload_t yprec(index_t cluster) {
    return m_payload[cluster * static_cast<index_t>(Index::outputUnitSize) +
                     static_cast<index_t>(Index::yPrecOffset)];
  }

 private:
  std::vector<payload_t> m_payload;
};

}  // namespace ActsTrk

#endif