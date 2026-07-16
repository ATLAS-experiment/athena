/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_SEEDCONTAINER_H
#define ACTSTRKEVENT_SEEDCONTAINER_H 1

#include <algorithm>
#include <utility>
#include <vector>

#include "Acts/EventData/SeedContainer.hpp"
#include "xAODInDetMeasurement/SpacePoint.h"

namespace ActsTrk {

// This is similar to Acts::SeedContainer but stores pointers to
// xAOD::SpacePoints instead of indices into an Acts::SpacePointContainer.
//
// Having a separate container is beneficial as there is not too much overlap
// with Acts::SeedContainer and the implementation is rather simple. This
// decouples us from the implementation of Acts::SeedContainer and allows us to
// link directly to the xAOD::SpacePoints without needing to convert back and
// forth between indices and pointers.

// hack extension of std::span with necessary at() method for seed parameter
// estimation
struct SpacePointRange final : public std::span<const xAOD::SpacePoint* const> {
  using Base = std::span<const xAOD::SpacePoint* const>;

  using Base::Base;

  const xAOD::SpacePoint* at(std::size_t index) const {
    if (index >= size()) {
      throw std::out_of_range("SpacePointRange index out of range");
    }
    return Base::operator[](index);
  }
};

struct SeedContainer;

struct Seed final {
  using Index = Acts::SeedIndex;

  Seed(const SeedContainer& container, Index index)
      : m_container(&container), m_index(index) {}

  const SeedContainer& container() const noexcept { return *m_container; }
  Index index() const noexcept { return m_index; }

  SpacePointRange spacePoints() const noexcept;
  float quality() const noexcept;
  float vertexZ() const noexcept;

  // emulate old Acts::Seed methods
  SpacePointRange sp() const noexcept { return spacePoints(); }
  float z() const noexcept { return vertexZ(); }
  float seedQuality() const noexcept { return quality(); }

 private:
  const SeedContainer* m_container{nullptr};
  Index m_index{0};
};

struct SeedContainer final {
  using Index = Acts::SeedIndex;
  using value_type = Seed;

  std::size_t size() const noexcept { return m_size; }
  bool empty() const noexcept { return size() == 0; }
  void reserve(std::size_t size, float averageSpacePoints = 3) noexcept {
    m_spacePointOffsets.reserve(size);
    m_spacePointCounts.reserve(size);
    m_qualities.reserve(size);
    m_vertexZs.reserve(size);
    m_spacePoints.reserve(static_cast<std::size_t>(size * averageSpacePoints));
  }
  void clear() noexcept {
    m_size = 0;
    m_spacePointOffsets.clear();
    m_spacePointCounts.clear();
    m_qualities.clear();
    m_vertexZs.clear();
    m_spacePoints.clear();
  }

  Seed operator[](Index index) const noexcept { return Seed(*this, index); }
  Seed at(Index index) const {
    if (index >= size()) {
      throw std::out_of_range("SeedContainer index out of range");
    }
    return Seed(*this, index);
  }

  SpacePointRange spacePoints(Index index) const noexcept {
    const std::uint32_t offset = m_spacePointOffsets[index];
    const std::uint8_t count = m_spacePointCounts[index];
    return SpacePointRange(m_spacePoints.data() + offset, count);
  }
  float quality(Index index) const noexcept { return m_qualities[index]; }
  float vertexZ(Index index) const noexcept { return m_vertexZs[index]; }

  using const_iterator =
      Acts::detail::ContainerIterator<SeedContainer, Seed, Index, true>;

  const_iterator begin() const noexcept { return const_iterator(*this, 0); }
  const_iterator end() const noexcept { return const_iterator(*this, size()); }

  // various push_back() styles

  Seed push_back(SpacePointRange spacePoints, float quality, float vertexZ) {
    push_back_(spacePoints.size(), quality, vertexZ);
    m_spacePoints.insert(m_spacePoints.end(), spacePoints.begin(),
                         spacePoints.end());
    return at(m_size++);
  }

  template <typename arbitrary_sp_range_t, typename xaod_sp_ptr_projector_t>
  Seed push_back(const arbitrary_sp_range_t& arbitrarySpacePoints,
                 const xaod_sp_ptr_projector_t& xAODspProjector, float quality,
                 float vertexZ) {
    push_back_(arbitrarySpacePoints.size(), quality, vertexZ);
    std::ranges::copy(
        std::views::transform(arbitrarySpacePoints, xAODspProjector),
        std::back_inserter(m_spacePoints));
    return at(m_size++);
  }

  Seed push_back(SpacePointRange spacePoints,
                 const Acts::ConstSeedProxy& seed) {
    return push_back(spacePoints, seed.quality(), seed.vertexZ());
  }

  template <typename xaod_sp_ptr_projector_t>
  Seed push_back(const Acts::ConstSeedProxy& seed,
                 const xaod_sp_ptr_projector_t& xAODspProjector) {
    return push_back(seed.spacePointIndices(), xAODspProjector, seed);
  }

  template <typename arbitrary_sp_range_t, typename xaod_sp_ptr_projector_t>
  Seed push_back(const arbitrary_sp_range_t& arbitrarySpacePoints,
                 const xaod_sp_ptr_projector_t& xAODspProjector,
                 const Acts::ConstSeedProxy& seed) {
    return push_back(arbitrarySpacePoints, xAODspProjector, seed.quality(),
                     seed.vertexZ());
  }

 private:
  std::uint32_t m_size{0};
  std::vector<std::uint32_t> m_spacePointOffsets;
  std::vector<std::uint8_t> m_spacePointCounts;
  std::vector<float> m_qualities;
  std::vector<float> m_vertexZs;

  std::vector<const xAOD::SpacePoint*> m_spacePoints;

  void push_back_(std::size_t nSpacePoints, float quality, float vertexZ) {
    const std::uint32_t offset =
        static_cast<std::uint32_t>(m_spacePoints.size());
    const std::uint8_t count = static_cast<std::uint8_t>(nSpacePoints);

    m_spacePointOffsets.push_back(offset);
    m_spacePointCounts.push_back(count);
    m_qualities.push_back(quality);
    m_vertexZs.push_back(vertexZ);
  }
};

inline SpacePointRange Seed::spacePoints() const noexcept {
  return container().spacePoints(m_index);
}
inline float Seed::quality() const noexcept {
  return container().quality(m_index);
}
inline float Seed::vertexZ() const noexcept {
  return container().vertexZ(m_index);
}

}  // namespace ActsTrk

// Set up a CLID for the type:
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::SeedContainer, 1261318102, 2)

#endif
