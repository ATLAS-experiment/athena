// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef __DKFTRACK_H__
#define __DKFTRACK_H__

#include <memory>
#include <vector>

class TrkTrackState;
class TrkBaseNode;
class TrkPlanarSurface;
class RecTrack;

class DkfTrack
{
  public:
    DkfTrack(std::unique_ptr<TrkTrackState>,const RecTrack*);
    virtual ~DkfTrack(void);

    std::unique_ptr<TrkTrackState> m_pTrackState;
    std::vector<TrkBaseNode*> m_vpNodes;
    const RecTrack* m_pRecTrack;
    std::vector<const TrkPlanarSurface*> m_vpSurfaces;
    std::vector<TrkTrackState*> m_vpTrackStates;

  private:
    double m_dChi2;
    int m_nNDOF;

  public:
    double getChi2(void);
    void addChi2(double,int);
    int getNDOF(void);
};

#endif
