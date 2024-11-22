/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOEVENT_TRIGINDETTRACKSEEDINGRESULT_H
#define TRIGINDETPATTRECOEVENT_TRIGINDETTRACKSEEDINGRESULT_H

struct TrigInDetTrackSeedingResult {

TrigInDetTrackSeedingResult() : m_nPixelSPs(0), m_nStripSPs(0), m_nGraphEdges(0), m_nEdgeLinks(0) {};
  
  int m_nPixelSPs, m_nStripSPs, m_nGraphEdges, m_nEdgeLinks;

};

#endif
