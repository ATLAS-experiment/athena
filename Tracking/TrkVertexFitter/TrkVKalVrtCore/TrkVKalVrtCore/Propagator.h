/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/* General track propagation implementation                */
/* If external propagator is provided as                   */
/* either an object inherited from the basePropagator class*/
/* or a function addrPropagator - the vkalPropagator class */
/* will use it, otherwise the vkalPropagor uses simple     */
/* propagatiob in constant magnetic field.                 */
/* 							   */
/*  Thread-safe implementation				   */
/*---------------------------------------------------------*/
#ifndef TRKVKALVRTCORE_PROPAGATOR_H
#define TRKVKALVRTCORE_PROPAGATOR_H

namespace Trk {

#define vkalUseRKMPropagator 0

class VKalVrtControl;
class IVKalState;

typedef void (*addrPropagator)(long int, long int,
                               const double *,  // ParOld
                               const double *,  // CovOld
                               const double *,  // RefStart
                               const double *,  // RefEnd
                               double *,        // ParNew
                               double *);       // CovNew

class VKTrack;
class VKalVrtControlBase;

//
// Base class for concrete implementation of propagator (e.g. Athena one) to be
// called by vkalPropagator
//
class basePropagator {
 public:
  basePropagator();
  virtual ~basePropagator();
  //
  // Propagator from RefStart point to RefEnd point
  //     ParOld[5] - track perigee parameters with respect to RefStart point
  //     ParNew[5] - track perigee parameters with respect to RefEnd point
  virtual void Propagate(long int TrkID, long int Charge, const double *ParOld,
                         const double *CovOld, const double *RefStart, const double *RefEnd,
                         double *ParNew, double *CovNew,
                         IVKalState &istate) const = 0;
  virtual bool checkTarget(double *RefEnd, const IVKalState &istate) const = 0;
  //
};

//
// Main propagator in VKalVrtCore package.
// Depending on VKalVrtControlBase object it either calls external propagator
// or uses default implementations
//
class vkalPropagator {
 public:
  vkalPropagator();
  ~vkalPropagator() = default;

  static void Propagate(long int TrkID, long int Charge, const double *ParOld,
                               const double *CovOld, const double *RefOld, double *RefNew,
                               double *ParNew, double *CovNew,
                               VKalVrtControlBase *FitControl = 0);
  static bool checkTarget(const double *RefEnd);
  static void Propagate(VKTrack *trk, const double *RefStart, const double *RefEnd,
                        double *ParNew, double *CovNew,
                        VKalVrtControlBase *FitControl = 0);
};

}  // namespace Trk
#endif
