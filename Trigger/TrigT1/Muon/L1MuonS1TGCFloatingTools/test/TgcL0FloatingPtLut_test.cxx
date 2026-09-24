/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */

#include "TgcL0FloatingPtLut.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

namespace {

enum class Malformation {
  None,
  DuplicateThreshold,
  NonMonotonicKnot,
  InvalidNumber,
  NonFiniteNumber,
  NegativeDimension,
  LateMetadata,
  LeadingEmptyField,
  InteriorEmptyField,
  TrailingEmptyField
};

void writeCalibration(const std::filesystem::path& path,
                      const Malformation malformation = Malformation::None,
                      const bool repeatKnotResponse = false) {
  std::ofstream output{path};
  if (malformation == Malformation::LeadingEmptyField) {
    output << ",META,schemaVersion,1\n";
  } else if (malformation == Malformation::InteriorEmptyField) {
    output << "META,,schemaVersion,1\n";
  } else if (malformation == Malformation::TrailingEmptyField) {
    output << "META,schemaVersion,1,\n";
  }
  output << "META,schemaVersion,1\n"
         << "META,payloadVersion,unit_test\n"
         << "META,etaBins,"
         << (malformation == Malformation::NegativeDimension ? "-1" : "1")
         << "\n"
         << "META,phiBinsPerFold,2\n"
         << "META,absEtaMin,1.0\n"
         << "META,absEtaMax,2.0\n"
         << "META,payloadMode,development\n";
  for (unsigned int phi = 0U; phi < 2U; ++phi) {
    const float scale = static_cast<float>(phi + 1U);
    output << "BIN,0," << phi << ','
           << (malformation == Malformation::InvalidNumber && phi == 0U
                   ? "invalid"
                   : malformation == Malformation::NonFiniteNumber &&
                             phi == 0U
                         ? "nan"
                         : std::to_string(scale))
           << ",10.0\n";
    for (unsigned int code = 1U; code <= 14U; ++code) {
      output << "THRESHOLD,0," << phi << ',' << code << ',' << code
             << ",1\n";
    }
    output << "KNOT,0," << phi << ",0,0.1," << 0.1F * scale << "\n"
           << "KNOT,0," << phi << ",1,0.2," << 0.2F * scale << "\n"
           << "KNOT,0," << phi << ",2,0.4,"
           << (malformation == Malformation::NonMonotonicKnot && phi == 0U
                   ? 0.15F
                   : repeatKnotResponse && phi == 0U
                         ? 0.2F
                         : 0.4F * scale)
           << "\n";
  }
  if (malformation == Malformation::DuplicateThreshold) {
    output << "THRESHOLD,0,0,1,1,1\n";
  } else if (malformation == Malformation::LateMetadata) {
    output << "META,etaBins,1\n";
  }
}

bool close(const float left, const float right) {
  return std::abs(left - right) < 1.e-5F;
}

bool check(const bool condition, const std::string_view description) {
  if (!condition) {
    std::cerr << "Test failed: " << description << '\n';
  }
  return condition;
}

}  // namespace

int main() {
  bool success = true;
  const std::filesystem::path calibration{"TgcL0FloatingPtLut_test.txt"};
  writeCalibration(calibration);

  std::string error;
  success &= check(!L0Muon::TgcL0FloatingPtLut::loadAscii(
                       "TgcL0FloatingPtLut_test_missing.txt", error),
                   "missing payload is rejected");
  success &= check(!error.empty(), "missing payload reports an error");

  error.clear();
  const auto lut = L0Muon::TgcL0FloatingPtLut::loadAscii(
      calibration.string(), error);
  if (!check(static_cast<bool>(lut), "valid payload is loaded")) {
    std::filesystem::remove(calibration);
    return EXIT_FAILURE;
  }
  success &= check(error.empty(), "valid payload reports no error");
  success &= check(lut->version() == "unit_test", "payload version");
  success &= check(lut->isDevelopmentPayload(), "development payload mode");
  success &= check(lut->etaBins() == 1U, "eta-bin count");
  success &= check(lut->phiBinsPerFold() == 2U, "phi-bin count");
  success &= check(lut->knotCount() == 6U, "knot count");

  const auto linear = lut->evaluate(1.5F, 0.F, 0.05F);
  success &= check(linear.modelValid && linear.ptEstimateValid,
                   "linear estimate validity");
  success &= check(
      linear.responseMode == L0Muon::TgcL0FloatingPtResponseMode::Linear,
      "linear response mode");
  success &= check(close(linear.ptEstimateGeV, 20.F), "linear pT estimate");
  success &= check(linear.estimatedPtValueIndex == 40U,
                   "linear encoded pT");
  success &= check(linear.estimatedCharge == -1, "negative charge estimate");
  success &= check(linear.thresholdCode == 14U, "linear threshold code");
  success &= check(linear.phiFoldBin == 0, "first folded-phi bin");

  const auto foldedPhi = lut->evaluate(1.5F, 0.6F, 0.1F);
  success &= check(foldedPhi.phiFoldBin == 1, "second folded-phi bin");
  success &= check(close(foldedPhi.ptEstimateGeV, 20.F),
                   "folded-phi pT estimate");

  success &= check(lut->evaluate(1.5F, -0.1F, 0.1F).phiFoldBin == 1,
                   "negative phi folding");

  const auto exactKnot = lut->evaluate(1.5F, 0.F, -0.2F);
  success &= check(
      exactKnot.responseMode ==
          L0Muon::TgcL0FloatingPtResponseMode::FloatingMonotonicLut,
      "LUT response mode");
  success &= check(close(exactKnot.ptEstimateGeV, 5.F), "exact-knot pT");
  success &= check(exactKnot.estimatedCharge == 1,
                   "positive charge estimate");
  success &= check(exactKnot.thresholdCode == 5U,
                   "exact-knot threshold code");

  const auto interpolated = lut->evaluate(1.5F, 0.F, 0.15F);
  success &= check(close(interpolated.ptEstimateGeV, 1.F / 0.15F),
                   "interpolated pT");
  success &= check(interpolated.thresholdCode == 6U,
                   "interpolated threshold code");

  const auto extrapolated = lut->evaluate(1.5F, 0.F, 0.5F);
  success &= check(close(extrapolated.ptEstimateGeV, 2.5F),
                   "extrapolated pT");

  const auto zero = lut->evaluate(1.5F, 0.F, 0.F);
  success &= check(zero.ptEstimateValid && !zero.chargeEstimateValid,
                   "zero-bending validity");
  success &= check(
      close(zero.ptEstimateGeV,
            L0Muon::TgcL0FloatingPtLut::s_maxEncodedPtGeV),
      "zero-bending saturation");
  success &= check(zero.estimatedPtValueIndex == 255U,
                   "zero-bending encoded pT");

  const auto saturated = lut->evaluate(1.5F, 0.F, 0.0005F);
  success &= check(saturated.ptEstimateValid && saturated.chargeEstimateValid,
                   "saturated estimate validity");
  success &= check(
      saturated.rawPtEstimateGeV >
          L0Muon::TgcL0FloatingPtLut::s_maxEncodedPtGeV,
      "unbounded raw pT");
  success &= check(
      close(saturated.ptEstimateGeV,
            L0Muon::TgcL0FloatingPtLut::s_maxEncodedPtGeV),
      "operational pT saturation");
  success &= check(saturated.estimatedPtValueIndex == 255U,
                   "saturated encoded pT");

  success &= check(lut->evaluate(1.F, 0.F, 0.1F).modelValid,
                   "lower eta boundary");
  success &= check(lut->evaluate(2.F, 0.F, 0.1F).modelValid,
                   "upper eta boundary");
  success &= check(!lut->evaluate(0.99F, 0.F, 0.1F).modelValid,
                   "eta below range");
  success &= check(!lut->evaluate(2.01F, 0.F, 0.1F).modelValid,
                   "eta above range");
  success &= check(
      !lut->evaluate(1.5F, 0.F, std::numeric_limits<float>::quiet_NaN())
           .modelValid,
      "non-finite dTheta");
  success &= check(
      !lut->evaluate(std::numeric_limits<float>::quiet_NaN(), 0.F, 0.1F)
           .modelValid,
      "non-finite eta");
  success &= check(
      !lut->evaluate(1.5F, std::numeric_limits<float>::quiet_NaN(), 0.1F)
           .modelValid,
      "non-finite phi");

  const std::filesystem::path repeatedResponseCalibration{
      "TgcL0FloatingPtLut_test_repeated_response.txt"};
  writeCalibration(repeatedResponseCalibration, Malformation::None, true);
  error.clear();
  const auto repeatedResponseLut = L0Muon::TgcL0FloatingPtLut::loadAscii(
      repeatedResponseCalibration.string(), error);
  success &= check(static_cast<bool>(repeatedResponseLut),
                   "repeated knot response is loaded");
  success &= check(error.empty(),
                   "repeated knot response reports no error");
  if (repeatedResponseLut) {
    success &= check(repeatedResponseLut->knotCount() == 5U,
                     "repeated knot response is collapsed");
    const auto plateau = repeatedResponseLut->evaluate(1.5F, 0.F, 0.2F);
    success &= check(plateau.ptEstimateValid &&
                         close(plateau.ptEstimateGeV, 2.5F),
                     "response plateau uses the conservative pT estimate");
  }
  std::filesystem::remove(repeatedResponseCalibration);

  constexpr std::array malformedPayloads{
      Malformation::DuplicateThreshold, Malformation::NonMonotonicKnot,
      Malformation::InvalidNumber, Malformation::NonFiniteNumber,
      Malformation::NegativeDimension, Malformation::LateMetadata};
  for (std::size_t index = 0U; index < malformedPayloads.size(); ++index) {
    const std::filesystem::path path{
        "TgcL0FloatingPtLut_test_malformed_" + std::to_string(index) +
        ".txt"};
    writeCalibration(path, malformedPayloads[index]);
    error.clear();
    success &= check(!L0Muon::TgcL0FloatingPtLut::loadAscii(
                         path.string(), error),
                     "malformed payload is rejected");
    success &= check(!error.empty(), "malformed payload reports an error");
    std::filesystem::remove(path);
  }

  constexpr std::array emptyFieldPayloads{
      Malformation::LeadingEmptyField, Malformation::InteriorEmptyField,
      Malformation::TrailingEmptyField};
  for (std::size_t index = 0U; index < emptyFieldPayloads.size(); ++index) {
    const std::filesystem::path path{
        "TgcL0FloatingPtLut_test_empty_field_" +
        std::to_string(index) + ".txt"};
    writeCalibration(path, emptyFieldPayloads[index]);
    error.clear();
    success &= check(!L0Muon::TgcL0FloatingPtLut::loadAscii(
                         path.string(), error),
                     "empty CSV field is rejected");
    success &= check(!error.empty(), "empty CSV field reports an error");
    std::filesystem::remove(path);
  }

  std::filesystem::remove(calibration);
  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
