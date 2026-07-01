/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_INTERFACES_KNOWN_SG_KEYS_H
#define COLUMNAR_INTERFACES_KNOWN_SG_KEYS_H

#include <SGCore/sgkey_t.h>

#include <string>
#include <unordered_map>

namespace columnar
{
  /// @brief lookup table from container name to its sgkey hash
  ///
  /// This is shared between the columnar test fixtures and the
  /// columnar helper tools, so that both use the same source of truth
  /// when translating a raw sgkey read from an xAOD file into the
  /// compact key used by the columnar variant-link representation.
  ///
  /// The keys are calculated as crc64(name) extended with the container
  /// CLID and masked to 30 bits (see @ref computeSgKey, which
  /// reproduces @ref SG::StringPool::stringToKey). For sole-target link
  /// columns python computes the keys from @ref
  /// ColumnInfo::soleLinkTargetClid. This table is used in situations
  /// in which the container type (and with it the CLID) is not readily
  /// available.
  inline const std::unordered_map<std::string,SG::sgkey_t> knownSgKeys =
  {
    {"AnalysisMuons", 0x3a6b126f},
    {"AnalysisElectrons", 0x3902fec0},
    {"AnalysisPhotons", 0x35d1472f},
    {"AnalysisJets", 0x1afd1919},
    {"egammaClusters", 0x15788d1f},
    {"GSFConversionVertices", 0x1f3e85c9},
    {"InDetTrackParticles", 0x1d3890db},
    {"CombinedMuonTrackParticles", 0x340d9196},
    {"ExtrapolatedMuonTrackParticles", 0x14e35e9f},
    {"GSFTrackParticles", 0x2e42db0b},
    {"InDetForwardTrackParticles", 0x143c6846},
    {"MuonSpectrometerTrackParticles", 0x3993c8f3},
  };
}

#endif
