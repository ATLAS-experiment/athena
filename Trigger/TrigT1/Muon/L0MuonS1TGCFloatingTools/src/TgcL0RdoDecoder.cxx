/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0RdoDecoder.h"

#include "Identifier/Identifier.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/TgcRawData.h"
#include "MuonRDO/TgcRdo.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"

#include <string>

namespace {

L0Muon::TgcL0Floating::Station station(
    const Identifier& identifier, const Muon::IMuonIdHelperSvc& idHelperSvc) {
  using L0Muon::TgcL0Floating::Station;
  const std::string stationName = idHelperSvc.tgcIdHelper().stationNameString(
      idHelperSvc.tgcIdHelper().stationName(identifier));
  if (stationName.rfind("T1", 0) == 0) {
    return Station::M1;
  }
  if (stationName.rfind("T2", 0) == 0) {
    return Station::M2;
  }
  if (stationName.rfind("T3", 0) == 0) {
    return Station::M3;
  }
  if (stationName.rfind("T4", 0) == 0) {
    return Station::Inner;
  }
  return Station::Unknown;
}

}  // namespace

namespace L0Muon {
namespace TgcL0Floating {

StatusCode RdoDecoder::decode(const TgcRdoContainer& rdos,
                              const Muon::TgcCablingMap& cabling,
                              const Muon::IMuonIdHelperSvc& idHelperSvc,
                              HitGroups& hitGroups,
                              DecodeStatistics& statistics) const {
  hitGroups.clear();
  statistics = DecodeStatistics{};

  // This decoder assumes the Run-3 ROD, SSW and SLB-based TGC RDO format.
  // The Run-4 TGC RDO will have a substantially different structure, and
  // this decoder and its hit organization will require a broad rewrite when
  // that format becomes available. The HitGroups output is the boundary to
  // the later reconstruction, so the downstream Station Coincidence,
  // segment, candidate, Inner Coincidence and Track Selector code is not
  // expected to require corresponding changes.
  for (const TgcRdo* rdo : rdos) {
    for (const TgcRawData* rawData : *rdo) {
      ++statistics.nRawData;
      if (rawData->type() != TgcRawData::TYPE_HIT) {
        continue;
      }

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
      const Hit hit{
          .subDetectorId = rawData->subDetectorId(),
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
      };
      const HitGroupKey key{
          .subDetectorId = hit.subDetectorId,
          .detectorSector = hit.detectorSector,
          .bcTag = hit.bcTag,
          .stationEta = hit.stationEta,
          .stationPhi = hit.stationPhi,
          .station = hit.station,
          .isStrip = hit.isStrip,
      };
      hitGroups[key].push_back(hit);

      ++statistics.nHits;
      if (hitIsStrip) {
        ++statistics.nStripHits;
      } else {
        ++statistics.nWireHits;
      }
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
}  // namespace L0Muon
