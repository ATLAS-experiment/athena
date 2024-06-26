/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FPTracker/ConfigData.h"
#include "FPTracker/DataDumperHelpers.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
namespace FPTracker{
      

  const double ConfigData::speedOfLight    = 0.299792458; // in the appropriate units

    ConfigData::ConfigData():
      IP              (0),
      useaper         (false),
      apermb          (0.),
      xcol1           (999.),
      xcol2           (999.),
      pbeam0          (7000),
      brho            (7000./speedOfLight),

      // tracking will be done to the following 
      // z-value * side sign
      endMarker       (420.),


      // absZMagMax - do not  read in magnets 
      // from TWISS file with z greater than this value.
      // default value set to AFP (= 437 for ALFA)
      absZMagMax      (437.) 
    {
    }
  
  void ConfigData::setpbeam(double pbeam) {
    pbeam0 = pbeam;
    brho   = pbeam/speedOfLight;
  }

  std::string ConfigData::toString() const{
    std::ostringstream ost;
    ost << std::left
      << std::setw(20) << "IP"            << IP         << '\n'
      << std::setw(20) << "useaper"       << useaper    << '\n'
      << std::setw(20) << "apermb"        << apermb     << '\n'
      << std::setw(20) << "xcol1"         << xcol1      << '\n'
      << std::setw(20) << "xcol2"         << xcol2      << '\n'
      << std::setw(20) << "Brho"          << brho       << '\n'
      << std::setw(20) << "pbeam0"        << pbeam0     << '\n'
      << std::setw(20) << "endMarker"     << endMarker  << '\n'
      << std::setw(20) << "absZMagMax"    << absZMagMax << '\n';
    return ost.str();
    
  }
  
  std::ostream& operator<<(std::ostream& os, const ConfigData& cd){
    os<<cd.toString();
    return os;
  }

}
