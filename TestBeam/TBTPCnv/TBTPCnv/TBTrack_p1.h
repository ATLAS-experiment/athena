/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// -------------------------------------------------------------------
// Persistent representation of TBEvent/TBTrack
// See: https://twiki.cern.ch/twiki/bin/view/Atlas/TransientPersistentSeparation#TP_converters_for_component_type
// Author: Iftach Sadeh (iftach.sadeh@NOSPAMTODAYcern.ch) , February 2010
// -------------------------------------------------------------------
#ifndef TBTRACK_P1_H
#define TBTRACK_P1_H


class TBTrack_p1
{

public:

  // number of hits used for reconstruction
  int m_hitNumberU = 0, m_hitNumberV = 0;
  
  // Residuals between detector hit and fitted hit (in detector local coordonates)
  std::vector<double> m_residualu, m_residualv;

  // track parameters
  double m_chi2 = 0, m_chi2u = 0, m_chi2v = 0;
  double m_angle = 0;               // angle between track and Z TestBeam axe
  double m_uslope = 0,m_vslope = 0; // slopes with respect to Z TestBeam axe
  double m_uintercept = 0, m_vintercept = 0;
  
  // extrapoled hit position at cryostat (reconstruction coordonates);
  double m_cryou = 0,m_cryov = 0,m_cryow = 0;    
};


#endif
