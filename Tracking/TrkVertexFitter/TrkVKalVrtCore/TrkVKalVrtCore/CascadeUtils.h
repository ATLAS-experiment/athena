/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_CASCADEUTILS_H
#define TRKVKALVRTCORE_CASCADEUTILS_H
#include <vector>

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"
namespace Trk {

int fixPseudoTrackPt(long int NPar, double *fullMtx, double *LSide,
                     CascadeEvent &cascadeEvent_);
VKTrack *getCombinedVTrack(VKVertex *vk);
int getCascadeNPar(CascadeEvent &cascadeEvent_, int Type = 0);
void setFittedParameters(const double *result, std::vector<int> &matrixPnt,
                         CascadeEvent &cascadeEvent_);
void setFittedMatrices(const double *COVFIT, long int MATRIXSIZE,
                       std::vector<int> &matrixPnt,
                       std::vector<std::vector<double> > &covarCascade,
                       CascadeEvent &cascadeEvent_);
std::vector<double> transformCovar(int NPar, double **Deriv,
                                   const std::vector<double> &covarI);
void addCrossVertexDeriv(CascadeEvent &cascadeEvent_, double *ader,
                         long int MATRIXSIZE,
                         const std::vector<int> &matrixPnt);
void copyFullMtx(const double *Input, long int IPar, long int IDIM,
                 double *Target, long int TStart, long int TDIM) noexcept;
void getNewCov(const double *OldCov, const double *Der, double *Cov,
               long int DIM);
}  // namespace Trk

#endif
