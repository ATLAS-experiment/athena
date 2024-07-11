// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#pragma once
#include<vector>

class RecTrack;
struct TrackInfoStruct;

class TrackSlimmer
{
  public:
    TrackSlimmer(void);
    ~TrackSlimmer(void);

    void processTracks(std::vector<const RecTrack*>&);

  private:
    std::vector<struct TrackInfoStruct*> m_vpTIS;
};
