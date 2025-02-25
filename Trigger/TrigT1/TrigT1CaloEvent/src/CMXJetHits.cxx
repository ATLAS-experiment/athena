/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/




#ifndef  TRIGGERSPACE
#include "TrigT1CaloEvent/CMXJetHits.h"
#else
#include "CMXJetHits.h"
#endif

namespace LVL1 {


/** constructs a CMXJetHits object, specifying crate, cmx, source. */
CMXJetHits::CMXJetHits(int crate, int source):
  m_crate(crate),
  m_source(source)
{
}

/** constructs a CMXJetHits object and fill all data members */
CMXJetHits::CMXJetHits(int crate, int source,
                       const std::vector<unsigned int>& hits0,
                       const std::vector<unsigned int>& hits1,
  		       const std::vector<int>& error0,
		       const std::vector<int>& error1,
		       int peak):
  m_crate(crate),
  m_source(source),
  m_peak(peak),
  m_hits0(hits0),
  m_hits1(hits1),
  m_error0(error0),
  m_error1(error1)
{
}

} // end of namespace bracket
