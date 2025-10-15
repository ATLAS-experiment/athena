/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_TRACKCONTAINER_H
#define ACTSTRKEVENT_TRACKCONTAINER_H 1

#include "AthLinks/tools/DefaultIndexingPolicy.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"

namespace ActsTrk {
  using TrackBackend = Acts::ConstVectorTrackContainer;
  using MutableTrackBackend = Acts::VectorTrackContainer;
  using TrackStateBackend = Acts::ConstVectorMultiTrajectory;
  using MutableTrackStateBackend = Acts::VectorMultiTrajectory;
  
  // Containers without xAOD backends - this is for transient objects
  using TrackContainerBase = Acts::TrackContainer<TrackBackend,
                                                  TrackStateBackend,
                                                  Acts::detail::ValueHolder>;
  
  using MutableTrackContainer = Acts::TrackContainer<MutableTrackBackend,
                                                     MutableTrackStateBackend,
                                                     Acts::detail::ValueHolder>;


  class TrackContainer
    : public TrackContainerBase {
  public:
    using TrackContainerBase::TrackContainerBase;
    using value_type = typename TrackContainerBase::ConstTrackProxy;
    
    ConstTrackProxy operator[](unsigned int index) const { return getTrack(index); }
    bool empty() const { return size() == 0; }
  };


  // Special indexing policy which will dereference element links
  // into an std::optional rather than a pointer or reference to an
  // existing element in the destination collection. This is
  // needed because the Acts track container does have physical
  // representations of tracks, but only creats proxy objects
  // for tracks which are created on demand.
  template <typename track_container_t>
  class IndexingPolicy {
  public:
    /// The type of the const track proxy
    using ConstTrackProxy = typename track_container_t::ConstTrackProxy;
    /// The type we get when we dereference a link, and derived types.
    using ElementType = std::optional<ConstTrackProxy>;
    
    struct ConstTrackProxyPtr {
      ConstTrackProxyPtr(const ElementType *src) {
        if (src) {
          m_proxy = *src;
        }
      }
      ConstTrackProxyPtr(const ConstTrackProxyPtr &) = default;
      ConstTrackProxyPtr(ConstTrackProxyPtr &&) = default;
      ConstTrackProxyPtr(const ConstTrackProxy &val) :m_proxy(val) {}
      ConstTrackProxyPtr(ConstTrackProxy &&val) : m_proxy(std::move(val)) {}
      
      ConstTrackProxy operator*() const {
        return m_proxy.value();
      }
      const ConstTrackProxy *operator->() const {
        return &m_proxy.value();
      }
      bool operator!() const {
        return !m_proxy.has_value();
      }
      ConstTrackProxyPtr &operator=(const ElementType *src) {
        if (src) {
          m_proxy = *src;
        }
        else {
          m_proxy.reset();
        }
        return *this;
      }
      
      std::optional<ConstTrackProxy> m_proxy {std::nullopt};
    };

    using ElementConstReference = std::optional<ConstTrackProxy>;
    using ElementConstPointer = ConstTrackProxyPtr;
    
    /// The type of an index, as provided to or returned from a link.
    using index_type = track_container_t::IndexType;
    
    /// The type of an index, as stored internally within a link.
    using stored_index_type = index_type;
    
    static bool isValid (const stored_index_type& index) {
      return index != ConstTrackProxy::kInvalid;
    }
    
    static index_type storedToExternal (stored_index_type index) {
      return index;
    }
    
    static void reset (stored_index_type& index) {
      index=ConstTrackProxy::kInvalid;
    }
    
    static
    ElementType lookup(const stored_index_type& index, const track_container_t& container) {
      return container.getTrack(index);
    }
    
    static void
    reverseLookup([[maybe_unused]] const track_container_t& container,
                  ElementConstReference element,
                  index_type& index) {
      index = element.has_value() ? element.value().index() : ConstTrackProxy::kInvalid;
    }
  };

}  // namespace ActsTrk

// register special indexing policy for element links to Acts tracks i.e.
// ElementLink<ActsTrk::TrackContainer>
template <>
struct DefaultIndexingPolicy < ActsTrk::TrackContainer  > {
   using type = typename ActsTrk::IndexingPolicy< ActsTrk::TrackContainer >;
};

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::TrackContainer, 1210898253, 1)

#endif
