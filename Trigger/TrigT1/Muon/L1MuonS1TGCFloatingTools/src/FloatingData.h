/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_FLOATINGDATA_H
#define L1MUONS1TGCFLOATINGTOOLS_FLOATINGDATA_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <tuple>
#include <vector>

namespace L1Muon::L1TGC {

enum class FloatingStation : std::uint8_t { M1, M2, M3, Inner, Unknown, NumberOfStations };

/// Event-local hardware processing key. Hits are routed before Station
/// Coincidence, matching the validated Floating simulation flow.
struct FloatingHitGroupKey {
  using Tuple = std::tuple<std::uint16_t, std::uint16_t, std::uint16_t>;

  std::uint16_t subDetectorId{0};
  std::uint16_t triggerSector{0};
  std::uint16_t bcTag{0};

  Tuple tie() const;
  bool operator<(const FloatingHitGroupKey& other) const;
};

struct FloatingHit {
  std::uint16_t subDetectorId{0};
  std::uint16_t triggerSector{0};
  std::uint16_t detectorSector{0};
  std::uint16_t bcTag{0};
  std::uint16_t sswId{0};
  std::uint16_t slbId{0};
  std::uint16_t readoutChannel{0};
  std::int16_t stationEta{0};
  std::uint16_t stationPhi{0};
  std::uint8_t gasGap{0};
  std::uint16_t channel{0};
  FloatingStation station{FloatingStation::Unknown};
  bool isStrip{false};
  float eta{0.F};
  float phi{0.F};
  float r{0.F};
  float z{0.F};
};

using FloatingHitContainer = std::vector<FloatingHit>;
using FloatingHitGroups = std::map<FloatingHitGroupKey, FloatingHitContainer>;

struct FloatingDecodeStatistics {
  std::size_t nRawData{0};
  std::size_t nHits{0};
  std::size_t nWireHits{0};
  std::size_t nStripHits{0};
  std::size_t nMappingFailures{0};
  std::size_t nUnknownStation{0};
  std::size_t nM1Hits{0};
  std::size_t nM2Hits{0};
  std::size_t nM3Hits{0};
  std::size_t nInnerHits{0};
};

struct FloatingStationCoincidence {
  FloatingStationCoincidence(const FloatingHitGroupKey& key, FloatingStation station, bool isStrip,
                     std::uint16_t detectorSector, std::int16_t stationEta,
                     std::uint16_t stationPhi, std::uint16_t channel,
                     std::uint8_t layerMask, std::uint8_t observedLayers,
                     std::uint8_t nominalLayers, float eta, float phi,
                     float r, float z)
      : key{key},
        station{station},
        isStrip{isStrip},
        detectorSector{detectorSector},
        stationEta{stationEta},
        stationPhi{stationPhi},
        channel{channel},
        layerMask{layerMask},
        observedLayers{observedLayers},
        nominalLayers{nominalLayers},
        eta{eta},
        phi{phi},
        r{r},
        z{z} {}

  FloatingHitGroupKey key{};
  FloatingStation station{FloatingStation::Unknown};
  bool isStrip{false};
  /// Representative Run-3/chamber provenance; not a matching requirement.
  std::uint16_t detectorSector{0};
  std::int16_t stationEta{0};
  std::uint16_t stationPhi{0};
  std::uint16_t channel{0};
  std::uint8_t layerMask{0};
  std::uint8_t observedLayers{0};
  std::uint8_t nominalLayers{0};
  float eta{0.F};
  float phi{0.F};
  float r{0.F};
  float z{0.F};
};

using FloatingStationCoincidenceContainer = std::vector<FloatingStationCoincidence>;

}  // namespace L1Muon::L1TGC

#endif
