/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_MAGNETSET_H
#define FPTRACKER_MAGNETSET_H


#include "Magnet.h"
#include "FPTrackerConstants.h" //Side
#include <memory> //std::shared_ptr
#include <iosfwd> //std::ifstream

namespace FPTracker{
  class ConfigData;
  Magnet::Container_t  magnetSet(
				 const ConfigData&, 
				 const Side& side,
				 int magversion,
				 std::shared_ptr< std::ifstream> magfile);
}
#endif
