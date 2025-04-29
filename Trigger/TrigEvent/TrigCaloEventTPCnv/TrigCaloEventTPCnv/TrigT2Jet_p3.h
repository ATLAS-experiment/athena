/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigCaloEventTPCnv
 * @class  : TrigT2Jet_p3
 *
 * @brief persistent partner for TrigT2Jet
 *
 * @author Tobias Kruker    <kruker@cern.ch>					 - U. Bern
 **********************************************************************************/
#ifndef TRIGCALOEVENTTPCNV_TRIGT2JET_P3_H
#define TRIGCALOEVENTTPCNV_TRIGT2JET_P3_H

class TrigT2Jet_p3 
{
  friend class TrigT2JetCnv_p3;
 public:
  
  TrigT2Jet_p3() = default;
  virtual ~TrigT2Jet_p3() = default;
  
 private:
  
  float m_allTheFloats[9]{};
  //float m_e;
  //float m_ehad0;
  //float m_eem0;
  //float m_eta;
  //float m_phi  ;
  //float m_coneRadius  ; this variable is not being persistified anymore since _p2
  long m_roiWord{};
  // cleaning:
  int m_nLeadingCells{999};
  //float m_cleaningFloats[4];
  // m_cleaningFloats[0] = m_hecf, m_cleaningFloats[1] = m_jetQuality
  // m_cleaningFloats[2] = m_emf, m_cleaningFloats[3] =  m_jetTimeCells
};

#endif
