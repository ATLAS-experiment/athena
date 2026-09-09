/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// VKalVrtAtlas.h
//
#ifndef TRKVKALVRTFITTER_VKALEXTPROPAGATOR_H
#define TRKVKALVRTFITTER_VKALEXTPROPAGATOR_H

// External propagator
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkVKalVrtCore/Propagator.h"

namespace Trk {

class TrkVKalVrtFitter;
class StraightLineSurface;

//  External propagator access for TrkVKalVrtCore
//-----------------------------------------------------
class VKalExtPropagator : public Trk::basePropagator {
 public:
  VKalExtPropagator(TrkVKalVrtFitter *);
  virtual ~VKalExtPropagator();

  //
  // Propagator from RefStart point  to RefEnd point in local coordinate
  //   system. Global coordinates are encapsulated inside function
  //
  virtual void Propagate(long int TrkID, long int Charge, const double *ParOld,
                         const double *CovOld, const double *RefStart, const double *RefEnd,
                         double *ParNew, double *CovNew,
                         IVKalState &istate) const override;
  virtual bool checkTarget(double *, const IVKalState &istate) const override;

  void setPropagator(const IExtrapolator *);

  const TrackParameters *myExtrapWithMatUpdate(long int TrkID,
                                               const TrackParameters *inpPer,
                                               Amg::Vector3D *endPoint,
                                               const IVKalState &istate) const;
  const TrackParameters *myExtrapToLine(long int TrkID,
                                        const TrackParameters *inpPer,
                                        Amg::Vector3D *endPoint,
                                        StraightLineSurface &lineTarget,
                                        const IVKalState &istate) const;
  const NeutralParameters *myExtrapNeutral(const NeutralParameters *inpPer,
                                           Amg::Vector3D *endPoint) const;

  const Perigee *myxAODFstPntOnTrk(const xAOD::TrackParticle *xprt) const;

 private:
  const IExtrapolator *m_extrapolator;  //!< Pointer to Extrapolator AlgTool
  TrkVKalVrtFitter *m_vkalFitSvc;       //!< Pointer to TrkVKalVrtFitter

  double Protection(const double *, const IVKalState &istate) const;
};

}  // namespace Trk

#endif
