/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGPTLUT_H
#define L1MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGPTLUT_H

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace L0Muon {

enum class TgcL0FloatingPtResponseMode : std::uint8_t {
  Invalid = 0,
  Linear = 1,
  FloatingMonotonicLut = 2
};

enum class TgcL0FloatingThresholdCalibrationStatus : std::uint8_t {
  None = 0,
  Direct = 1,
  PhiMedianFallback = 2,
  EtaNeighborFallback = 3,
  GlobalFallback = 4,
  Provisional = 5
};

struct TgcL0FloatingPtEvaluation {
  bool modelValid{false};
  bool ptEstimateValid{false};
  int etaBin{-1};
  int phiFoldBin{-1};
  /// Unbounded floating-point estimate retained for diagnostics only.
  float rawPtEstimateGeV{0.F};
  /// Operational pT value, saturated to the downstream 8-bit range.
  float ptEstimateGeV{0.F};
  std::uint8_t estimatedPtValueIndex{0};
  bool chargeEstimateValid{false};
  std::int8_t estimatedCharge{0};
  std::uint8_t thresholdCode{0};
  float thresholdCutGeV{0.F};
  TgcL0FloatingPtResponseMode responseMode{
      TgcL0FloatingPtResponseMode::Invalid};
  TgcL0FloatingThresholdCalibrationStatus thresholdCalibrationStatus{
      TgcL0FloatingThresholdCalibrationStatus::None};
};

/**
 * @brief Immutable Floating-pT runtime calibration loaded from ASCII.
 *
 * Loading performs all file I/O and validation. Evaluation is read-only.
 */
class TgcL0FloatingPtLut {
 public:
  static constexpr float s_maxEncodedPtGeV = 127.5F;

  /// Load the human-readable calibration payload.
  static std::unique_ptr<TgcL0FloatingPtLut> loadAscii(
      const std::string& calibrationPath, std::string& error);

  TgcL0FloatingPtEvaluation evaluate(float eta, float phi,
                                     float signedDTheta) const;

  const std::string& version() const { return m_version; }
  unsigned int etaBins() const { return m_etaBins; }
  unsigned int phiBinsPerFold() const { return m_phiBinsPerFold; }
  std::size_t knotCount() const { return m_knots.size(); }
  float absEtaMin() const { return m_absEtaMin; }
  float absEtaMax() const { return m_absEtaMax; }
  bool isDevelopmentPayload() const { return m_isDevelopmentPayload; }

 private:
  TgcL0FloatingPtLut() = default;

  struct Knot {
    float inversePtGeVInv{0.F};
    float responseMagnitudeRad{0.F};
  };

  struct Bin {
    float linearSlopeMagnitudeRadGeV{0.F};
    float transitionPtGeV{0.F};
    std::uint32_t knotOffset{0U};
    std::uint32_t knotCount{0U};
    std::array<float, 16> thresholdCutsGeV{};
    std::array<TgcL0FloatingThresholdCalibrationStatus, 16>
        thresholdStatuses{};
  };

  int etaBin(float eta) const;
  int phiFoldBin(float phi) const;
  const Bin* findBin(int etaBin, int phiFoldBin) const;
  bool invertFloatingResponse(const Bin& bin, float responseMagnitudeRad,
                              float& inversePtGeVInv) const;

  std::string m_version{};
  unsigned int m_etaBins{0U};
  unsigned int m_phiBinsPerFold{0U};
  float m_absEtaMin{0.F};
  float m_absEtaMax{0.F};
  std::vector<Bin> m_bins{};
  std::vector<Knot> m_knots{};
  bool m_isDevelopmentPayload{false};
};

}  // namespace L0Muon

#endif
