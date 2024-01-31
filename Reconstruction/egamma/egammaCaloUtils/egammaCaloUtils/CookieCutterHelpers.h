/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COOKIECUTTERHELPERS_H
#define COOKIECUTTERHELPERS_H

#include "GaudiKernel/SystemOfUnits.h"

namespace CookieCutterHelpers
{
struct CentralPosition
{
  float etaB = 999;
  float phiB = 999;
  float emaxB = -999 * Gaudi::Units::GeV;
  float etaEC = 999;
  float phiEC = 999;
  float emaxEC = -999 * Gaudi::Units::GeV;
};

struct PhiSize
{
  float plusB = 0;
  float minusB = 0;
  float plusEC = 0;
  float minusEC = 0;
};
}

#endif

