/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FloatingRdoDecoder.h"

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

L1Muon::L1TGC::FloatingStation station(
    const Identifier& identifier, const Muon::IMuonIdHelperSvc& idHelperSvc) {
  using L1Muon::L1TGC::FloatingStation;
  using Muon::MuonStationIndex::PhiIndex;

  switch (idHelperSvc.phiIndex(identifier)) {
    case PhiIndex::T1:
      return FloatingStation::M1;
    case PhiIndex::T2:
      return FloatingStation::M2;
    case PhiIndex::T3:
      return FloatingStation::M3;
    case PhiIndex::T4:
      return FloatingStation::Inner;
    default:
      return FloatingStation::Unknown;
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

namespace L1Muon::L1TGC {

StatusCode FloatingRdoDecoder::decode(const TgcRdoContainer& rdos,
                              const Muon::TgcCablingMap& cabling,
                              const Muon::IMuonIdHelperSvc& idHelperSvc,
                              const MuonGM::MuonDetectorManager& detectorManager,
                              FloatingHitGroups& hitGroups,
                              FloatingDecodeStatistics& statistics) const {
  hitGroups.clear();
  statistics = FloatingDecodeStatistics{};

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

      const FloatingStation hitStation = station(identifier, idHelperSvc);
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

      const FloatingHit hit{
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
      const FloatingHitGroupKey key{.subDetectorId = hit.subDetectorId,
                            .triggerSector = hit.triggerSector,
                            .bcTag = hit.bcTag};
      hitGroups[key].emplace_back(hit);

      ++statistics.nHits;
      if (hitIsStrip) ++statistics.nStripHits;
      else ++statistics.nWireHits;
      switch (hitStation) {
        case FloatingStation::M1:
          ++statistics.nM1Hits;
          break;
        case FloatingStation::M2:
          ++statistics.nM2Hits;
          break;
        case FloatingStation::M3:
          ++statistics.nM3Hits;
          break;
        case FloatingStation::Inner:
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

}  // namespace L1Muon::L1TGC
