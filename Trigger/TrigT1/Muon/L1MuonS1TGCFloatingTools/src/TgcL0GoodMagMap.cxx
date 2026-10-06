/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TgcL0GoodMagMap.h"

#include "CxxUtils/StringUtils.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <numbers>
#include <string_view>

namespace {

bool parseInteger(const std::string& text, int& value) {
  try {
    value = CxxUtils::atoi(text);
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

bool parseUnsignedInteger(const std::string& text, unsigned int& value) {
  int parsed = 0;
  if (!parseInteger(text, parsed) || parsed < 0) return false;
  value = static_cast<unsigned int>(parsed);
  return true;
}

constexpr bool hasEmptyCsvField(const std::string_view line) {
  return line.empty() || line.front() == ',' || line.back() == ',' ||
         line.find(",,") != std::string_view::npos;
}

}  // namespace

namespace L1Muon {

std::unique_ptr<TgcL0GoodMagMap> TgcL0GoodMagMap::loadAscii(
    const std::string& calibrationPath, std::string& error) {
  error.clear();
  std::ifstream input{calibrationPath};
  if (!input) {
    error = "cannot open ASCII GoodMag map: " + calibrationPath;
    return nullptr;
  }

  auto map = std::unique_ptr<TgcL0GoodMagMap>{new TgcL0GoodMagMap};
  unsigned int schemaVersion = 0U;
  bool schemaVersionSeen = false;
  bool payloadVersionSeen = false;
  bool etaBinsSeen = false;
  bool phiBinsSeen = false;
  bool mapRecordsStarted = false;
  std::string line;
  std::size_t lineNumber = 0U;
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
      if (mapRecordsStarted) {
        error = "META row after map records at line " +
                std::to_string(lineNumber);
        return nullptr;
      }
      if (fields.size() != 3U) {
        error = "invalid META row at line " + std::to_string(lineNumber);
        return nullptr;
      }
      const std::string& key = fields[1];
      if (key == "schemaVersion") {
        if (schemaVersionSeen ||
            !parseUnsignedInteger(fields[2], schemaVersion)) {
          error = "invalid or duplicate schemaVersion";
          return nullptr;
        }
        schemaVersionSeen = true;
      } else if (key == "payloadVersion") {
        if (payloadVersionSeen || fields[2].empty()) {
          error = "invalid or duplicate payloadVersion";
          return nullptr;
        }
        map->m_version = fields[2];
        payloadVersionSeen = true;
      } else if (key == "etaBins") {
        if (etaBinsSeen ||
            !parseUnsignedInteger(fields[2], map->m_etaBins)) {
          error = "invalid or duplicate etaBins";
          return nullptr;
        }
        etaBinsSeen = true;
      } else if (key == "phiBinsPerFold") {
        if (phiBinsSeen ||
            !parseUnsignedInteger(fields[2], map->m_phiBinsPerFold)) {
          error = "invalid or duplicate phiBinsPerFold";
          return nullptr;
        }
        phiBinsSeen = true;
      } else {
        error = "unknown metadata key at line " +
                std::to_string(lineNumber) + ": " + key;
        return nullptr;
      }
      continue;
    }

    mapRecordsStarted = true;
    if (record != "POOR") {
      error = "unknown map record at line " + std::to_string(lineNumber) +
              ": " + record;
      return nullptr;
    }
    if (fields.size() != 3U || map->m_etaBins == 0U ||
        map->m_phiBinsPerFold == 0U) {
      error = "invalid POOR row at line " + std::to_string(lineNumber);
      return nullptr;
    }
    int etaBin = -1;
    int phiFoldBin = -1;
    if (!parseInteger(fields[1], etaBin) ||
        !parseInteger(fields[2], phiFoldBin) || etaBin < 0 ||
        phiFoldBin < 0 || etaBin >= static_cast<int>(map->m_etaBins) ||
        phiFoldBin >= static_cast<int>(map->m_phiBinsPerFold)) {
      error = "invalid map bin at line " + std::to_string(lineNumber);
      return nullptr;
    }
    if (map->m_poorBins.empty()) {
      const std::size_t count = static_cast<std::size_t>(map->m_etaBins) *
                                map->m_phiBinsPerFold;
      map->m_poorBins.assign(count, false);
    }
    const std::size_t index = static_cast<std::size_t>(etaBin) *
                                  map->m_phiBinsPerFold +
                              static_cast<std::size_t>(phiFoldBin);
    if (map->m_poorBins[index]) {
      error = "duplicate POOR bin at line " + std::to_string(lineNumber);
      return nullptr;
    }
    map->m_poorBins[index] = true;
    ++map->m_poorBinCount;
  }

  if (!schemaVersionSeen || schemaVersion != 1U || !payloadVersionSeen ||
      !etaBinsSeen || !phiBinsSeen || map->m_etaBins == 0U ||
      map->m_phiBinsPerFold == 0U) {
    error = "incomplete or unsupported ASCII GoodMag metadata";
    return nullptr;
  }
  if (map->m_poorBins.empty()) {
    const std::size_t count = static_cast<std::size_t>(map->m_etaBins) *
                              map->m_phiBinsPerFold;
    map->m_poorBins.assign(count, false);
  }
  return map;
}

bool TgcL0GoodMagMap::isGood(const int etaBin,
                             const int phiFoldBin) const {
  if (etaBin < 0 || phiFoldBin < 0 ||
      etaBin >= static_cast<int>(m_etaBins) ||
      phiFoldBin >= static_cast<int>(m_phiBinsPerFold)) {
    return false;
  }
  const std::size_t index = static_cast<std::size_t>(etaBin) *
                                m_phiBinsPerFold +
                            static_cast<std::size_t>(phiFoldBin);
  return !m_poorBins[index];
}

int TgcL0GoodMagMap::etaBin(const float eta, const float absEtaMin,
                            const float absEtaMax) const {
  if (!std::isfinite(eta) || !std::isfinite(absEtaMin) ||
      !std::isfinite(absEtaMax) || !(absEtaMax > absEtaMin)) {
    return -1;
  }
  const float absEta = std::abs(eta);
  if (absEta < absEtaMin || absEta > absEtaMax) return -1;
  if (absEta == absEtaMax) return static_cast<int>(m_etaBins) - 1;
  const float scaled =
      (absEta - absEtaMin) * m_etaBins / (absEtaMax - absEtaMin);
  const int bin = static_cast<int>(std::floor(scaled));
  return bin >= 0 && bin < static_cast<int>(m_etaBins) ? bin : -1;
}

int TgcL0GoodMagMap::phiFoldBin(const float phi) const {
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

bool TgcL0GoodMagMap::isGood(const float eta, const float phi,
                             const float absEtaMin,
                             const float absEtaMax) const {
  return isGood(etaBin(eta, absEtaMin, absEtaMax), phiFoldBin(phi));
}

}  // namespace L1Muon
