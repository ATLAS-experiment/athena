/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITS2NTUPLE_H
#define LARDIGITS2NTUPLE_H

#include "LArCalibTools/LArCond2NtupleBase.h"
#include "StoreGate/ReadHandleKey.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArAccumulatedCalibDigitContainer.h"
#include "LArRawEvent/LArAccumulatedDigitContainer.h"
#include "LArRawEvent/LArFebHeaderContainer.h"


class LArDigits2Ntuple : public LArCond2NtupleBase
{
 public:
  LArDigits2Ntuple(const std::string & name, ISvcLocator * pSvcLocator);
  ~LArDigits2Ntuple();

  // Standard algorithm methods
  virtual StatusCode initialize();
  virtual StatusCode execute();

 protected:

  int m_ipass;
  long m_event;

  Gaudi::Property< unsigned int >  m_Nsamples{this, "NSamples", 32, "number of samples to store"};
  Gaudi::Property< std::vector<unsigned int> > m_FTlist{this, "FTlist", {}, "which FT to dump"};
  Gaudi::Property< std::vector<unsigned int> > m_Slotlist{this, "Slotlist", {}, "which Slot to dump"};
  Gaudi::Property< std::vector<unsigned int> > m_Sidelist{this, "Sidelist", {}, "which side to dump"};
  Gaudi::Property< std::vector<unsigned int> > m_BElist{this, "BElist", {}, "which B or E to dump"};
  Gaudi::Property< bool > m_fillEMB{this, "FillEMB", true, "if to fill EMB"};
  Gaudi::Property< bool > m_fillEndcap{this, "FillEndcap", true, "if to fill Eendcap"};
  Gaudi::Property< bool > m_fillBCID{this, "FillBCID", false, "if to fill BCID"};
  Gaudi::Property< bool > m_fillLB{this, "FillLB", false, "if to fill LB in Evnt tree"};

  NTuple::Item<long> m_ntNsamples;
  NTuple::Item<short> m_gain;
  NTuple::Item<short> m_bcid;
  NTuple::Item<unsigned long> m_ELVL1Id;
  NTuple::Item<unsigned long long> m_IEvent;
  NTuple::Array<short>  m_samples;
  // variables for accCalibDigit case
  NTuple::Array<float>  m_mean;
  NTuple::Array<float>  m_RMS;
  NTuple::Item<unsigned int> m_dac;
  NTuple::Item<unsigned int> m_delay;
  NTuple::Item<unsigned int> m_pulsed;

  //
  //Event based ntuple pointer
  NTuple::Tuple* m_evt_nt = nullptr;

  NTuple::Item<unsigned long long> m_IEventEvt;
  NTuple::Item<short> m_LB;

  SG::ReadHandleKey<LArDigitContainer> m_contKey{this, "ContainerKey", "", "key for LArDigitContainer"};
  SG::ReadHandleKey<LArAccumulatedCalibDigitContainer> m_accCalibContKey{this, "AccCalibContainerKey", "", "key for LArAccumulatedCalibDigitDigitContainer"};
  SG::ReadHandleKey<LArAccumulatedDigitContainer> m_accContKey{this, "AccContainerKey", "", "key for LArAccumulatedDigitDigitContainer"};
  SG::ReadHandleKey<LArFebHeaderContainer> m_LArFebHeaderContainerKey { this, "LArFebHeaderKey", "LArFebHeader" };
};

#endif
