/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_EFLOWRINGTHICKNESSES_H
#define EFLOWREC_EFLOWRINGTHICKNESSES_H

#include "eflowCaloRegions.h"

class eflowRingThicknesses { 

public:
   static double ringThickness(const eflowCaloENUM& layer);

};

#endif
