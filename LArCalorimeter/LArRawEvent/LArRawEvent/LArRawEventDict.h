/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "LArRawEvent/LArRawChannel.h" 
#include "LArRawEvent/LArDigit.h" 
#include "LArRawEvent/LArDigitContainer.h" 
#include "LArRawEvent/LArTTL1.h" 
#include "LArRawEvent/LArTTL1Container.h" 
#include "LArRawEvent/LArFebErrorSummary.h" 
#include "LArRawEvent/LArLATOMEHeaderContainer.h" 
//#include "LArRawEvent/LArRawChannelContainer.h" 
#include "LArRawEvent/LArSCDigit.h" 
#include "LArRawEvent/LArSCDigitContainer.h" 
#include "LArRawEvent/LArRawSC.h" 
#include "LArRawEvent/LArRawSCContainer.h" 
#include "LArRawEvent/LArFebHeaderContainer.h" 
#include "LArRawEvent/LArRawChannelContainer.h" 

struct GCCXML_DUMMY_INSTANTIATION_LARRAWCNV {
  LArFebHeader::RodHeader m_rowHeader;
  LArFebHeader::DspHeader m_dspHeader;
  LArFebHeader m_febHeader;
  DataVector<LArFebHeader> m_febHeaderDV;
  std::vector<LArFebHeader*> m_febHeaderVPtr;
  LArFebHeaderContainer m_febHeaderCont;

};