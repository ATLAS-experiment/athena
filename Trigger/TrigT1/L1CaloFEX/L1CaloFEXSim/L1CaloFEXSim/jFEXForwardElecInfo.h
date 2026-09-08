/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//              jFEXForwardElecInfo - Class to store information about trigger towers
//                                    in the fcal and associated jet information.
//                              -------------------
//     begin                : 12 05 2022
//     email                : ulla.blumenschein
//***************************************************************************

#ifndef jFEXForwardElecInfo_H
#define jFEXForwardElecInfo_H

#include "L1CaloFEXSim/jFEXForwardElecTOB.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace LVL1 {

class jFEXForwardElecInfo {

 public:
  
  
  // Setup with key parameters and menu info
  void setup(int jfex, uint ttid, int neta, int nphi); //used 
  void setup(int *cval, uint reso = 200);

  // Basic IDs and energies (TT level)
  uint  getCoreTTID() const;
  uint  getCoreIphi() const;
  uint  getCoreIeta() const;             //used
  void  setCoreTTEtEM(int ET_EM );//used
  int  getCoreTTEtEM() const;           //used
  void  setCoreTTSatEM(bool sat);
  bool getCoreTTSatEM() const; 
  void  setNextTTID(uint TTID );   //used  
  uint  getNextTTID() const;
  void  setNextTTEtEM(int ET_EM); //used 
  int  getNextTTEtEM() const;     //used 
  void  setNextTTSatEM(bool sat);
  bool getNextTTSatEM() const;
  void  setTTEtEMiso(int iso_ET); //used     
  void  addTTEtEMiso(int iso_ET);  //used 
  int  getTTEtEMiso() const;
  void  setTTEtHad1(int ET_HAD);  //used, EMfr1 
  void  addTTEtHad1(int ET_HAD);  //used, special SC 
  int  getTTEtHad1() const;
  void  setTTEtHad2(int ET_HAD);  //used, EMfr2 
  int  getTTEtHad2() const;
  void  calcTTClusEtEM();          //used
  int  getTTClusEtEM() const;      //used
  bool getTTClusSatEM() const;
  void  includeTTinSearchWindow(uint TT_ID);
  const std::vector<uint>& getTTinSearchWindow() const;
  // floating point values
  void  setCoreTTfEta(float feta);//used
  float getCoreTTfEta() const;          //used
  void  setCoreTTfPhi(float fphi);//used
  float getCoreTTfPhi() const;          //used
  uint  getCoreTTiEta() const;          //used  
  // EDM quantities, corrected for resolution  
  uint  getEtEMiso() const;
  uint  getEtHad1() const;
  uint  getEtHad2() const;
  uint  getEtEM() const;
  uint  getEt() const;
  int   getGlobalEta() const;
  uint  getGlobalPhi() const;
  uint32_t getTobWord() const;          //used

  /** Calculate variables for top word **/
  void calcFwdElEDM();            //used

  /** Calculate top word **/
  std::unique_ptr<jFEXForwardElecTOB> getFwdElTOBs();


 private:

  int  m_coreTTEtEM = 0;
  bool m_coreTTsatEM = false;
  uint  m_nextTTID = 0;
  int  m_nextTTEtEM = 0;
  bool m_nextTTsatEM = false;
  int  m_TTEtEMiso = 0;
  int  m_TTEtHad1 = 0;
  int  m_TTEtHad2 = 0;
  int  m_TTClusEtEM = 0;
  bool m_TTClusSatEM = false;
  float m_coreTTfEta = 0;
  float m_coreTTfPhi = 0;
  std::vector<uint> m_TTsInSearchWindow = {};
  //---- configured (constructor, setup) ----- 
  uint  m_coreTTID = 0; 
  int   m_ieta = 0; 
  int   m_iphi = 0; 
  uint  m_jfex = 0; 
  uint  m_reso = 200; 
  int  m_cval[9] = {1,2,3,20,30,40,20,30,40}; 
  //---- modified in calcTobValues  -----  
  uint  m_EtEMiso = 0;
  uint  m_EtHad1  = 0;
  uint  m_EtHad2  = 0;
  uint  m_EtEM    = 0;
  uint  m_et      = 0; 
  uint  m_emiso   = 0; 
  uint  m_emfr1   = 0; 
  uint  m_emfr2   = 0; 
  uint  m_eta     = 0; 
  uint  m_phi     = 0; 
  uint  m_sat     = 0; 
  uint  m_res     = 0;
  uint32_t m_tob  = 0; 
  //----
  /*
  uint m_coreL2TTID = 0;
  uint m_coreL3TTID = 0; 
  uint m_nextL2TTID = 0;
  uint m_thirdL2TTID = 0;   
   */



};


}//end of namespace

CLASS_DEF( LVL1::jFEXForwardElecInfo , 219384518 , 1 )
#endif
