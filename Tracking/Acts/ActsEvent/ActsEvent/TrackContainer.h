/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_TRACKCONTAINER_H
#define ACTSTRKEVENT_TRACKCONTAINER_H 1

#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackStorageContainer.h"

namespace ActsTrk {
using MutableTrackBackend = ActsTrk::MutableTrackStorageContainer;
using TrackBackend = ActsTrk::TrackStorageContainer;
using MutableTrackStateBackend = ActsTrk::MutableMultiTrajectory;
using TrackStateBackend = ActsTrk::MultiTrajectory;

template <typename T>
struct DataLinkHolder {
  DataLink<T> m_link;
  DataLinkHolder(const DataLink<T>& link) : m_link{link} {}

  const T& operator*() const { return *(m_link.cptr()); }
  const T* operator->() const { return m_link.cptr(); }
};

using TrackContainer =
    Acts::TrackContainer<ActsTrk::TrackBackend, ActsTrk::TrackStateBackend,
                         ActsTrk::DataLinkHolder>;

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

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::TrackContainer, 1210898253, 1)
#endif
