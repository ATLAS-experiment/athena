/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimObjects/FPGATrackSimOfflineHit.h"
#include <iostream>



std::ostream& operator<<(std::ostream& s, const FPGATrackSimOfflineHit& h) {

  s << "locX " << h.getLocX()
    << " \t locY " << h.getLocY()
    << " \t isPixel " << h.isPixel()
    << " \t isBarrel " << h.isBarrel()
    << " \t layer " << h.getLayer()
    << " \t clusterID " << h.getClusterID()
    << " \t track number " << h.getTrackNumber()
    << std::endl;


  return s;
}
