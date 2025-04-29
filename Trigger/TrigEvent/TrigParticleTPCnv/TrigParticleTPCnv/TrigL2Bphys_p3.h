/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigParticleTPCnv
 * @class  : TrigL2Bphys_p3
 *
 * @brief persistent partner for TrigL2Bphys
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <F.Bucci@cern.ch>          - U. Geneva
 **********************************************************************************/
#ifndef TRIGPARTICLETPCNV_TRIGL2BPHYS_P3_H
#define TRIGPARTICLETPCNV_TRIGL2BPHYS_P3_H

#include "DataModelAthenaPool/ElementLink_p3.h"
#include "DataModelAthenaPool/ElementLinkVector_p1.h"

class TrigL2Bphys_p3 
{
  friend class TrigL2BphysCnv_p3;

 public:
  
  TrigL2Bphys_p3() = default;
  virtual ~TrigL2Bphys_p3() = default;
  
  float m_allFloats[8]{};//m_eta,m_phi,m_mass,m_fitmass,m_fitchi2,m_fitx,m_fity,m_fitz
  int m_allInts[3]{};//m_roiID,m_particleType,m_fitndof

  ElementLinkIntVector_p1 m_trackVector;
  ElementLinkInt_p3 m_secondaryDecay;

};

#endif
