/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigCaloEventTPCnv
 * @class  : TrigTauCluster_p1
 *
 * @brief persistent partner for TrigTauCluster
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <f.bucci@cern.ch>          - U. Geneva
 **********************************************************************************/
#ifndef TRIGCALOEVENTTPCNV_TRIGTAUCLUSTER_P1_H
#define TRIGCALOEVENTTPCNV_TRIGTAUCLUSTER_P1_H

#include "AthenaPoolUtilities/TPObjRef.h"

//need this for MAXSIZE, NUMEMSAMP, and NUMHADSAMP
#include "TrigCaloEvent/TrigTauCluster.h"
#include "CxxUtils/unused.h"

class TrigTauCluster_p1
{
 public:
  
  TrigTauCluster_p1() = default;
  friend class TrigTauClusterCnv_p1;
  
 private:
  
  // Some of these are unused, but we shouldn't delete them
  // since they're part of the persistent data.
  float m_EMenergy{};
  float m_HADenergy{};
  float ATH_UNUSED_MEMBER(m_eEMCalib){};
  float m_eCalib{};
  float ATH_UNUSED_MEMBER(m_EnergyS[MAXSIZE]){};
  float m_Eta{-999};
  float m_Phi{-999};
  float m_EMRadius[NUMEMSAMP]{};
  float m_IsoFrac{};
  float m_EMenergyWidth[NUMEMSAMP]{};
  float ATH_UNUSED_MEMBER(m_HADenergyWidth[NUMHADSAMP]){};
  int    m_numStripCells{};
  float m_stripWidth{};
  
  float ATH_UNUSED_MEMBER(m_EMenergyNor[NUMEMSAMP]){};
  float ATH_UNUSED_MEMBER(m_EMenergyWid[NUMEMSAMP]){};
  float ATH_UNUSED_MEMBER(m_EMenergyNar[NUMEMSAMP]){};
  
  float ATH_UNUSED_MEMBER(m_HADenergyNor[NUMHADSAMP]){};
  float ATH_UNUSED_MEMBER( m_HADenergyWid[NUMHADSAMP]){};
  float ATH_UNUSED_MEMBER(m_HADenergyNar[NUMHADSAMP]){};
  
  //the TrigCaloCluster base class
  TPObjRef m_trigCaloCluster;

};

#endif
