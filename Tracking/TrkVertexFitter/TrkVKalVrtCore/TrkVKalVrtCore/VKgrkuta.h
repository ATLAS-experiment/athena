/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_VKGRKUTA_H
#define TRKVKALVRTCORE_VKGRKUTA_H

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {
void vkgrkuta_(const double charge, const double step, double *vect,
               double *vout, const VKalVrtControlBase *CONTROL);

}  // namespace Trk

#endif

