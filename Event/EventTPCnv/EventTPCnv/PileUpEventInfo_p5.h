/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENTTPCNV_PILEUPEVENTINFO_P5_H
#define EVENTTPCNV_PILEUPEVENTINFO_P5_H
/**
 * @file PileUpEventInfo_p5.h
 *
 * @brief This class is the persistent representation of PileUpEventInfo_p5
 *
 * @author ilija@vukotic.me
 */

#include "EventTPCnv/EventInfo_p4.h"
#include <vector>

class MsgStream;

class PileUpEventInfo_p5 : public EventInfo_p4 {
public:

  PileUpEventInfo_p5() {};
  virtual ~PileUpEventInfo_p5() {};
        
  struct SubEvent {
    short		m_time = 0;
    short		m_index = 0;
    short		m_type = 0;
    EventInfo_p4	m_subEventInfo; 
  };

  typedef std::vector<SubEvent>	SubEvVect_t;
  SubEvVect_t		m_subEvents; 
};


#endif


