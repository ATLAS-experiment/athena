/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */

#include "TgcL0GoodMagMap.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <numbers>
#include <string>
#include <string_view>

namespace {

enum class Malformation {
  None,
  UnsupportedSchema,
  MissingVersion,
  NegativeDimension,
  InvalidBin,
  DuplicatePoorBin,
  LateMetadata,
  UnknownRecord,
  LeadingEmptyField,
  InteriorEmptyField,
  TrailingEmptyField
};

void writeMap(const std::filesystem::path& path,
              const Malformation malformation = Malformation::None) {
  std::ofstream output{path};
  if (malformation == Malformation::LeadingEmptyField) {
    output << ",META,schemaVersion,1\n";
  } else if (malformation == Malformation::InteriorEmptyField) {
    output << "META,,schemaVersion,1\n";
  } else if (malformation == Malformation::TrailingEmptyField) {
    output << "META,schemaVersion,1,\n";
  }
  output << "META,schemaVersion,"
         << (malformation == Malformation::UnsupportedSchema ? "2" : "1")
         << "\n";
  if (malformation != Malformation::MissingVersion) {
    output << "META,payloadVersion,unit_test\n";
  }
  output << "META,etaBins,"
         << (malformation == Malformation::NegativeDimension ? "-2" : "2")
         << "\n"
         << "META,phiBinsPerFold,3\n"
         << "POOR,0,1\n"
         << "POOR,1,2\n";
  if (malformation == Malformation::InvalidBin) {
    output << "POOR,2,0\n";
  } else if (malformation == Malformation::DuplicatePoorBin) {
    output << "POOR,0,1\n";
  } else if (malformation == Malformation::LateMetadata) {
    output << "META,etaBins,2\n";
  } else if (malformation == Malformation::UnknownRecord) {
    output << "GOOD,0,0\n";
  }
}

bool check(const bool condition, const std::string_view description) {
  if (!condition) std::cerr << "Test failed: " << description << '\n';
  return condition;
}

}  // namespace

int main() {
  bool success = true;
  const std::filesystem::path mapPath{"TgcL0GoodMagMap_test.txt"};
  writeMap(mapPath);

  std::string error;
  success &= check(!L0Muon::TgcL0GoodMagMap::loadAscii(
                       "TgcL0GoodMagMap_test_missing.txt", error),
                   "missing payload is rejected");
  success &= check(!error.empty(), "missing payload reports an error");

  error.clear();
  const auto map =
      L0Muon::TgcL0GoodMagMap::loadAscii(mapPath.string(), error);
  if (!check(static_cast<bool>(map), "valid payload is loaded")) {
    std::filesystem::remove(mapPath);
    return EXIT_FAILURE;
  }
  success &= check(error.empty(), "valid payload reports no error");
  success &= check(map->version() == "unit_test", "payload version");
  success &= check(map->etaBins() == 2U, "eta-bin count");
  success &= check(map->phiBinsPerFold() == 3U, "phi-bin count");
  success &= check(map->poorBinCount() == 2U, "poor-bin count");
  success &= check(map->isGood(0, 0), "unmasked bin is GoodMag");
  success &= check(!map->isGood(0, 1), "first poor bin is masked");
  success &= check(!map->isGood(1, 2), "second poor bin is masked");
  success &= check(!map->isGood(-1, 0), "negative eta bin is rejected");
  success &= check(!map->isGood(2, 0), "high eta bin is rejected");
  success &= check(!map->isGood(0, 3), "high phi bin is rejected");

  constexpr float absEtaMin = 1.F;
  constexpr float absEtaMax = 2.F;
  constexpr float phiPeriod = 2.F * std::numbers::pi_v<float> / 8.F;
  success &= check(map->isGood(1.1F, 0.F, absEtaMin, absEtaMax),
                   "physical coordinates use the map eta and phi binning");
  success &= check(!map->isGood(1.1F, phiPeriod / 2.F, absEtaMin,
                                absEtaMax),
                   "physical coordinates find the first poor bin");
  success &= check(!map->isGood(1.1F, -phiPeriod / 2.F, absEtaMin,
                                absEtaMax),
                   "negative physical phi is folded into the map");
  success &= check(!map->isGood(-1.9F, 5.F * phiPeriod / 6.F, absEtaMin,
                                absEtaMax),
                   "absolute eta and folded phi find the second poor bin");
  success &= check(map->isGood(2.F, phiPeriod, absEtaMin, absEtaMax),
                   "upper eta edge and repeated phi sector are accepted");
  success &= check(!map->isGood(0.9F, 0.F, absEtaMin, absEtaMax),
                   "physical eta below the calibrated range is rejected");
  success &= check(
      !map->isGood(std::numeric_limits<float>::quiet_NaN(), 0.F,
                   absEtaMin, absEtaMax),
      "non-finite physical eta is rejected");
  success &= check(
      !map->isGood(1.1F, std::numeric_limits<float>::infinity(),
                   absEtaMin, absEtaMax),
      "non-finite physical phi is rejected");
  success &= check(!map->isGood(1.1F, 0.F, absEtaMax, absEtaMin),
                   "invalid physical eta range is rejected");

  constexpr std::array malformedPayloads{
      Malformation::UnsupportedSchema, Malformation::MissingVersion,
      Malformation::NegativeDimension, Malformation::InvalidBin,
      Malformation::DuplicatePoorBin, Malformation::LateMetadata,
      Malformation::UnknownRecord, Malformation::LeadingEmptyField,
      Malformation::InteriorEmptyField, Malformation::TrailingEmptyField};
  for (std::size_t index = 0U; index < malformedPayloads.size(); ++index) {
    const std::filesystem::path path{
        "TgcL0GoodMagMap_test_malformed_" + std::to_string(index) + ".txt"};
    writeMap(path, malformedPayloads[index]);
    error.clear();
    success &= check(
        !L0Muon::TgcL0GoodMagMap::loadAscii(path.string(), error),
        "malformed payload is rejected");
    success &= check(!error.empty(), "malformed payload reports an error");
    std::filesystem::remove(path);
  }

  std::filesystem::remove(mapPath);
  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
