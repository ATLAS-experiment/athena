/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0RdoDecoder.h"

#include "MuonRDO/TgcRawData.h"
#include "MuonRDO/TgcRdo.h"
#include "MuonRDO/TgcRdoContainer.h"

namespace {

L0Muon::TgcL0Floating::Station station(const TgcRawData::SlbType type) {
  using L0Muon::TgcL0Floating::Station;
  switch (type) {
    case TgcRawData::SLB_TYPE_TRIPLET_WIRE:
    case TgcRawData::SLB_TYPE_TRIPLET_STRIP:
      return Station::M1;
    case TgcRawData::SLB_TYPE_DOUBLET_WIRE:
    case TgcRawData::SLB_TYPE_DOUBLET_STRIP:
      return Station::M2M3;
    case TgcRawData::SLB_TYPE_INNER_WIRE:
    case TgcRawData::SLB_TYPE_INNER_STRIP:
      return Station::Inner;
    default:
      return Station::Unknown;
  }
}

bool isStrip(const TgcRawData::SlbType type) {
  return type == TgcRawData::SLB_TYPE_DOUBLET_STRIP ||
         type == TgcRawData::SLB_TYPE_TRIPLET_STRIP ||
         type == TgcRawData::SLB_TYPE_INNER_STRIP;
}

}  // namespace

namespace L0Muon {
namespace TgcL0Floating {

StatusCode RdoDecoder::decode(const TgcRdoContainer& rdos,
                              HitGroups& hitGroups,
                              DecodeStatistics& statistics) const {
  hitGroups.clear();
  statistics = DecodeStatistics{};

  // The decoder assumes the Run-3 ROD, SSW and SLB-based TGC RDO format.
  // The Run-4 TGC RDO will have a substantially different structure, and
  // this decoder and its hit organization are expected to require a broad
  // rewrite when that format becomes available. The HitGroups output is the
  // boundary to the later reconstruction, so the downstream segment,
  // candidate, Inner Coincidence and Track Selector code is not expected to
  // require corresponding changes.
  for (const TgcRdo* rdo : rdos) {
    for (const TgcRawData* rawData : *rdo) {
      ++statistics.nRawData;
      if (rawData->type() != TgcRawData::TYPE_HIT) {
        continue;
      }

      const Station hitStation = station(rawData->slbType());
      const bool hitIsStrip = isStrip(rawData->slbType());
      const Hit hit{
          .subDetectorId = rawData->subDetectorId(),
          .detectorSector = rawData->rodId(),
          .bcTag = rawData->bcTag(),
          .sswId = rawData->sswId(),
          .slbId = rawData->slbId(),
          .channel = rawData->channel(),
          .station = hitStation,
          .isStrip = hitIsStrip,
      };
      const HitGroupKey key{
          .subDetectorId = hit.subDetectorId,
          .detectorSector = hit.detectorSector,
          .bcTag = hit.bcTag,
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
        case Station::M2M3:
          ++statistics.nM2M3Hits;
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
