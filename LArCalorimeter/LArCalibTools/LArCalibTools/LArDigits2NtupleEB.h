/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITS2NTUPLEEB_H
#define LARDIGITS2NTUPLEEB_H

#include "LArCalibTools/LArCond2NtupleBaseEB.h"
#include "StoreGate/ReadHandleKey.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "LArRawEvent/LArFebHeaderContainer.h"



class LArDigits2NtupleEB : public LArCond2NtupleBaseEB
{
 public:
  LArDigits2NtupleEB(const std::string & name, ISvcLocator * pSvcLocator);

  // Standard algorithm methods
  virtual StatusCode initialize()  override;
  virtual StatusCode execute(const EventContext& ctx)  override;

 protected:

  int m_ipass;

  long m_event;

  Gaudi::Property< unsigned int >  m_Nsamples{this, "NSamples", 32, "number of samples to store"};
  Gaudi::Property< unsigned int >  m_Net{this, "Net", 5, "number of energies to store"};
  Gaudi::Property< std::vector<unsigned int> > m_FTlist{this, "FTlist", {}, "which FT to dump"};
  Gaudi::Property< bool > m_fillBCID{this, "FillBCID", false, "if to fill BCID"};

  NTuple::Array<int> m_ntNsamples;
  NTuple::Array<short> m_gain;
  NTuple::Array<short> m_ELVL1Id;
  NTuple::Item<unsigned long long> m_IEvent;
  NTuple::Matrix<short>  m_samples;
  NTuple::Item<short> m_bcid;

  NTuple::Matrix<float> m_energyVec_ET;
  NTuple::Matrix<float> m_bcidVec_ET;
  NTuple::Matrix<bool> m_saturVec_ET;

  NTuple::Matrix<float> m_energyVec_ET_ID;
  NTuple::Matrix<float> m_bcidVec_ET_ID;
  NTuple::Matrix<bool> m_saturVec_ET_ID;

  SG::ReadHandleKey<LArDigitContainer> m_contKey{this, "ContainerKey", "FREE", "key for LArDigitContainer"};
  SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey { this, "EventInfoKey", "EventInfo", "SG for EventInfo Key" };
  SG::ReadHandleKey<LArFebHeaderContainer> m_LArFebHeaderContainerKey { this, "LArFebHeaderKey", "LArFebHeader" };
};

#endif
