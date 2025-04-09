/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_MISSINGETFILTER_H
#define GENERATORFILTERS_MISSINGETFILTER_H

#include "GeneratorModules/GenFilter.h"

/// Filters on total missing energy from nus and LSPs
/// @author Seth Zenz, December 2005
class MissingEtFilter:public GenFilter {
public:

  MissingEtFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode filterEvent();

 private:

  Gaudi::Property<double> m_METmin{this,"METCut",10000.};
  // Normally we'd include them, but this is unstable if using EvtGen
  Gaudi::Property<bool> m_useHadronicNu{this, "UseNeutrinosFromHadrons", false};
  Gaudi::Property<bool> m_useChargedNonShowering{this, "UseChargedNonShowering",false};
  Gaudi::Property<bool> m_allowOld{this, "AllowOldFilter", false};
};

#endif
