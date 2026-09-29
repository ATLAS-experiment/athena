// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ACTSTRKEVENT_GEOMETRYIDMAPPING_H
#define ACTSTRKEVENT_GEOMETRYIDMAPPING_H

#include "Identifier/Identifier.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>

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
    m_detrayToActs.reserve(nSurfaces);
    m_detrayToAthena.reserve(nSurfaces);
    m_actsToDetray.reserve(nSurfaces);
    m_athenaToDetray.reserve(nSurfaces);
  }

  /// Register one detray surface, it has to have a corresponding ACTS surface 
  /// athenaId is optional, unset for surfaces that couldn't be matched to an Athena module
  /// (e.g. passive surfaces)
  void addEntry(detray_id_type detrayId, acts_id_type actsId,
                std::optional<Identifier> athenaId = std::nullopt) {
    

    m_detrayToActs.emplace(detrayId,actsId);
    m_actsToDetray.emplace(actsId, detrayId);

    if (athenaId.has_value()) {
      m_detrayToAthena.emplace(detrayId,athenaId->get_compact());
      m_athenaToDetray.emplace(athenaId->get_compact(), detrayId);
    }
  }

  void addDetDescIndex(detray_id_type detray_id, unsigned int index) {
    m_detrayToDetDescIndex[detray_id] = index;
  }

  /// @name detray_id <-> acts_id
  ///@{
  std::optional<acts_id_type> detrayToActs(detray_id_type detrayId) const {
    auto it = m_detrayToActs.find(detrayId);
    return it == m_detrayToActs.end() ? std::nullopt
                                       : std::optional{it->second};
  }
  std::optional<detray_id_type> actsToDetray(acts_id_type actsId) const {
    auto it = m_actsToDetray.find(actsId);
    return it == m_actsToDetray.end() ? std::nullopt
                                       : std::optional{it->second};
  }
  ///@}

  /// @name detray_id <-> Athena Identifier
  ///@{
  std::optional<Identifier> detrayToAthena(detray_id_type detrayId) const {
    auto it = m_detrayToAthena.find(detrayId);
    if (it == m_detrayToAthena.end()) return std::nullopt;
    return Identifier(it->second);
  }
  std::optional<detray_id_type> athenaToDetray(const Identifier& athenaId) const {
    auto it = m_athenaToDetray.find(athenaId.get_compact());
    return it == m_athenaToDetray.end() ? std::nullopt
                                         : std::optional{it->second};
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
  const std::unordered_map<detray_id_type, Identifier::value_type>&
    detrayToAthenaMap() const { return m_detrayToAthena; }

  std::optional<size_t> detrayToDetDescIndex(detray_id_type detray_id) const {
    auto it = m_detrayToDetDescIndex.find(detray_id);
    return it == m_detrayToDetDescIndex.end() ? std::nullopt : std::optional(it->second);
  }

 private:
  std::unordered_map<detray_id_type, acts_id_type> m_detrayToActs;
  std::unordered_map<detray_id_type, Identifier::value_type> m_detrayToAthena;

  std::unordered_map<acts_id_type, detray_id_type> m_actsToDetray;
  std::unordered_map<Identifier::value_type, detray_id_type> m_athenaToDetray;

  std::unordered_map<detray_id_type, size_t> m_detrayToDetDescIndex;
};

}  // namespace ActsTrk

// Needed to record/retrieve this via StoreGate
#include "AthenaKernel/CLASS_DEF.h"

CLASS_DEF(ActsTrk::GeometryIdMapping, 263041249, 1)

#endif  // ACTSTRKEVENT_GEOMETRYIDMAPPING_H