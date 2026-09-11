/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//  Creates interface vkalPropagator object which contains pointers to real
//  propagators ( external or internal)
//
//  Single myPropagator object of vkalPropagator type is created in CFit.cxx
//------------------------------------------------------------------------
// Header include
#include "TrkVKalVrtCore/Propagator.h"

#include <cmath>

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"
#include "TrkVKalVrtCore/VKalVrtBMag.h"
#include "TrkVKalVrtCore/cfErPr.h"
#include "TrkVKalVrtCore/cfNewP.h"
#include "TrkVKalVrtCore/cfNewPm.h"
//-------------------------------------------------

namespace {
// anonymous namespace for keeping internal
// implementation methods
// So as to have internal linkage
// no symbols exported
using namespace Trk;

//-------------------------------------------------------------------------
//   Simple propagator in constant field
//   Due to a translation invariance propagation is done in relative coordinates
//   assuming that starting point is (0,0,0)
//
void PropagateSTD(long int /*TrkID*/, long int Charge,
                  const double *ParOld, const double *CovOld,
                  const double *RefStart, const double *RefEnd,
                  double *ParNew, double *CovNew,
                  const VKalVrtControlBase *CONTROL) {
  double Way, closePoint[3], Goal[3];
  Goal[0] = RefEnd[0] - RefStart[0];
  Goal[1] = RefEnd[1] - RefStart[1];
  Goal[2] = RefEnd[2] - RefStart[2];
  cfnewp(Charge, ParOld, &Goal[0], &Way, ParNew, closePoint);
  if (CovOld != nullptr)
    cferpr(Charge, ParOld, &Goal[0], Way, CovOld, CovNew);
  if (Charge ==
      0) {  // Correction for different magnetic field values in Ini and End
    double vBx, vBy, vBz, vBzn;
    Trk::vkalMagFld::getMagFld(RefStart[0], RefStart[1], RefStart[2], vBx, vBy,
                               vBz, CONTROL);
    Trk::vkalMagFld::getMagFld(RefEnd[0], RefEnd[1], RefEnd[2], vBx, vBy, vBzn,
                               CONTROL);
    double Corr = vBzn / vBz;
    ParNew[4] *= Corr;
    if (CovOld != nullptr) {
      CovNew[10] *= Corr;
      CovNew[11] *= Corr;
      CovNew[12] *= Corr;
      CovNew[13] *= Corr;
      CovNew[14] *= Corr * Corr;
    }
  }
}

//--------------------------------------------------------------------------------
//   Runge-Kutta propagator in nonuniform field
//
void PropagateRKM(long int Charge,
                  const double *ParOld, const double *CovOld,
                  const double *RefStart, const double *RefEnd,
                  double *ParNew, double *CovNew,
                  VKalVrtControlBase *CONTROL){
  double Way;
  double closePoint[3], Goal[3];
  Goal[0] = RefEnd[0] - RefStart[0];
  Goal[1] = RefEnd[1] - RefStart[1];
  Goal[2] = RefEnd[2] - RefStart[2];
  cfnewp(Charge, ParOld, &Goal[0], &Way, ParNew, closePoint);
  if (CovOld != nullptr)
    cferpr(Charge, ParOld, &Goal[0], Way, CovOld, CovNew);

  if (Charge != 0)
    cfnewpm(ParOld, RefStart, RefEnd, Way, ParNew, closePoint, CONTROL);

  if (Charge ==
      0) {  // Correction for different magnetic field values in Ini and End
    double vBx, vBy, vBz, vBzn;
    Trk::vkalMagFld::getMagFld(RefStart[0], RefStart[1], RefStart[2], vBx, vBy,
                               vBz, CONTROL);
    Trk::vkalMagFld::getMagFld(RefEnd[0], RefEnd[1], RefEnd[2], vBx, vBy, vBzn,
                               CONTROL);
    double Corr = vBzn / vBz;
    ParNew[4] *= Corr;
    if (CovOld != nullptr) {
      CovNew[10] *= Corr;
      CovNew[11] *= Corr;
      CovNew[12] *= Corr;
      CovNew[13] *= Corr;
      CovNew[14] *= Corr * Corr;
    }
  }
}

}  // namespace

namespace Trk {

vkalPropagator::vkalPropagator() = default;
basePropagator::basePropagator() = default;
basePropagator::~basePropagator() = default;

bool vkalPropagator::checkTarget(const double *) {
  // if ( m_typePropagator == 3 ) return vk_objectProp->checkTarget(RefNew);
  return true;
}
//------------------------------------------------------------------------
//  Old propagator functions:
//      CFNEWP  - doesn't use magnetic field at all. Only track curvature.
//                So it's symmetric for back and forward propagation
//      CFNEWPM - use exact nonuniform magnetic field so is not symmetric.
//                Used for forward propagation  (0,0,0) -> REF.
//                Returns perigee parameters with respect to REF
//                (then in new reference system with center at REF!!!)
//
//  Usually all package routines propagate from (0,0,0) to REF.
//  Only XYZTRP propagates from REF to (0,0,0) and then uses BACKPROPAGATION
//   Propagation now is done from  RefStart point to RefEnd point
//   A final position after propagation is PERIGEE assuming that a REF(target)
//   is a CENTER OF NEW COORDINATE SYSTEM  with axes parallel to initial ones

void vkalPropagator::Propagate(long int TrkID, long int Charge, const double *ParOld,
                               const double *CovOld, const double *RefOld, double *RefNew,
                               double *ParNew, double *CovNew,
                               VKalVrtControlBase *FitControl) {
  if (RefOld[0] == RefNew[0] && RefOld[1] == RefNew[1] &&
      RefOld[2] == RefNew[2]) {
    std::copy(ParOld, ParOld + 5, ParNew);
    if (CovOld != nullptr) {
      std::copy(CovOld, CovOld + 15, CovNew);
    }
    return;
  }
  //
  //-- Propagation itself
  //
  if (FitControl == nullptr ||
      (FitControl->vk_objProp == nullptr &&
       FitControl->vk_funcProp ==
           nullptr)) {  // No external propagators, use internal ones
    // std::cout<<" Core: use INTERNAL propagator. Charge="<<Charge<<'\n';
    if (vkalUseRKMPropagator) {
      PropagateRKM(Charge, ParOld, CovOld, RefOld, RefNew, ParNew, CovNew,
                   FitControl);
    } else {
      PropagateSTD(TrkID, Charge, ParOld, CovOld, RefOld, RefNew, ParNew,
                   CovNew, FitControl);
    }
    return;
  }

  if (FitControl->vk_objProp) {
    // std::cout<<" Core: use EXTERNAL propagator. Charge="<<Charge<<'\n';
    if (Charge == 0) {
      PropagateSTD(TrkID, Charge, ParOld, CovOld, RefOld, RefNew, ParNew,
                   CovNew, FitControl);
    } else {
      FitControl->vk_objProp->Propagate(TrkID, Charge, ParOld, CovOld, RefOld,
                                        RefNew, ParNew, CovNew,
                                        *FitControl->vk_istate);
      if (ParNew[0] == 0. && ParNew[1] == 0. && ParNew[2] == 0. &&
          ParNew[3] == 0. && ParNew[4] == 0.) {
        PropagateRKM(Charge, ParOld, CovOld, RefOld, RefNew, ParNew, CovNew,
                     FitControl);
      }
    }
    return;
  }
  //-------------------
  if (FitControl->vk_funcProp) {
    FitControl->vk_funcProp(TrkID, Charge, ParOld, CovOld, RefOld, RefNew,
                            ParNew, CovNew);
    return;
  }
  //-------------------
}

void vkalPropagator::Propagate(VKTrack *trk, const double *RefOld, const double *RefNew,
                               double *ParNew, double *CovNew,
                               VKalVrtControlBase *FitControl) {
  if (RefOld[0] == RefNew[0] && RefOld[1] == RefNew[1] &&
      RefOld[2] == RefNew[2]) {
    std::copy(trk->refPerig, trk->refPerig + 5, ParNew);
    std::copy(trk->refCovar, trk->refCovar + 15, CovNew);
    return;
  }
  long int TrkID = trk->Id;
  long int Charge = trk->Charge;
  //
  //-- Propagation itself
  //
  if (FitControl == nullptr ||
      (FitControl->vk_objProp == nullptr &&
       FitControl->vk_funcProp ==
           nullptr)) {  // No external propagators, use internal ones
    if (vkalUseRKMPropagator) {
      PropagateRKM(Charge, trk->refPerig, trk->refCovar, RefOld, RefNew, ParNew,
                   CovNew, FitControl);
    } else {
      PropagateSTD(TrkID, Charge, trk->refPerig, trk->refCovar, RefOld, RefNew,
                   ParNew, CovNew, FitControl);
    }
    return;
  }

  if (FitControl->vk_objProp) {
    if (Charge == 0) {
      PropagateSTD(TrkID, Charge, trk->refPerig, trk->refCovar, RefOld, RefNew,
                   ParNew, CovNew, FitControl);
    } else {
      FitControl->vk_objProp->Propagate(TrkID, Charge, trk->refPerig,
                                        trk->refCovar, RefOld, RefNew, ParNew,
                                        CovNew, *FitControl->vk_istate);
      if (ParNew[0] == 0. && ParNew[1] == 0. && ParNew[2] == 0. &&
          ParNew[3] == 0. && ParNew[4] == 0.) {
        PropagateRKM(Charge, trk->refPerig, trk->refCovar, RefOld, RefNew,
                     ParNew, CovNew, FitControl);
      }
    }
    return;
  }
  //-------------------
  if (FitControl->vk_funcProp) {
    FitControl->vk_funcProp(TrkID, Charge, trk->refPerig, trk->refCovar, RefOld,
                            RefNew, ParNew, CovNew);
    return;
  }
}

}  // namespace Trk

