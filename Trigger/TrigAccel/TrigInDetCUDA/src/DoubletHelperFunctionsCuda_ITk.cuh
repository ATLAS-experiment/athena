/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_DOUBLETHELPERFUNCTIONSCUDA_ITK_CUH
#define TRIGINDETCUDA_DOUBLETHELPERFUNCTIONSCUDA_ITK_CUH

/**
 * @file TrigInDetCUDA/DoubletHelperFunctionsCuda.cuh
 * @brief Common function between ITk Track Seeding kernels, in particular DoubletCounting and DoubletMaking.
 */


namespace GPUTrackSeedingItkHelpers {

/**
 * @brief Calculate eta for a doublet
 * @param dr radius difference between doublet's space points
 * @param dz z axis difference between doublet's space points
 * @param dL doublet length
 */
__device__ static float getEta (float dr, float dz, float dL) {
  return -std::log((dL-dz)/dr);
}

/**
 * @brief Calculate maximum doublet length, expressed as a quartic function, for a given eta
 * @param eta pseudorapidity of a doublet
 */
__device__ static float getMaxDeltaLEta (float eta) {
  float hardCut = 1300;
  float maxDL = eta*eta*eta*eta*1.97572003 + eta*eta*92.29732795 + 168.54257599;
  return (maxDL > hardCut) ? hardCut : maxDL;
}

__device__ static int getInnerDoubletIdx (int pairIdx, int nOuter) {
  return nOuter > 0 ? pairIdx/nOuter : 0;
}

__device__ static int getOuterDoubletIdx (int pairIdx, int nOuter, int startOfOuter) {
  return startOfOuter + pairIdx % nOuter;
}


}

#endif
