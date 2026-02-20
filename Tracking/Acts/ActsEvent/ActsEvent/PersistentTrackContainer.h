/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_PERSISTENTTRACKCONTAINER_H
#define ACTSTRKEVENT_PERSISTENTTRACKCONTAINER_H

#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackSummaryContainer.h"
#include "Acts/EventData/TrackContainer.hpp"

namespace ActsTrk {
  // Containers with xAOD backends - this is for persistification
  using MutablePersistentTrackBackend = ActsTrk::MutableTrackSummaryContainer;
  using PersistentTrackBackend = ActsTrk::TrackSummaryContainer;
  using MutablePersistentTrackStateBackend = ActsTrk::MutableMultiTrajectory;
  using PersistentTrackStateBackend = ActsTrk::MultiTrajectory;

  template <typename T>
  struct DataLinkHolder {
    using element_type = T;
    DataLink<T> m_link;
    DataLinkHolder(const DataLink<T>& link) : m_link{link} {}
    
    const T& operator*() const { return *(m_link.cptr()); }
    const T* operator->() const { return m_link.cptr(); }
    T& operator*() { return *(m_link.ptr()); }
    T* operator->() { return m_link.ptr(); }

    operator bool() const { return m_link.isValid(); }
  };

  using PersistentTrackContainerBase = Acts::TrackContainer<ActsTrk::PersistentTrackBackend,
                                                            ActsTrk::PersistentTrackStateBackend,
                                                            ActsTrk::DataLinkHolder>;
  
  class PersistentTrackContainer :
    public PersistentTrackContainerBase
  {
  public:
    using PersistentTrackContainerBase::PersistentTrackContainerBase;
    using value_type = ConstTrackProxy;
    ConstTrackProxy operator[](unsigned int index) const {
      return getTrack(index);
    }
    bool empty() const {
      return size() == 0;
    }    
  };
  
  struct MutablePersistentTrackContainer
    : public Acts::TrackContainer<ActsTrk::MutablePersistentTrackBackend,
                                  ActsTrk::MutablePersistentTrackStateBackend,
                                  Acts::detail::ValueHolder> {
    MutablePersistentTrackContainer()
      : Acts::TrackContainer<ActsTrk::MutablePersistentTrackBackend,
                             ActsTrk::MutablePersistentTrackStateBackend,
                             Acts::detail::ValueHolder>(
       MutablePersistentTrackBackend(),
       MutablePersistentTrackStateBackend()) {}
  };
  
}  // namespace ActsTrk

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::PersistentTrackContainer, 1316311468, 1)

#endif
