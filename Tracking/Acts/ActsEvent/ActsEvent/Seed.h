/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_SEED_H
#define ACTSTRKEVENT_SEED_H 1

#include "xAODInDetMeasurement/SpacePoint.h"
#include <boost/container/small_vector.hpp>

namespace ActsTrk {

// Acts::Seed (renamed to ActsSeed) definition copied from Acts/EventData/Seed.hpp
// modified to allow variable number of SPs.
// Otherwise minimise API changes for N=3.

template <typename external_spacepoint_t, std::size_t N = 3ul>
class ActsSeed {
  // static_assert(N >= 3ul);

 public:
  using value_type = external_spacepoint_t;
  using container_type = boost::container::small_vector<const external_spacepoint_t*, N>;
  static constexpr std::size_t DIM = N;

  template <typename... args_t>
    requires(sizeof...(args_t) == N) &&
            (std::same_as<external_spacepoint_t, args_t> && ...)
  explicit ActsSeed(const args_t&... points);

  template <typename arg_t>
    requires(N != 1)
  explicit ActsSeed(const arg_t& points);

  void setVertexZ(float vertex);
  void setQuality(float seedQuality);

  const container_type& sp() const;
  float z() const;
  float seedQuality() const;

 private:
  container_type m_spacepoints{};
  float m_vertexZ{0.f};
  float m_seedQuality{-std::numeric_limits<float>::infinity()};
};

template <typename external_spacepoint_t, std::size_t N>
template <typename... args_t>
  requires(sizeof...(args_t) == N) &&
          (std::same_as<external_spacepoint_t, args_t> && ...)
ActsSeed<external_spacepoint_t, N>::ActsSeed(const args_t&... points)
    : m_spacepoints({&points...}) {}

template <typename external_spacepoint_t, std::size_t N>
template <typename arg_t>
  requires(N != 1)
ActsSeed<external_spacepoint_t, N>::ActsSeed(const arg_t& points)
    : m_spacepoints(points) {}

template <typename external_spacepoint_t, std::size_t N>
void ActsSeed<external_spacepoint_t, N>::setVertexZ(float vertex) {
  m_vertexZ = vertex;
}

template <typename external_spacepoint_t, std::size_t N>
void ActsSeed<external_spacepoint_t, N>::setQuality(float seedQuality) {
  m_seedQuality = seedQuality;
}

template <typename external_spacepoint_t, std::size_t N>
const ActsSeed<external_spacepoint_t, N>::container_type&
ActsSeed<external_spacepoint_t, N>::sp() const {
  return m_spacepoints;
}

template <typename external_spacepoint_t, std::size_t N>
float ActsSeed<external_spacepoint_t, N>::z() const {
  return m_vertexZ;
}

template <typename external_spacepoint_t, std::size_t N>
float ActsSeed<external_spacepoint_t, N>::seedQuality() const {
  return m_seedQuality;
}

typedef ActsSeed<xAOD::SpacePoint, 3ul> Seed;

}

// Set up a CLID for the type:
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::Seed, 207128231, 1)

#endif
