//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


/*
   * @author R. Lafaye
   * 12. 06. 2008
   * Modifications:
   * 25.7.2014 P. Strizenec, moving to LArCond2NtupleBase
   * 9.3.2023 P. Strizenec, moving to rel. 23
*/

#ifndef LARACCUMULATEDDIGITS2NTUPLE_H
#define LARACCUMULATEDDIGITS2NTUPLE_H

#include "LArCalibTools/LArCond2NtupleBase.h"
#include "StoreGate/ReadHandleKey.h"
#include "LArRawEvent/LArAccumulatedDigitContainer.h"

class LArAccumulatedDigits2Ntuple : public LArCond2NtupleBase
{
 public:
  LArAccumulatedDigits2Ntuple(const std::string & name, ISvcLocator * pSvcLocator);
  ~LArAccumulatedDigits2Ntuple();

  //standard algorithm methods
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) override final;
 private:

   Gaudi::Property< int > m_normalize{this, "Normalize", 1, "Normalisation factor for covr"};
   Gaudi::Property< unsigned int > m_Nsamples{this, "NSamples", 7, "number of samples to store"};
   SG::ReadHandleKey<LArAccumulatedDigitContainer> m_contKey{this,"ContainerKey","","Key for LArAccumulatedDigits object"}; 

  int m_ipass;
  long m_event;

  NTuple::Item<long> m_Ntrigger;
  NTuple::Item<long> m_IEvent;
  NTuple::Item<long> m_EventNum;
  NTuple::Item<long> m_ntNsamples;
  NTuple::Array<long>  m_sum;
  NTuple::Array<long>  m_sumsq;
  NTuple::Item<float>  m_mean;
  NTuple::Item<float>  m_rms;
  NTuple::Array<float> m_covr;
  
};

#endif
