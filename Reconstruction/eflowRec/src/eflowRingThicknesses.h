/*   Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration */

#ifndef EFLOWRINGTHICKNESSES_H
#define EFLOWRINGTHICKNESSES_H

#include "eflowCaloRegions.h"

class eflowRingThicknesses { 

public:
   static double ringThickness(const eflowCaloENUM& layer);

};

#endif
