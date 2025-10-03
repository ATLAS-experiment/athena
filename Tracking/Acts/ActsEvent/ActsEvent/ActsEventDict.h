/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSEVENT_DICT_H
#define ACTSEVENT_DICT_H

#include "AthLinks/ElementLink.h"

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/PersistentTrackContainer.h"

namespace {
  struct GCCXML_DUMMY_ELACTSTRK_TRACKCONTAINER {
     ActsTrk::TrackContainer              m_dest;
     ActsTrk::PersistentTrackContainer m_persdest;
     ElementLink<ActsTrk::TrackContainer> m_linkToDest;
     std::vector<ElementLink<ActsTrk::TrackContainer> > m_two;
  };
}

#endif
