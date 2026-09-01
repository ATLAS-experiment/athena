/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TgcL0FloatingPtLut.h"

#include "CxxUtils/StringUtilsTemplates.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <fstream>
#include <limits>
#include <numbers>
#include <string_view>
#include <type_traits>

namespace {

template <std::integral T>
bool parseInteger(std::string_view text, T& value)
{
  try {
    CxxUtils::convertToNumber(text, value);
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

template <std::floating_point T>
bool parseFloatingPoint(std::string_view text, T& value)
{
  try {
    CxxUtils::convertToNumber(text, value);
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

constexpr bool hasEmptyCsvField(const std::string_view line) {
  return line.empty() || line.front() == ',' || line.back() == ',' ||
         line.find(",,") != std::string_view::npos;
}

std::uint8_t encodePtValue(const float ptGeV) {
  if (!std::isfinite(ptGeV) || ptGeV <= 0.F) return 0U;
  const long encoded = std::lround(2.F * std::min(
      ptGeV, L0Muon::TgcL0FloatingPtLut::s_maxEncodedPtGeV));
  return static_cast<std::uint8_t>(std::clamp(encoded, 0L, 255L));
}

std::int8_t chargeFromSignedDTheta(const float signedDTheta) {
  if (!std::isfinite(signedDTheta) || signedDTheta == 0.F) return 0;
  return static_cast<std::int8_t>(-std::copysign(1.F, signedDTheta));
}

}  // namespace

namespace L0Muon {

std::unique_ptr<TgcL0FloatingPtLut> TgcL0FloatingPtLut::loadAscii(
    const std::string& calibrationPath, std::string& error) {
  error.clear();
  std::ifstream input{calibrationPath};
  if (!input) {
    error = "cannot open ASCII calibration file: " + calibrationPath;
    return nullptr;
  }

  auto lut = std::unique_ptr<TgcL0FloatingPtLut>{new TgcL0FloatingPtLut};
  unsigned int schemaVersion = 0U;
  std::vector<bool> binSeen;
  std::vector<std::array<bool, 16>> thresholdSeen;
  std::vector<std::vector<std::pair<unsigned int, Knot>>> knotsByBin;
  std::string line;
  std::size_t lineNumber = 0U;
  bool calibrationRecordsStarted = false;
  while (std::getline(input, line)) {
    ++lineNumber;
    if (line.empty() || line[0] == '#') continue;
    if (hasEmptyCsvField(line)) {
      error = "empty CSV field at line " + std::to_string(lineNumber);
      return nullptr;
    }
    const auto fields = CxxUtils::tokenize(line, ',');
    if (fields.empty()) continue;
    const std::string& record = fields[0];
    if (record == "META") {
      if (calibrationRecordsStarted) {
        error = "META row after calibration records at line " +
                std::to_string(lineNumber);
        return nullptr;
      }
      if (fields.size() != 3U) {
        error = "invalid META row at line " + std::to_string(lineNumber);
        return nullptr;
      }
      if (fields[1] == "schemaVersion") {
        if (!parseInteger(fields[2], schemaVersion)) {
          error = "invalid schemaVersion";
          return nullptr;
        }
      } else if (fields[1] == "payloadVersion") {
        lut->m_version = fields[2];
      } else if (fields[1] == "etaBins") {
        if (!parseInteger(fields[2], lut->m_etaBins)) {
          error = "invalid etaBins";
          return nullptr;
        }
      } else if (fields[1] == "phiBinsPerFold") {
        if (!parseInteger(fields[2], lut->m_phiBinsPerFold)) {
          error = "invalid phiBinsPerFold";
          return nullptr;
        }
      } else if (fields[1] == "absEtaMin") {
        if (!parseFloatingPoint(fields[2], lut->m_absEtaMin)) {
          error = "invalid absEtaMin";
          return nullptr;
        }
      } else if (fields[1] == "absEtaMax") {
        if (!parseFloatingPoint(fields[2], lut->m_absEtaMax)) {
          error = "invalid absEtaMax";
          return nullptr;
        }
      } else if (fields[1] == "payloadMode") {
        lut->m_isDevelopmentPayload = fields[2] == "development";
      }
      continue;
    }
    calibrationRecordsStarted = true;

    if (lut->m_etaBins == 0U || lut->m_phiBinsPerFold == 0U) {
      error = "META dimensions must precede calibration records";
      return nullptr;
    }
    const std::size_t count = static_cast<std::size_t>(lut->m_etaBins) *
                              lut->m_phiBinsPerFold;
    if (lut->m_bins.empty()) {
      lut->m_bins.resize(count);
      binSeen.assign(count, false);
      thresholdSeen.resize(count);
      knotsByBin.resize(count);
    }

    int eta = -1;
    int phi = -1;
    if (fields.size() < 3U || !parseInteger(fields[1], eta) ||
        !parseInteger(fields[2], phi) || eta < 0 || phi < 0 ||
        eta >= static_cast<int>(lut->m_etaBins) ||
        phi >= static_cast<int>(lut->m_phiBinsPerFold)) {
      error = "invalid calibration bin at line " +
              std::to_string(lineNumber);
      return nullptr;
    }
    const std::size_t index = static_cast<std::size_t>(eta) *
                                  lut->m_phiBinsPerFold +
                              static_cast<std::size_t>(phi);
    Bin& bin = lut->m_bins[index];
    if (record == "BIN") {
      if (fields.size() != 5U || binSeen[index] ||
          !parseFloatingPoint(fields[3], bin.linearSlopeMagnitudeRadGeV) ||
          !parseFloatingPoint(fields[4], bin.transitionPtGeV)) {
        error = "invalid BIN row at line " + std::to_string(lineNumber);
        return nullptr;
      }
      binSeen[index] = true;
    } else if (record == "THRESHOLD") {
      int code = 0;
      int status = 0;
      float cut = 0.F;
      if (fields.size() != 6U || !parseInteger(fields[3], code) ||
          !parseFloatingPoint(fields[4], cut) ||
          !parseInteger(fields[5], status) || code < 1 || code > 14 ||
          status < 1 || status > 5 || thresholdSeen[index][code]) {
        error = "invalid THRESHOLD row at line " +
                std::to_string(lineNumber);
        return nullptr;
      }
      bin.thresholdCutsGeV[code] = cut;
      bin.thresholdStatuses[code] =
          static_cast<TgcL0FloatingThresholdCalibrationStatus>(status);
      thresholdSeen[index][code] = true;
    } else if (record == "KNOT") {
      unsigned int knotIndex = 0U;
      Knot knot;
      if (fields.size() != 6U ||
          !parseInteger(fields[3], knotIndex) ||
          !parseFloatingPoint(fields[4], knot.inversePtGeVInv) ||
          !parseFloatingPoint(fields[5], knot.responseMagnitudeRad)) {
        error = "invalid KNOT row at line " + std::to_string(lineNumber);
        return nullptr;
      }
      knotsByBin[index].emplace_back(knotIndex, knot);
    } else {
      error = "unknown calibration record at line " +
              std::to_string(lineNumber) + ": " + record;
      return nullptr;
    }
  }

  if (schemaVersion != 1U || lut->m_version.empty() ||
      !std::isfinite(lut->m_absEtaMin) ||
      !std::isfinite(lut->m_absEtaMax) ||
      !(lut->m_absEtaMax > lut->m_absEtaMin) || lut->m_bins.empty()) {
    error = "incomplete or unsupported ASCII calibration metadata";
    return nullptr;
  }
  for (std::size_t index = 0U; index < lut->m_bins.size(); ++index) {
    Bin& bin = lut->m_bins[index];
    if (!binSeen[index] ||
        !std::isfinite(bin.linearSlopeMagnitudeRadGeV) ||
        !std::isfinite(bin.transitionPtGeV) ||
        !(bin.linearSlopeMagnitudeRadGeV > 0.F) ||
        !(bin.transitionPtGeV > 0.F)) {
      error = "missing or invalid BIN record for bin " +
              std::to_string(index);
      return nullptr;
    }
    float previousCut = 0.F;
    for (unsigned int code = 1U; code <= 14U; ++code) {
      const float cut = bin.thresholdCutsGeV[code];
      if (!thresholdSeen[index][code] || !std::isfinite(cut) ||
          !(cut > 0.F) || cut < previousCut) {
        error = "missing or invalid threshold for bin " +
                std::to_string(index);
        return nullptr;
      }
      previousCut = cut;
    }
    auto& indexedKnots = knotsByBin[index];
    std::sort(indexedKnots.begin(), indexedKnots.end(),
              [](const auto& left, const auto& right) {
                return left.first < right.first;
              });
    if (indexedKnots.size() < 2U) {
      error = "fewer than two knots for bin " + std::to_string(index);
      return nullptr;
    }
    bin.knotOffset = static_cast<std::uint32_t>(lut->m_knots.size());
    bin.knotCount = static_cast<std::uint32_t>(indexedKnots.size());
    unsigned int expectedIndex = 0U;
    float previousInversePt = -1.F;
    float previousResponse = -1.F;
    for (const auto& [knotIndex, knot] : indexedKnots) {
      if (knotIndex != expectedIndex++ ||
          !std::isfinite(knot.inversePtGeVInv) ||
          !std::isfinite(knot.responseMagnitudeRad) ||
          !(knot.inversePtGeVInv > 0.F) ||
          !(knot.responseMagnitudeRad >= 0.F) ||
          knot.inversePtGeVInv < previousInversePt ||
          knot.responseMagnitudeRad < previousResponse) {
        error = "invalid or non-monotonic knot sequence for bin " +
                std::to_string(index);
        return nullptr;
      }
      previousInversePt = knot.inversePtGeVInv;
      previousResponse = knot.responseMagnitudeRad;
      lut->m_knots.push_back(knot);
    }
  }

  return lut;
}

int TgcL0FloatingPtLut::etaBin(const float eta) const {
  if (!std::isfinite(eta)) return -1;
  const float absEta = std::abs(eta);
  if (absEta < m_absEtaMin || absEta > m_absEtaMax) return -1;
  if (absEta == m_absEtaMax) return static_cast<int>(m_etaBins) - 1;
  const float scaled = (absEta - m_absEtaMin) * m_etaBins /
                       (m_absEtaMax - m_absEtaMin);
  const int bin = static_cast<int>(std::floor(scaled));
  return bin >= 0 && bin < static_cast<int>(m_etaBins) ? bin : -1;
}

int TgcL0FloatingPtLut::phiFoldBin(const float phi) const {
  if (!std::isfinite(phi)) return -1;
  const float period = 2.F * std::numbers::pi_v<float> / 8.F;
  float folded = std::fmod(phi, period);
  if (folded < 0.F) folded += period;
  float fraction = folded / period;
  if (fraction >= 1.F) fraction = 0.F;
  return std::clamp(
      static_cast<int>(std::floor(fraction * m_phiBinsPerFold)), 0,
      static_cast<int>(m_phiBinsPerFold) - 1);
}

const TgcL0FloatingPtLut::Bin* TgcL0FloatingPtLut::findBin(
    const int eta, const int phi) const {
  if (eta < 0 || eta >= static_cast<int>(m_etaBins) || phi < 0 ||
      phi >= static_cast<int>(m_phiBinsPerFold)) {
    return nullptr;
  }
  return &m_bins[static_cast<std::size_t>(eta) * m_phiBinsPerFold + phi];
}

bool TgcL0FloatingPtLut::invertFloatingResponse(
    const Bin& bin, const float responseMagnitudeRad,
    float& inversePtGeVInv) const {
  const auto begin = m_knots.begin() + bin.knotOffset;
  const auto end = begin + bin.knotCount;
  if (responseMagnitudeRad <= begin->responseMagnitudeRad) {
    inversePtGeVInv = begin->inversePtGeVInv;
    return true;
  }
  const auto upper = std::lower_bound(
      begin, end, responseMagnitudeRad,
      [](const Knot& knot, const float value) {
        return knot.responseMagnitudeRad < value;
      });
  if (upper != end) {
    if (upper->responseMagnitudeRad == responseMagnitudeRad) {
      inversePtGeVInv = upper->inversePtGeVInv;
      return true;
    }
    const Knot& low = *(upper - 1);
    const float delta = upper->responseMagnitudeRad -
                        low.responseMagnitudeRad;
    if (!(delta > 0.F)) return false;
    const float fraction =
        (responseMagnitudeRad - low.responseMagnitudeRad) / delta;
    inversePtGeVInv = low.inversePtGeVInv +
                      fraction * (upper->inversePtGeVInv -
                                  low.inversePtGeVInv);
    return true;
  }
  const Knot& last = *(end - 1);
  const Knot& previous = *(end - 2);
  const float deltaResponse =
      last.responseMagnitudeRad - previous.responseMagnitudeRad;
  if (!(deltaResponse > 0.F)) return false;
  inversePtGeVInv = last.inversePtGeVInv +
                    (last.inversePtGeVInv - previous.inversePtGeVInv) /
                        deltaResponse *
                        (responseMagnitudeRad - last.responseMagnitudeRad);
  inversePtGeVInv = std::clamp(inversePtGeVInv, 0.F,
                               last.inversePtGeVInv);
  return true;
}

TgcL0FloatingPtEvaluation TgcL0FloatingPtLut::evaluate(
    const float eta, const float phi, const float signedDTheta) const {
  TgcL0FloatingPtEvaluation result{};
  result.etaBin = etaBin(eta);
  result.phiFoldBin = phiFoldBin(phi);
  if (result.etaBin < 0 || result.phiFoldBin < 0 ||
      !std::isfinite(signedDTheta)) {
    return result;
  }
  const Bin* bin = findBin(result.etaBin, result.phiFoldBin);
  if (bin == nullptr) return result;
  result.modelValid = true;
  result.estimatedCharge = chargeFromSignedDTheta(signedDTheta);
  result.chargeEstimateValid = result.estimatedCharge != 0;

  const float magnitude = std::abs(signedDTheta);
  const float transitionMagnitude =
      bin->linearSlopeMagnitudeRadGeV / bin->transitionPtGeV;
  float inversePt = 0.F;
  if (magnitude <= transitionMagnitude) {
    result.responseMode = TgcL0FloatingPtResponseMode::Linear;
    if (magnitude == 0.F) {
      // Zero bending means that only a lower bound can be represented.  Keep
      // the diagnostic raw value finite while saturating the operational pT.
      result.rawPtEstimateGeV = s_maxEncodedPtGeV;
      result.ptEstimateValid = true;
    } else {
      inversePt = magnitude / bin->linearSlopeMagnitudeRadGeV;
    }
  } else {
    result.responseMode = TgcL0FloatingPtResponseMode::FloatingMonotonicLut;
    if (!invertFloatingResponse(*bin, magnitude, inversePt)) return result;
  }
  if (!result.ptEstimateValid) {
    if (!std::isfinite(inversePt) || inversePt <= 0.F) return result;
    result.rawPtEstimateGeV = 1.F / inversePt;
    result.ptEstimateValid = std::isfinite(result.rawPtEstimateGeV) &&
                             result.rawPtEstimateGeV > 0.F;
  }
  if (!result.ptEstimateValid) return result;

  // The raw floating-point estimate is diagnostic provenance.  Candidate
  // ordering, thresholds and downstream EDM values use the representable
  // 8-bit range and therefore saturate at 127.5 GeV.
  result.ptEstimateGeV =
      std::min(result.rawPtEstimateGeV, s_maxEncodedPtGeV);
  result.estimatedPtValueIndex = encodePtValue(result.ptEstimateGeV);
  for (int code = 14; code >= 1; --code) {
    const float cut = bin->thresholdCutsGeV[code];
    if (result.ptEstimateGeV >= cut) {
      result.thresholdCode = static_cast<std::uint8_t>(code);
      result.thresholdCutGeV = cut;
      result.thresholdCalibrationStatus = bin->thresholdStatuses[code];
      break;
    }
  }
  return result;
}

}  // namespace L0Muon
