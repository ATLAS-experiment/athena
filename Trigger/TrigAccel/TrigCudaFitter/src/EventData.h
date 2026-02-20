// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef __EVENTDATA_H__
#define __EVENTDATA_H__

#include <memory>
#include <vector>

class RecTrack;
class McTrack;

class EventData
{
  public:
    EventData() = default;
    ~EventData();

    void setEventNumber(int n);
    void setMcTrack(std::unique_ptr<const McTrack> ptrack);
    void addRecTrack(const RecTrack* ptrack);

    std::vector<const RecTrack*> m_tracks;
    std::unique_ptr<const McTrack> m_mcTrack;
    int m_eventNumber{0};
};

#endif
