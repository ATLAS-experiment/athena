/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_SETUPBEAMLINE_H
#define FPTRACKER_SETUPBEAMLINE_H

#include "FPTracker/FPTrackerConstants.h" //Side
#include <memory>
#include <iosfwd>  //std::ifstream


namespace FPTracker{
  class Beamline;
  class ConfigData;
  Beamline setupBeamline(
			 const ConfigData&,
			 const Side&,
			 int magver,
			 std::shared_ptr< std::ifstream >
			 );
}
#endif
