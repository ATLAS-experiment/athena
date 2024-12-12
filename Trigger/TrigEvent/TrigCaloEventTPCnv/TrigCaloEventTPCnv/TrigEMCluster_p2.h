/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigCaloEventTPCnv
 * @class  : TrigEMCluster_p1
 *
 * @brief persistent partner for TrigEMCluster
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <f.bucci@cern.ch>          - U. Geneva
 **********************************************************************************/
#ifndef TRIGCALOEVENTTPCNV_TRIGEMCLUSTER_P2_H
#define TRIGCALOEVENTTPCNV_TRIGEMCLUSTER_P2_H

#include "AthenaPoolUtilities/TPObjRef.h"

//need this for MAXSIZE, NUMEMSAMP, and NUMHADSAMP
#include "TrigCaloEvent/TrigEMCluster.h"
#include "DataModelAthenaPool/ElementLinkVector_p1.h"

class TrigEMCluster_p2 {
  friend class TrigEMClusterCnv;
  template <class T>
  friend class TrigEMClusterConverterBase;
    
 public:
    
  // default constructor
  TrigEMCluster_p2() = default;

  // default destructor
  ~TrigEMCluster_p2() = default;

 private:

  float m_Energy{};
  float m_Et{};
  float m_EnergyS[MAXSIZE]{};
  float m_Eta{-99};
  float m_Phi{-99};
  float m_e237{};
  float m_e277{999999};
  float m_fracs1{};
  float m_weta2{};
  float m_ehad1{};
  float m_Eta1{-99};
  float m_emaxs1{};
  float m_e2tsts1{};

  //the TrigCaloCluster base class
  TPObjRef m_trigCaloCluster;

  ElementLinkInt_p1 m_rings;

};

#endif
