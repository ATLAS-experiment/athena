// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ACTSTRKEVENT_GEOMETRYIDMAPPING_H
#define ACTSTRKEVENT_GEOMETRYIDMAPPING_H

#include "Identifier/Identifier.h"

#include <cstdint>
#include <limits>
#include <optional>
#include <unordered_map>
#include <vector>

namespace ActsTrk {

/// @class GeometryIdMapping
///
/// Lookup table between the three surface-identifier "worlds" used across
/// the ACTS / Detray / Athena tracking geometries:
///
///  - detray_id : dense index of a surface inside the detray detector
///                
///  - acts_id   : Acts::GeometryIdentifier::value_type of the matching
///                ACTS surface.
///  - athena_id : Identifier of the matching Athena (Pixel/Strip) module
///                side. Only present for sensitive detray surfaces that
///                were successfully matched to an Athena module.
///
/// Built once in DeviceDetectorDescriptionCondAlg::initialize() from the
/// tracking geometry (geometry-tag static), and recorded to the detector
/// store.

class GeometryIdMapping {
 public:
  using detray_id_type = std::uint64_t;
  using acts_id_type = std::uint64_t;

  GeometryIdMapping() = default;

  /// Reserve storage for @p nSurfaces detray surfaces.
  void reserve(std::size_t nSurfaces) {
    m_detrayToActs.assign(nSurfaces, invalidActs());
    m_detrayToAthena.assign(nSurfaces, 0);
    m_hasAthena.assign(nSurfaces, false);
    m_actsToDetray.reserve(nSurfaces);
    m_athenaToDetray.reserve(nSurfaces);
  }

  /// Register one detray surface, it has to have a corresponding ACTS surface 
  /// athenaId is optional, unset for surfaces that couldn't be matched to an Athena module
  /// (e.g. passive surfaces)
  void addEntry(detray_id_type detrayId, acts_id_type actsId,
                std::optional<Identifier> athenaId = std::nullopt) {
    if (detrayId >= m_detrayToActs.size()) {
      m_detrayToActs.resize(detrayId + 1, invalidActs());
      m_detrayToAthena.resize(detrayId + 1, 0);
      m_hasAthena.resize(detrayId + 1, false);
    }

    m_detrayToActs[detrayId] = actsId;
    m_actsToDetray.emplace(actsId, detrayId);

    if (athenaId.has_value()) {
      m_detrayToAthena[detrayId] = athenaId->get_compact();
      m_hasAthena[detrayId] = true;
      m_athenaToDetray.emplace(athenaId->get_compact(), detrayId);
    }
  }

  /// @name detray_id <-> acts_id
  ///@{
  std::optional<acts_id_type> detrayToActs(detray_id_type detrayId) const {
    if (detrayId >= m_detrayToActs.size()) return std::nullopt;
    return m_detrayToActs[detrayId];
  }
  std::optional<detray_id_type> actsToDetray(acts_id_type actsId) const {
    auto it = m_actsToDetray.find(actsId);
    if (it == m_actsToDetray.end()) return std::nullopt;
    return it->second;
  }
  ///@}

  /// @name detray_id <-> Athena Identifier
  ///@{
  std::optional<Identifier> detrayToAthena(detray_id_type detrayId) const {
    if (detrayId >= m_hasAthena.size() || !m_hasAthena[detrayId])
      return std::nullopt;
    return Identifier(m_detrayToAthena[detrayId]);
  }
  std::optional<detray_id_type> athenaToDetray(const Identifier& athenaId) const {
    auto it = m_athenaToDetray.find(athenaId.get_compact());
    if (it == m_athenaToDetray.end()) return std::nullopt;
    return it->second;
  }
  ///@}

  std::optional<Identifier> actsToAthena(acts_id_type actsId) const {
    auto d = actsToDetray(actsId);
    return d ? detrayToAthena(*d) : std::nullopt;
  }
  std::optional<acts_id_type> athenaToActs(const Identifier& athenaId) const {
    auto d = athenaToDetray(athenaId);
    return d ? detrayToActs(*d) : std::nullopt;
  }

  std::size_t size() const { return m_detrayToActs.size(); }

 private:
  static acts_id_type invalidActs() {
    return std::numeric_limits<acts_id_type>::max();
  }

  std::vector<acts_id_type> m_detrayToActs;
  std::vector<Identifier::value_type> m_detrayToAthena;
  std::vector<bool> m_hasAthena;

  std::unordered_map<acts_id_type, detray_id_type> m_actsToDetray;
  std::unordered_map<Identifier::value_type, detray_id_type> m_athenaToDetray;
};

}  // namespace ActsTrk

// Needed to record/retrieve this via StoreGate — put the actual hash
// somewhere stable (e.g. generate with clid.db or pick one and register it).
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::GeometryIdMapping, 263041249, 1)

#endif  // ACTSTRKEVENT_GEOMETRYIDMAPPING_H