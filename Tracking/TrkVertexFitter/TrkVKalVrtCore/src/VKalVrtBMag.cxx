/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//  Create object vkalMagFld which containg magnetic field.
//  It accepts external handlers with magnetic field ( function or object)
//  and takes data from them. If they are absent - default constants
//  ATLAS magnetic field (1.996 tesla) is used
//
//   Single object myMagFld of vkalMagFld type is created in CFit.cxx
//-------------------------------------------------------------------------
#include "TrkVKalVrtCore/VKalVrtBMag.h"

#include <iostream>

#include "TrkVKalVrtCore/TrkVKalVrtCore.h"

namespace {
constexpr double s_cnstBMAG = 1.997;  // Tesla
}

namespace Trk {

void vkalMagFld::getMagFld(const double X, const double Y, const double Z,
                           double& bx, double& by, double& bz,
                           const VKalVrtControlBase* FitControl = nullptr) {
  bx = by = 0.;
  if (FitControl == nullptr || (FitControl->vk_funcMagFld == nullptr &&
                                FitControl->vk_objMagFld == nullptr)) {
    bz = s_cnstBMAG;
    return;
  }
  if (FitControl->vk_funcMagFld) {
    FitControl->vk_funcMagFld(X, Y, Z, bx, by, bz);
  } else if (FitControl->vk_objMagFld) {
    FitControl->vk_objMagFld->getMagFld(X, Y, Z, bx, by, bz);
  }
}

double vkalMagFld::getMagFld(
    const double xyz[3], const VKalVrtControlBase* FitControl = nullptr) {
  double bx = 0., by = 0., bz = 0.;
  if (FitControl == nullptr || (FitControl->vk_funcMagFld == nullptr &&
                                FitControl->vk_objMagFld == nullptr)) {
    bz = s_cnstBMAG;
    return bz;
  }
  if (FitControl->vk_funcMagFld) {
    FitControl->vk_funcMagFld(xyz[0], xyz[1], xyz[2], bx, by, bz);
  } else if (FitControl->vk_objMagFld) {
    FitControl->vk_objMagFld->getMagFld(xyz[0], xyz[1], xyz[2], bx, by, bz);
  }
  return bz;
}

}  // namespace Trk

