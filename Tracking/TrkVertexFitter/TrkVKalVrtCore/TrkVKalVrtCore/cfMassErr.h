/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFMASSERR_H
#define TRKVKALVRTCORE_CFMASSERR_H

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {

void cfmasserr(VKVertex *vk, const int *list, double *MASS, double *sigM);

}  // namespace Trk

#endif

