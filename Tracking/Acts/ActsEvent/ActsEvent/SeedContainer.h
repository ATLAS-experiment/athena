/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_SEEDCONTAINER_H
#define ACTSTRKEVENT_SEEDCONTAINER_H 1

#include "Acts/EventData/SeedContainer2.hpp"
#include "xAODInDetMeasurement/SpacePoint.h"
#include "Acts/EventData/Seed.hpp"  // only needed for conversion from Acts::Seed

#include <vector>
#include <algorithm>
#include <utility>

namespace ActsTrk {
  struct Seed;
  struct SeedContainer : public Acts::SeedContainer2 {
    using Base = Acts::SeedContainer2;
    using value_type = Seed;
    using const_iterator = Acts::detail::ContainerIterator<SeedContainer, value_type, Index, true>;
    using SpacePointValue = const xAOD::SpacePoint*;
    using SpacePointContainer = std::vector<SpacePointValue>;

    const_iterator begin() const noexcept { return const_iterator(*this, 0); }
    const_iterator end() const noexcept { return const_iterator(*this, size()); }
    Seed at(Index index) const;
    Seed operator[](Index index) const noexcept;

    const SpacePointContainer& spacePoints() const noexcept { return m_spacePoints; }
    SpacePointContainer& spacePoints() noexcept { return m_spacePoints; }

    // various push_back() styles

    Acts::MutableSeedProxy2 push_back(Acts::SpacePointIndexSubset2 sp) {
      Acts::MutableSeedProxy2 seed = createSeed();
      seed.assignSpacePointIndices(sp);
      return seed;
    }

    Acts::MutableSeedProxy2 push_back(Acts::MutableSeedProxy2 seed) {
      Acts::MutableSeedProxy2 newseed = push_back(seed.spacePointIndices());
      newseed.vertexZ() = seed.vertexZ();
      newseed.quality() = seed.quality();
      return newseed;
    }

    // convert from old Acts::Seed<Acts::SpacePointProxy> used by SeedingTool and OrthogonalSeedingTool
    template <typename SpacePointProxy_t, std::size_t N>
    Acts::MutableSeedProxy2 push_back(const Acts::Seed<SpacePointProxy_t, N>* pSeed) {
      return push_back_helper(pSeed, [this](const SpacePointProxy_t* sp) -> Acts::SpacePointIndex2 {
        assert(sp->index() < m_spacePoints.size());
        assert(m_spacePoints[sp->index()] == &sp->externalSpacePoint());
        std::ignore = m_spacePoints.size();  // Silence Clang warning about unused lambda capture
        return sp->index();
      });
    }

    // convert from old Acts::Seed<xAOD::SpacePoint> used by GbtsSeedingTool.
    Acts::MutableSeedProxy2 push_back(const Acts::Seed<xAOD::SpacePoint, 3ul>* pSeed) {
      return push_back_helper(pSeed, [this](SpacePointValue sp) -> Acts::SpacePointIndex2 {
        auto it = std::find(m_spacePoints.begin(), m_spacePoints.end(), sp);
        if (it == m_spacePoints.end()) {
          m_spacePoints.push_back(sp);
          return m_spacePoints.size() - 1ul;
        } else {
          return std::distance(m_spacePoints.begin(), it);
        }
      });
    }

  private:
    // helper to do push_back() for different old-style seed types
    template <typename seed_type, typename spindex_fun>
    Acts::MutableSeedProxy2 push_back_helper(seed_type* pSeed, spindex_fun spindex) {
      std::vector<Acts::SpacePointIndex2> spacePointIndices;
      spacePointIndices.reserve(pSeed->sp().size());
      for (auto sp2 : pSeed->sp()) {
        Acts::SpacePointIndex2 ind = spindex(sp2);
        spacePointIndices.push_back(ind);
      }
      Acts::MutableSeedProxy2 seed = createSeed();
      seed.assignSpacePointIndices(spacePointIndices);
      seed.vertexZ() = pSeed->z();
      seed.quality() = pSeed->seedQuality();
      return seed;
    }

    SpacePointContainer m_spacePoints;
  };

  // Following classes are proxies based on SeedContainer, so don't need a CLID
  struct SpacePointIndexSubset;
  struct Seed : public Acts::ConstSeedProxy2 {
    using Base = Acts::ConstSeedProxy2;
    using SpacePointValue = SeedContainer::SpacePointValue;
    using SpacePointContainer = SeedContainer::SpacePointContainer;

    const SpacePointContainer& spacePointContainer() const noexcept { return static_cast<const SeedContainer&>(container()).spacePoints(); }
    SpacePointIndexSubset sp() const;
    // emulate old Acts::Seed methods
    float z() const noexcept { return vertexZ(); }
    float seedQuality() const noexcept { return quality(); }
  };

  inline Seed SeedContainer::at(Index index) const { return static_cast<Seed>(Base::at(index)); }
  inline Seed SeedContainer::operator[](Index index) const noexcept { return static_cast<Seed>(Base::operator[](index)); }

  struct SpacePointIndexSubset : public Acts::SpacePointIndexSubset2 {
    using Base = Acts::SpacePointIndexSubset2;
    using value_type = SeedContainer::SpacePointValue;
    using SpacePointContainer = SeedContainer::SpacePointContainer;
    using Index = Base::size_type;
    using const_iterator = Acts::detail::ContainerIterator<SpacePointIndexSubset, value_type, Index, true>;

    SpacePointIndexSubset() = default;
    SpacePointIndexSubset(const SpacePointContainer& spacePointContainer, Base spacePointIndices)
        : Base(spacePointIndices), m_spacePointContainer(&spacePointContainer) {}

    const_iterator begin() const noexcept { return const_iterator(*this, 0); }
    const_iterator end() const noexcept { return const_iterator(*this, size()); }
    value_type operator[](Index index) const noexcept { return (*m_spacePointContainer)[Base::operator[](index)]; }
    value_type at(Index index) const { return m_spacePointContainer->at(Base::operator[](index)); }
    value_type front() const noexcept { return operator[](0ul); }
    value_type back() const noexcept { return operator[](size() - 1ul); }

  private:
    const SpacePointContainer* m_spacePointContainer{nullptr};
  };

  inline SpacePointIndexSubset Seed::sp() const { return SpacePointIndexSubset(spacePointContainer(), spacePointIndices()); }

}  // namespace ActsTrk

// Set up a CLID for the type:
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::SeedContainer, 1261318102, 2)

#endif
