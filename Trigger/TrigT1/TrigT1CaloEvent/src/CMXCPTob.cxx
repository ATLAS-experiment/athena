/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/




#ifndef  TRIGGERSPACE
#include "TrigT1CaloEvent/CMXCPTob.h"
#else
#include "CMXCPTob.h"
#endif

namespace LVL1 {



/** constructs a CMXCPTob object, specifying crate, cmx, cpm etc. */
CMXCPTob::CMXCPTob(int crate, int cmx, int cpm, int chip, int loc):
  m_crate(crate),
  m_cmx(cmx),
  m_cpm(cpm),
  m_chip(chip),
  m_location(loc)
{
}

/** constructs a CMXCPTob object and fill all data members */
CMXCPTob::CMXCPTob(int crate, int cmx, int cpm, int chip, int loc,
                   const std::vector<int>& energy,
                   const std::vector<int>& isolation,
		   const std::vector<int>& error,
		   const std::vector<unsigned int>& presenceMap,
		   int peak):
  m_crate(crate),
  m_cmx(cmx),
  m_cpm(cpm),
  m_chip(chip),
  m_location(loc),
  m_peak(peak),
  m_energy(energy),
  m_isolation(isolation),
  m_error(error),
  m_presenceMap(presenceMap)
{
}

} // end of namespace bracket
