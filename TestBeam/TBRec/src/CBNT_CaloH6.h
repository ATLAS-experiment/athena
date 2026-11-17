/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//######################################################
//#                                                    #
//# File   : CBNT_Calo_H6.h                            #
//# Package: TBRec                                     #
//#                                                    #
//# Author : Marco Bieri <mbieri@sfu.ca>               #
//# Adopted for CBNTAA Pavol Strizenec                 #
//#                                                    #
//######################################################

#ifndef TBREC_CBNT_CALOH6_H
#define TBREC_CBNT_CALOH6_H

#include "GaudiKernel/ToolHandle.h"
#include "CBNT_TBRecBase.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloID.h"
// #include "LArDetDescr/LArDetDescrManager.h"

// NOISE
#include "CaloConditions/CaloNoise.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <fstream>
#include <string>
#include <vector>

class CBNT_CaloH6 : public CBNT_TBRecBase
{
 public:
  CBNT_CaloH6(const std::string & name, ISvcLocator * pSvcLocator);
  
  virtual ~CBNT_CaloH6();
  
  //standart algorithm methods
  virtual StatusCode CBNT_initialize() override;
  virtual StatusCode CBNT_execute() override;
  virtual StatusCode CBNT_finalize() override;
  virtual StatusCode CBNT_clear() override;
  
 private:
  // LArDetDescrManager* m_larMgr;

  // Identifier Pointers
  const LArEM_ID* m_emecID_help{nullptr};
  const LArHEC_ID* m_hecID_help{nullptr};
  const LArFCAL_ID* m_fcalID_help{nullptr};

  SG::ReadCondHandleKey<CaloNoise> m_elecNoiseKey
    { this, "ElecNoiseKey", "electronicNoise", "SG key for electronic noise" };
  
  // picking detector and number of channels
  Gaudi::Property<std::string> m_det_type{this, "Det_Type", "", "Detector type (HEC, EMEC or FCAL)"};
  Gaudi::Property<int> m_numchan{this, "Max_Chan", 0, "Maximum number of channels"};

  /*-------------------Ntuple--------------------------------*/
  
  std::vector<float> *m_energy{nullptr};
  std::vector<float> *m_NoiseRMS{nullptr};
  std::vector<float> *m_ieta{nullptr};
  std::vector<float> *m_iphi{nullptr};
  std::vector<float> *m_eta{nullptr};
  std::vector<float> *m_phi{nullptr};
  std::vector<float> *m_sampling{nullptr};
  std::vector<float> *m_region{nullptr};

};                                                                      
#endif
