/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigParticleTPCnv
 * @class  : TrigEFBphys_p1
 *
 * @brief persistent partner for TrigEFBphys
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <F.Bucci@cern.ch>          - U. Geneva
 **********************************************************************************/
#ifndef TRIGPARTICLETPCNV_TRIGEFBPHYS_P1_H
#define TRIGPARTICLETPCNV_TRIGEFBPHYS_P1_H

#include "AthenaPoolUtilities/TPObjRef.h"

class TrigEFBphys_p1 
{
  friend class TrigEFBphysCnv_p1;

 public:
  
  TrigEFBphys_p1() = default;
  virtual ~TrigEFBphys_p1() = default;
  
  enum pType_p1{PHIKK=0, DSPHIPI=1, BMUMU=2, BMUMUX=3, JPSIEE=4, JPSIMUMU=5, MULTIMU=6, BKMUMU=7, BDKSTMUMU=8, BSPHIMUMU=9, LBLMUMU=10, BCDSMUMU=11};
  
  int   m_roiID{};
  pType_p1   m_particleType{};
  float m_eta{};
  float m_phi{};
  float m_mass{};
  bool  m_valid{};
  TPObjRef  m_secondaryDecay;

};

#endif
