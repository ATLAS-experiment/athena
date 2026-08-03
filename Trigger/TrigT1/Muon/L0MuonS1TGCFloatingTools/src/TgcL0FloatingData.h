/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGDATA_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGDATA_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <tuple>
#include <vector>

namespace L0Muon {
namespace TgcL0Floating {

enum class Station : std::uint8_t { M1, M2, M3, Inner, Unknown, NumberOfStations };

/// Event-local hardware processing key. Hits are routed before Station
/// Coincidence, matching the validated Floating simulation flow.
struct HitGroupKey {
  using Tuple = std::tuple<std::uint16_t, std::uint16_t, std::uint16_t>;

  std::uint16_t subDetectorId{0};
  std::uint16_t triggerSector{0};
  std::uint16_t bcTag{0};

  Tuple tie() const;
  bool operator<(const HitGroupKey& other) const;
};

struct Hit {
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
  Station station{Station::Unknown};
  bool isStrip{false};
  float eta{0.F};
  float phi{0.F};
  float r{0.F};
  float z{0.F};
};

using HitContainer = std::vector<Hit>;
using HitGroups = std::map<HitGroupKey, HitContainer>;

struct DecodeStatistics {
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

struct StationCoincidence {
  StationCoincidence(const HitGroupKey& key, Station station, bool isStrip,
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

  HitGroupKey key{};
  Station station{Station::Unknown};
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

using StationCoincidenceContainer = std::vector<StationCoincidence>;

}  // namespace TgcL0Floating
}  // namespace L0Muon

#endif
