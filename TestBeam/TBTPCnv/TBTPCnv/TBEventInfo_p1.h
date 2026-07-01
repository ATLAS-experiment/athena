/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// -------------------------------------------------------------------
// Persistent representation of TBEvent/TBEventInfo
// See: https://twiki.cern.ch/twiki/bin/view/Atlas/TransientPersistentSeparation#TP_converters_for_component_type
// Author: Iftach Sadeh (iftach.sadeh@NOSPAMTODAYcern.ch) , February 2010
// -------------------------------------------------------------------
#ifndef TBEVENTINFO_P1_H
#define TBEVENTINFO_P1_H


class TBEventInfo_p1
{

public:

  int m_ev_number = 0;
  int m_ev_clock = 0;
  int m_ev_type = 0;
  unsigned int m_run_num = 0;
  float m_beam_moment = 0;
  std::string m_beam_part;
  float m_cryoX = 0;
  float m_cryoAngle = 0;
  float m_tableY = 0;
    
};


#endif
