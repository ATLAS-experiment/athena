/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigCaloEventTPCnv
 * @class  : TrigT2Jet_p2
 *
 * @brief persistent partner for TrigT2Jet
 *
 * @author Tobias Kruker    <kruker@cern.ch>					 - U. Bern
 **********************************************************************************/
#ifndef TRIGCALOEVENTTPCNV_TRIGT2JET_P2_H
#define TRIGCALOEVENTTPCNV_TRIGT2JET_P2_H

class TrigT2Jet_p2 
{
  friend class TrigT2JetCnv_p2;	// tobias: uncomment if necessary
 public:
  
  TrigT2Jet_p2() = default;
  virtual ~TrigT2Jet_p2() = default;
  
 private:
  
  float m_allTheFloats[9]{};
  //float m_e;
  //float m_ehad0;
  //float m_eem0;
  //float m_eta;
  //float m_phi  ;
  //float m_coneRadius  ;
  long m_roiWord{};
  // cleaning:
  int m_nLeadingCells{999};
  //float m_cleaningFloats[4];
  // m_cleaningFloats[0] = m_hecf, m_cleaningFloats[1] = m_jetQuality
  // m_cleaningFloats[2] = m_emf, m_cleaningFloats[3] =  m_jetTimeCells
};

#endif
