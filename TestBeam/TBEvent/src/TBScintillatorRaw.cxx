/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TBEvent/TBScintillatorRaw.h"

#include "TBEvent/TBBeamDetector.h"



TBScintillatorRaw::TBScintillatorRaw(std::string_view scintillatorName,
		 const TBTDCRawCont* tdcCont,TBTDCRaw* tbtdc,
		 const TBADCRawCont* adcCont,TBADCRaw* tbadc)
  : TBBeamDetector(scintillatorName)
{
  m_tdclink.toContainedElement(*tdcCont,tbtdc);
  m_adclink.toContainedElement(*adcCont,tbadc);    
}

void TBScintillatorRaw::setSignals(const TBTDCRawCont* tdcCont,TBTDCRaw* tbtdc,
				   const TBADCRawCont* adcCont,TBADCRaw* tbadc){
  m_tdclink.toContainedElement(*tdcCont,tbtdc);
  m_adclink.toContainedElement(*adcCont,tbadc);    
}

