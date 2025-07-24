/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_LARSTRIPNEIGHBORHOODDUMPER_H
#define GLOBALSIM_LARSTRIPNEIGHBORHOODDUMPER_H

#include "LArStripNeighborhoodContainer.h"
#include "LArStripNeighborhood.h"
#include "xAODEventInfo/EventInfo.h"

namespace GlobalSim {

  class LArStripNeighborhoodDumper {
  public:
    LArStripNeighborhoodDumper();

    StatusCode
    dump(const std::string& name,
	 const xAOD::EventInfo& eventInfo,
         const LArStripNeighborhoodContainer&) const;

    StatusCode
    dumpTerse(const std::string& name,
	      const xAOD::EventInfo& eventInfo,
              const LArStripNeighborhoodContainer&) const;    
  };
}

#endif
