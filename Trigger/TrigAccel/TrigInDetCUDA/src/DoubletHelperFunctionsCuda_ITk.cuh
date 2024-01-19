/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_DOUBLETHELPERFUNCTIONSCUDA_ITK_CUH
#define TRIGINDETCUDA_DOUBLETHELPERFUNCTIONSCUDA_ITK_CUH

/**
 * @file TrigInDetCUDA/DoubletHelperFunctionsCuda.cuh
 * @brief Common function between ITk Track Seeding kernels, in particular DoubletCounting and DoubletMaking.
 */


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
  if(std::abs(eta) < 3.5) return eta*eta*eta*eta*9.38522907 + eta*eta*88.1729 + 177.363;
  else return eta*eta*eta*eta*1.7582417 + eta*eta*-129.67033 + 3324.61538;
}


__device__ static float canBeMiddleSpacePoint (float r) {
  if(std::abs(r - 100.0) > 20.0) return false;
  return true;
}

#endif