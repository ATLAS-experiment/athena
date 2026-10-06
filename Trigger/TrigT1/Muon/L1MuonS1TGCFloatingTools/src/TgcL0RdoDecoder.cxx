/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0RdoDecoder.h"

#include "FourMomUtils/xAODP4Helpers.h"
#include "Identifier/Identifier.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/TgcRawData.h"
#include "MuonRDO/TgcRdo.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"

#include <cmath>
#include <numbers>

namespace {

L1Muon::TgcL0Floating::Station station(
    const Identifier& identifier, const Muon::IMuonIdHelperSvc& idHelperSvc) {
  using L1Muon::TgcL0Floating::Station;
  using Muon::MuonStationIndex::PhiIndex;

  switch (idHelperSvc.phiIndex(identifier)) {
    case PhiIndex::T1:
      return Station::M1;
    case PhiIndex::T2:
      return Station::M2;
    case PhiIndex::T3:
      return Station::M3;
    case PhiIndex::T4:
      return Station::Inner;
    default:
      return Station::Unknown;
  }
}

std::uint16_t triggerSector(const float phi) {
  if (!std::isfinite(phi)) return 0U;
  constexpr std::uint16_t nSectors = 24U;
  const float fullTurn = 2.F * std::numbers::pi_v<float>;
  const float sectorWidth = fullTurn / static_cast<float>(nSectors);
  float wrapped = static_cast<float>(xAOD::P4Helpers::deltaPhi(phi, 0.));
  if (wrapped < 0.F) wrapped += fullTurn;
  std::uint16_t sector = static_cast<std::uint16_t>(
      std::floor(wrapped / sectorWidth)) + 2U;
  if (sector > nSectors) sector -= nSectors;
  return sector;
}

}  // namespace

namespace L1Muon {
namespace TgcL0Floating {

StatusCode RdoDecoder::decode(const TgcRdoContainer& rdos,
                              const Muon::TgcCablingMap& cabling,
                              const Muon::IMuonIdHelperSvc& idHelperSvc,
                              const MuonGM::MuonDetectorManager& detectorManager,
                              HitGroups& hitGroups,
                              DecodeStatistics& statistics) const {
  hitGroups.clear();
  statistics = DecodeStatistics{};

  // Run-3 RDO conversion is isolated here. Hits are routed immediately to the
  // Phase-II side/Trigger-Sector/BC processing chain, as in the validated
  // Floating simulation. Chamber identifiers remain hit provenance only.
  for (const TgcRdo* rdo : rdos) {
    for (const TgcRawData* rawData : *rdo) {
      ++statistics.nRawData;
      if (rawData->type() != TgcRawData::TYPE_HIT) continue;

      Identifier identifier;
      const bool mapped = cabling.getOfflineIDfromReadoutID(
          identifier, rawData->subDetectorId(), rawData->rodId(),
          rawData->sswId(), rawData->slbId(), rawData->channel());
      if (!mapped) {
        ++statistics.nMappingFailures;
        continue;
      }

      const Station hitStation = station(identifier, idHelperSvc);
      const bool hitIsStrip = idHelperSvc.tgcIdHelper().isStrip(identifier);
      float eta = 0.F;
      float phi = 0.F;
      float r = 0.F;
      float z = 0.F;
      const MuonGM::TgcReadoutElement* readoutElement =
          detectorManager.getTgcReadoutElement(identifier);
      if (readoutElement != nullptr) {
        const Amg::Vector3D globalPosition = readoutElement->channelPos(identifier);
        eta = static_cast<float>(globalPosition.eta());
        phi = static_cast<float>(globalPosition.phi());
        r = static_cast<float>(globalPosition.perp());
        z = static_cast<float>(globalPosition.z());
      }
      const std::uint16_t hitTriggerSector = triggerSector(phi);
      if (hitTriggerSector == 0U) {
        ++statistics.nMappingFailures;
        continue;
      }

      const Hit hit{
          .subDetectorId = rawData->subDetectorId(),
          .triggerSector = hitTriggerSector,
          .detectorSector = rawData->rodId(),
          .bcTag = rawData->bcTag(),
          .sswId = rawData->sswId(),
          .slbId = rawData->slbId(),
          .readoutChannel = rawData->channel(),
          .stationEta = static_cast<std::int16_t>(
              idHelperSvc.tgcIdHelper().stationEta(identifier)),
          .stationPhi = static_cast<std::uint16_t>(
              idHelperSvc.tgcIdHelper().stationPhi(identifier)),
          .gasGap = static_cast<std::uint8_t>(
              idHelperSvc.tgcIdHelper().gasGap(identifier)),
          .channel = static_cast<std::uint16_t>(
              idHelperSvc.tgcIdHelper().channel(identifier)),
          .station = hitStation,
          .isStrip = hitIsStrip,
          .eta = eta,
          .phi = phi,
          .r = r,
          .z = z,
      };
      const HitGroupKey key{.subDetectorId = hit.subDetectorId,
                            .triggerSector = hit.triggerSector,
                            .bcTag = hit.bcTag};
      hitGroups[key].emplace_back(hit);

      ++statistics.nHits;
      if (hitIsStrip) ++statistics.nStripHits;
      else ++statistics.nWireHits;
      switch (hitStation) {
        case Station::M1:
          ++statistics.nM1Hits;
          break;
        case Station::M2:
          ++statistics.nM2Hits;
          break;
        case Station::M3:
          ++statistics.nM3Hits;
          break;
        case Station::Inner:
          ++statistics.nInnerHits;
          break;
        default:
          ++statistics.nUnknownStation;
          break;
      }
    }
  }
  return StatusCode::SUCCESS;
}

}  // namespace TgcL0Floating
}  // namespace L1Muon
