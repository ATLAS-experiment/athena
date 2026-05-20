/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_SEEDCONTAINER_H
#define ACTSTRKEVENT_SEEDCONTAINER_H 1

#include <algorithm>
#include <utility>
#include <vector>
#include <variant>
#include <cassert>

#include "Acts/EventData/SeedContainer2.hpp"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/SpacePoint.h"
#include "AthLinks/DataLink.h"

namespace ActsTrk {

// This is similar to Acts::SeedContainer2 but stores pointers to
// xAOD::SpacePoints instead of indices into an Acts::SpacePointContainer2.
//
// Having a separate container is beneficial as there is not too much overlap
// with Acts::SeedContainer2 and the implementation is rather simple. This
// decouples us from the implementation of Acts::SeedContainer2 and allows us to
// link directly to the xAOD::SpacePoints without needing to convert back and
// forth between indices and pointers.

// hack extension of std::span with necessary at() method for seed parameter
// estimation

struct SeedContainer;

struct SpacePointProxy {
   const SeedContainer *container;
   const unsigned int index;
   using ConstVoidPtr=const void *;

   mutable ConstVoidPtr elementPtrCache ATLAS_THREAD_SAFE = nullptr;
   std::span<const xAOD::UncalibratedMeasurement * const> measurements() const;
   xAOD::ConstVectorMap<3> globalPosition() const;
   unsigned int spacePointIndex() const { return index; }
   float x() const;
   float y() const;
   float z() const;
   std::optional<float> t() const;
   const SpacePointProxy *operator->() const { return this; }

};
// to support Acts::estimateTrackParamsFromSeed
inline bool operator==(const SpacePointProxy &a, const void *b) {
   return b!=nullptr || a.index == std::numeric_limits<unsigned int>::max();
}

struct SpacePointRange {
   //protected:
  friend class SeedContainer;
  using const_value_type = SpacePointProxy;
   
  const SeedContainer *m_container;
  std::span<const unsigned int> m_indices;
   //public:
  std::size_t size() const { return m_indices.size(); }
  struct const_iterator {
     const_iterator &operator++() {
        ++indexIter;
        return *this;
     }
     SpacePointProxy operator*() {
        assert( container );
        return SpacePointProxy{container,*indexIter};
     }
     const SeedContainer *container;
     std::span<const unsigned int>::iterator indexIter;
  };
  const_iterator begin() const {
     return const_iterator{m_container, m_indices.begin()};
  }
  const_iterator end() const {
     return const_iterator{m_container, m_indices.end()};
  }
  SpacePointProxy front() const {
     assert( m_container );
     assert(!m_indices.empty());
     return *begin();
  }
  SpacePointProxy back() const {
     assert(!m_indices.empty());
     assert( m_container );
     return SpacePointProxy{m_container,m_indices.back()};
  }
  SpacePointProxy operator[](std::size_t index) const {
     assert( index < m_indices.size() );
     assert( m_container );
     return SpacePointProxy{m_container,m_indices[index]};
  }
  SpacePointProxy at(std::size_t index) const {
    if (index >= size()) {
      throw std::out_of_range("SpacePointRange index out of range");
    }
    return operator[](index);
  }
};
inline bool operator!=(const SpacePointRange::const_iterator &a, const SpacePointRange::const_iterator &b) {
   assert(a.container==b.container);
   return a.indexIter != b.indexIter;
}
   
struct Seed final {
  using Index = Acts::SeedIndex2;

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
  friend struct SpacePointProxy;
  using Index = Acts::SeedIndex2;
  using value_type = Seed;
  using SpacePointVariant = std::variant<const xAOD::SpacePoint * const,
                                         const xAOD::PixelCluster * const>;

  std::size_t size() const noexcept { return m_size; }
  bool empty() const noexcept { return size() == 0; }
  void reserve(std::size_t size, float averageSpacePoints = 3) noexcept {
    m_spacePointOffsets.reserve(size);
    m_spacePointCounts.reserve(size);
    m_qualities.reserve(size);
    m_vertexZs.reserve(size);
    m_constituentIndex.reserve(static_cast<std::size_t>(size * averageSpacePoints));
  }
  void clear() noexcept {
    m_size = 0;
    m_spacePointOffsets.clear();
    m_spacePointCounts.clear();
    m_qualities.clear();
    m_vertexZs.clear();
    m_constituentIndex.clear();
    m_srcContainer.clear();
    m_srcContainerIndexOffset.clear();
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
    return SpacePointRange{ this, std::span<const unsigned int>(m_constituentIndex.data()+offset, static_cast<std::size_t>(count)) };
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
    for (SpacePointProxy proxy : spacePoints) {
       m_constituentIndex.push_back(proxy.spacePointIndex());
    }
    return at(m_size++);
  }

  template <typename arbitrary_sp_range_t, typename xaod_sp_ptr_projector_t>
  Seed push_back(const arbitrary_sp_range_t& arbitrarySpacePoints,
                 const xaod_sp_ptr_projector_t& xAODspProjector, float quality,
                 float vertexZ) {
    push_back_(arbitrarySpacePoints.size(), quality, vertexZ);
    std::ranges::copy(
        std::views::transform(arbitrarySpacePoints, xAODspProjector),
        std::back_inserter(m_constituentIndex));
    return at(m_size++);
  }

  Seed push_back(SpacePointRange spacePoints,
                 const Acts::ConstSeedProxy2& seed) {
    return push_back(spacePoints, seed.quality(), seed.vertexZ());
  }

  template <typename xaod_sp_ptr_projector_t>
  Seed push_back(const Acts::ConstSeedProxy2& seed,
                 const xaod_sp_ptr_projector_t& xAODspProjector) {
    return push_back(seed.spacePointIndices(), xAODspProjector, seed);
  }

  template <typename arbitrary_sp_range_t, typename xaod_sp_ptr_projector_t>
  Seed push_back(const arbitrary_sp_range_t& arbitrarySpacePoints,
                 const xaod_sp_ptr_projector_t& xAODspProjector,
                 const Acts::ConstSeedProxy2& seed) {
    return push_back(arbitrarySpacePoints, xAODspProjector, seed.quality(),
                     seed.vertexZ());
  }

  // @return intex offset for the elements of this container
  template <typename T_Container>
  unsigned int addSourceContainer(const EventContext &ctx, const T_Container &src_container) {
     m_srcContainer.emplace_back(DataLink<T_Container>(&src_container, ctx));
     if (m_srcContainerIndexOffset.empty()) {
        m_srcContainerIndexOffset.push_back(0u);
     }
     unsigned int element_offset_for_container = m_srcContainerIndexOffset.back();
     m_srcContainerIndexOffset.push_back(element_offset_for_container + src_container.size());
     return element_offset_for_container;
  }

  /// @param index a valid index
  /// @return a variant containint the space point associated to the given index
  /// @note the result is undefined if the index is outside the allowed range.
  SpacePointVariant getSpacePointVariant(unsigned int index) const;
 protected:
  using SrcContainerVariant = std::variant<DataLink<xAOD::SpacePointContainer>,
                                           DataLink<xAOD::PixelClusterContainer> >;
  std::pair<const SrcContainerVariant *,unsigned int> getSrcContainer(unsigned int index) const;
  std::span<const xAOD::UncalibratedMeasurement * const> measurementsOfSpacePoint(unsigned int index, SpacePointProxy::ConstVoidPtr &element_ptr_cache) const;
  xAOD::ConstVectorMap<3> spacePointGlobalPosition(unsigned int index) const;
  inline std::optional<float> spacePointTime(unsigned int index) const;

   
   
 private:
  std::uint32_t m_size{0};
  std::vector<std::uint32_t> m_spacePointOffsets;
  std::vector<std::uint8_t> m_spacePointCounts;
  std::vector<float> m_qualities;
  std::vector<float> m_vertexZs;

  std::vector<unsigned int> m_constituentIndex;
  std::vector<SrcContainerVariant> m_srcContainer;
  std::vector<unsigned int> m_srcContainerIndexOffset;

  void push_back_(std::size_t nSpacePoints, float quality, float vertexZ) {
    const std::uint32_t offset =
        static_cast<std::uint32_t>(m_constituentIndex.size());
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

#include "SeedContainer.icc"
// Set up a CLID for the type:
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::SeedContainer, 1261318102, 2)

#endif
