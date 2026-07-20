//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


/*
   * @author S. Laplace
   * 14. 12. 2005
   * Modifications:
   * P. Strizenec 9.3.2023 migration to rel. 23 
*/

#ifndef LARAVERAGES2NTUPLE_H
#define LARAVERAGES2NTUPLE_H

#include "LArCalibTools/LArCond2NtupleBase.h"
#include "StoreGate/ReadHandleKey.h"
#include "LArRawEvent/LArAccumulatedCalibDigitContainer.h"
#include "LArIdentifier/LArOnlineID.h"

class LArAverages2Ntuple : public LArCond2NtupleBase
{
 public:
  LArAverages2Ntuple(const std::string & name, ISvcLocator * pSvcLocator);
  ~LArAverages2Ntuple(){};

  //standard algorithm methods
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) override final;

 private:
  const LArOnlineID_Base* m_onlineHelper = nullptr;

  SG::ReadHandleKey<LArAccumulatedCalibDigitContainer> m_contKey{this, "ContainerKey","","LArAccumulatedCalibDigit key"};
  Gaudi::Property< unsigned int >  m_Nsamples{this, "NSamples", 32,"Number of samples to store"};
  Gaudi::Property< bool >  m_keepPulsed{this, "KeepOnlyPulsed", true};
  Gaudi::Property< std::vector<unsigned int> > m_keepFT{this,"KeepFT", {}, "list of FT to keep"};

  std::string m_ntName;

  unsigned long long m_event;
  bool m_pass;

  NTuple::Item<unsigned long long> m_IEvent;
  NTuple::Item<unsigned long long> m_EventNum;
  NTuple::Item<long> m_Nsteps;
  NTuple::Item<long> m_DAC;
  NTuple::Item<long> m_Ntrigger;
  NTuple::Item<long> m_delay;
  NTuple::Item<long> m_ntNsamples;
  NTuple::Item<long> m_isPulsed;
  
  NTuple::Item<unsigned long> m_StepIndex;
  
  NTuple::Array<unsigned int> m_Sum;
  NTuple::Array<unsigned int> m_SumSq;
  NTuple::Array<float> m_Mean;
  NTuple::Array<float> m_RMS;

};

#endif
