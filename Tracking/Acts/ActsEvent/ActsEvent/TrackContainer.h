/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_TRACKCONTAINER_H
#define ACTSTRKEVENT_TRACKCONTAINER_H 1

#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackSummaryContainer.h"

namespace ActsTrk {
using MutableTrackBackend = ActsTrk::MutableTrackSummaryContainer;
using TrackBackend = ActsTrk::TrackSummaryContainer;
using MutableTrackStateBackend = ActsTrk::MutableMultiTrajectory;
using TrackStateBackend = ActsTrk::MultiTrajectory;

template <typename T>
struct DataLinkHolder {
  DataLink<T> m_link;
  DataLinkHolder(const DataLink<T>& link) : m_link{link} {}

  const T& operator*() const { return *(m_link.cptr()); }
  const T* operator->() const { return m_link.cptr(); }
};

using TrackContainerBase = Acts::TrackContainer<ActsTrk::TrackBackend, ActsTrk::TrackStateBackend,
                                                ActsTrk::DataLinkHolder>;

class  TrackContainer :
   public  TrackContainerBase
{
public:
   using TrackContainerBase::TrackContainerBase;
   using value_type = ConstTrackProxy;
   ConstTrackProxy operator[](unsigned int index) const {
      return getTrack(index);
   }
  bool empty() const {
    return size() == 0;
  }

   // Special indexing policy which will dereference element links
   // into an std::optional rather than a pointer or reference to an
   // existing element in the destination collection. This is
   // needed because the Acts track container does have physical
   // representations of tracks, but only creats proxy objects
   // for tracks which are created on demand.
   class IndexingPolicy
   {
   public:
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

         std::optional<ConstTrackProxy> m_proxy;
      };
      using ElementConstReference = std::optional<ConstTrackProxy>;
      using ElementConstPointer = ConstTrackProxyPtr;

      /// The type of an index, as provided to or returned from a link.
      using index_type = TrackContainerBase::IndexType;

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
      ElementType lookup(const stored_index_type& index, const TrackContainerBase& container) {
         return container.getTrack(index);
      }

      static void
      reverseLookup([[maybe_unused]] const TrackContainerBase& container,
                    ElementConstReference element,
                    index_type& index) {
         index= element.has_value() ? element.value().index() : ConstTrackProxy::kInvalid;
      }
   };

};

struct MutableTrackContainer
    : public Acts::TrackContainer<ActsTrk::MutableTrackBackend,
                                  ActsTrk::MutableTrackStateBackend,
                                  Acts::detail::ValueHolder> {
  MutableTrackContainer()
      : Acts::TrackContainer<ActsTrk::MutableTrackBackend,
                             ActsTrk::MutableTrackStateBackend,
                             Acts::detail::ValueHolder>(
            MutableTrackBackend(), MutableTrackStateBackend()) {}
};

}  // namespace ActsTrk

// register special indexing policy for element links to Acts tracks i.e.
// ElementLink<ActsTrk::TrackContainer>
template <>
struct DefaultIndexingPolicy <ActsTrk::TrackContainer  > {
   using type = ActsTrk::TrackContainer::IndexingPolicy;
};


#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::TrackContainer, 1210898253, 1)
#endif
