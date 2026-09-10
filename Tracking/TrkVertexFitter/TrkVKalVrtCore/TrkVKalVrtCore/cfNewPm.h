/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CFNEWPM_H
#define TRKVKALVRTCORE_CFNEWPM_H

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {
void cfnewpm(const double *par, const double *xyzStart, const double *xyzEnd,
             const double ustep, double *parn, double *closePoint,
             const VKalVrtControlBase * CONTROL);

}  // namespace Trk

#endif

