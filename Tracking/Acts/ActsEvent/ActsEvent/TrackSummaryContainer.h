/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSEVENT_TRACKSUMMARYCONTAINER_H
#define ACTSEVENT_TRACKSUMMARYCONTAINER_H
#include <type_traits>

#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "ActsEvent/Decoration.h"
#include "xAODTracking/TrackSummaryContainer.h"
#include "xAODTracking/TrackSummaryAuxContainer.h"
#include "xAODTracking/TrackSurfaceAuxContainer.h"
#include "xAODTracking/TrackSurfaceContainer.h"
#include "ActsEvent/SurfaceEncoding.h"

namespace ActsTrk {
class MutableTrackSummaryContainer;
class TrackSummaryContainer;
}  // namespace ActsTrk

namespace Acts {
class Surface;
template <typename T>
struct IsReadOnlyTrackContainer {};

template <typename T>
struct IsReadOnlyTrackContainer<T&> : IsReadOnlyTrackContainer<T> {};

template <typename T>
struct IsReadOnlyTrackContainer<T&&> : IsReadOnlyTrackContainer<T> {};

template <>
struct IsReadOnlyTrackContainer<ActsTrk::TrackSummaryContainer>
    : std::true_type {};

template <>
struct IsReadOnlyTrackContainer<ActsTrk::MutableTrackSummaryContainer>
    : std::false_type {};

    
}  // namespace Acts

namespace ActsTrk {

using ConstParameters = Acts::TrackStateTraits<3>::Parameters;
using ConstCovariance = Acts::TrackStateTraits<3>::Covariance;
using Parameters = Acts::TrackStateTraits<3, false>::Parameters;
using Covariance = Acts::TrackStateTraits<3, false>::Covariance;

class MutableTrackSummaryContainer;

class TrackSummaryContainer {
 public:
  using IndexType = uint32_t; // TODO find common place for it
  static constexpr auto kInvalid = Acts::MultiTrajectoryTraits::kInvalid;
  TrackSummaryContainer(const DataLink<xAOD::TrackSummaryContainer>& lin = nullptr,
                        const DataLink<xAOD::TrackSurfaceAuxContainer>& surfLink = nullptr);
  static const std::set<std::string> staticVariables;
  /**
  * return true if the container has specific decoration
  */
  constexpr bool hasColumn_impl(Acts::HashedString key) const;

  /**
  * return pointer to reference surface
  */
  const Acts::Surface* referenceSurface_impl(ActsTrk::IndexType itrack) const;

  /**
  * return pointer to reference surface
  */
  Acts::ParticleHypothesis particleHypothesis_impl(IndexType itrack) const;

  /**
  * returns number of stored tracks
  */
  std::size_t size_impl() const;

  /**
  * access to components by pointer with type
  */
  std::any component_impl(Acts::HashedString key,
                          ActsTrk::IndexType itrack) const;

  /**
  * parameters of the track
  */
  ActsTrk::ConstParameters parameters(ActsTrk::IndexType itrack) const;

  /**
  * covariance of the track fit
  */
  ActsTrk::ConstCovariance covariance(ActsTrk::IndexType itrack) const;
  
  /**
  * surface
  */
  std::shared_ptr<const Acts::Surface> surface(ActsTrk::IndexType itrack) const;

  void fillFrom(ActsTrk::MutableTrackSummaryContainer& mtb);

  template<typename T>
  friend class MutableTrackContainerHandle;
  friend class MutableTrackSummaryContainer;

  void restoreDecorations();

  const xAOD::TrackSummaryContainer* trackBackend() const{
    return m_trackBackend.cptr();
  }

 protected:

  DataLink<xAOD::TrackSummaryContainer> m_trackBackend = nullptr;
  DataLink<xAOD::TrackSurfaceAuxContainer> m_surfBackendAux = nullptr;

  std::vector<ActsTrk::detail::Decoration> m_decorations;

  std::vector<std::shared_ptr<const Acts::Surface>> m_surfaces;
  std::vector<Acts::ParticleHypothesis> m_particleHypothesis; // TODO move the storage to the backend
};

class MutableTrackSummaryContainer : public TrackSummaryContainer {
 public:
  MutableTrackSummaryContainer();
  MutableTrackSummaryContainer(const MutableTrackSummaryContainer&) = delete;
  MutableTrackSummaryContainer operator=(const MutableTrackSummaryContainer&) = delete;
  MutableTrackSummaryContainer(MutableTrackSummaryContainer&&);
  MutableTrackSummaryContainer& operator=(MutableTrackSummaryContainer&& other) noexcept;
  
 
  /**
  * adds new surface to the tail of the container
  */
  ActsTrk::IndexType addSurface_impl();

  /**
  * clears surface data under index
  */
  void removeSurface_impl(ActsTrk::IndexType isurf);

  /**
  * adds new track to the tail of the container
  */
  ActsTrk::IndexType addTrack_impl();

  /**
  * clears track data under index
  */
  void removeTrack_impl(ActsTrk::IndexType itrack);

  /**
  * enables the container to support decoration of given name and type
  */
  template <typename T>
  constexpr void addColumn_impl(const std::string& key);

  /**
  * copies decorations from other container
  */
  void copyDynamicFrom_impl (ActsTrk::IndexType itrack,
                             const ActsTrk::TrackSummaryContainer& other,
                             ActsTrk::IndexType other_itrack);


  /**
  * write access to decorations
  */
  std::any component_impl(Acts::HashedString key,
                          ActsTrk::IndexType itrack);
  using TrackSummaryContainer::component_impl;

  /**
  * write access to parameters
  */
  ActsTrk::Parameters parameters(ActsTrk::IndexType itrack);
  using TrackSummaryContainer::parameters;

  /**
  * write access to covariance
  */
  ActsTrk::Covariance covariance(ActsTrk::IndexType itrack);
  using TrackSummaryContainer::covariance;

  /**
  * synchronizes decorations
  */
  void ensureDynamicColumns_impl(const MutableTrackSummaryContainer& other);
  void ensureDynamicColumns_impl(const TrackSummaryContainer& other);

  /**
  * preallocate number of track objects 
  */
  void reserve(ActsTrk::IndexType size);

  /** 
  * zeroes container
  */
  void clear();

  /**
  * point given track to surface
  * The surface ownership is shared
  */
  void setReferenceSurface_impl(ActsTrk::IndexType itrack,
                                std::shared_ptr<const Acts::Surface> surface);
  /**
  * sets particle hypothesis
  * @warning it will fail for an arbitrary particles as it converts to 
  * a predefined set (@see xAOD::ParticleHypothesis in TrackingPrimitives.h) of values
  */
  void setParticleHypothesis_impl(ActsTrk::IndexType itrack, 
                                  const Acts::ParticleHypothesis& particleHypothesis);

  template<typename T>
  friend class MutableTrackContainerHandle;


  xAOD::TrackSummaryContainer* trackBackend(){
    return m_mutableTrackBackend.get();
  }

  xAOD::TrackSurfaceContainer* surfBackend(){
    return m_mutableSurfBackend.get();
  }

 private:
  std::unique_ptr<xAOD::TrackSummaryContainer> m_mutableTrackBackend;
  std::unique_ptr<xAOD::TrackSummaryAuxContainer> m_mutableTrackBackendAux;

  std::unique_ptr<xAOD::TrackSurfaceContainer> m_mutableSurfBackend;
  std::unique_ptr<xAOD::TrackSurfaceAuxContainer> m_mutableSurfBackendAux;
};


constexpr bool ActsTrk::TrackSummaryContainer::hasColumn_impl(
    Acts::HashedString key) const {
  using namespace Acts::HashedStringLiteral;
  switch (key) {
    case "params"_hash:
    case "cov"_hash:
    case "nMeasurements"_hash:
    case "nHoles"_hash:
    case "d0"_hash:
    case "chi2"_hash:
    case "ndf"_hash:
    case "nOutliers"_hash:
    case "hSharedHits"_hash:
    case "tipIndex"_hash:
    case "stemIndex"_hash:

      return true;
  }
  for (auto& d : m_decorations) {
    if (d.hash == key) {
      return true;
    }
  }
  return false;
}

namespace details{
} // EOF detail

template <typename T>
constexpr void MutableTrackSummaryContainer::addColumn_impl(
    const std::string& name) {
  if (not ActsTrk::detail::accepted_decoration_types<T>::value) {
    throw std::runtime_error(
        "TrackSummaryContainer::addColumn_impl: "
        "unsupported decoration type");
  }
  m_decorations.emplace_back(ActsTrk::detail::decoration<T>(
      name,
      ActsTrk::detail::constDecorationGetter<T>, 
      ActsTrk::detail::decorationCopier<T>,
      ActsTrk::detail::decorationSetter<T>
      ));
}

}  // namespace ActsTrk

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::TrackSummaryContainer, 1185802350, 1 )
#endif
