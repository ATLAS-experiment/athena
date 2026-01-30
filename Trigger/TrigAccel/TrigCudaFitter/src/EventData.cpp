// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#include <stdlib.h>
#include "McTrack.h"
#include "RecTrack.h"
#include "EventData.h"

EventData::~EventData()
{
  for(std::vector<const RecTrack*>::iterator it=m_tracks.begin(); it!=m_tracks.end();++it)
    delete (*it);
}

void EventData::setEventNumber(int n)
{
  m_eventNumber=n;
}

void EventData::setMcTrack(std::unique_ptr<const McTrack> ptrack)
{
  m_mcTrack = std::move(ptrack);
}

void EventData::addRecTrack(const RecTrack* ptrack)
{
  m_tracks.push_back(ptrack);
}
