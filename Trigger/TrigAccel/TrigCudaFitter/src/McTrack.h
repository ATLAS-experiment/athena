// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef __MCTRACK_H__
#define __MCTRACK_H__

#include <vector>

class McBremPoint;

class McTrack
{
  public:
    McTrack(float[6]);
    virtual ~McTrack(void);

    double m_params[5];
    double m_totalEloss;
    std::vector<McBremPoint> m_bremPoints;

  public:
    void addBremPoint(float x, float y, float z, float loss);
};

#endif
