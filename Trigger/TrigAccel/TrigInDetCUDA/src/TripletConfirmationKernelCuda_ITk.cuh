/*
	Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_DOUBLETCONFIRMATIONKERNELCUDA_ITK_CUH
#define TRIGINDETCUDA_DOUBLETCONFIRMATIONKERNELCUDA_ITK_CUH


#include <cuda_runtime.h>
#include "SeedMakingDataStructures_ITk.h"
#include "DoubletHelperFunctionsCuda_ITk.cuh"

__device__ static bool isTripletInBarrel (int spiIdx, int spmIdx, int spoIdx, TrigAccel::ITk::SPACEPOINT_STORAGE* dSpacepoints) {
  return (dSpacepoints->m_type[spiIdx] == 0) && (dSpacepoints->m_type[spmIdx] == 0) && (dSpacepoints->m_type[spoIdx] == 0);
}

__global__ static void tripletConfirmationKernel_ITk(
    TrigAccel::ITk::SPACEPOINT_STORAGE* dSpacepoints,
    TrigAccel::ITk::OUTPUT_SEED_STORAGE* d_Triplets,
    TrigAccel::ITk::OUTPUT_SEED_STORAGE* d_Out,
    int nTriplets
) {

    __shared__ int nConfirming;
    __shared__ int currentSpiIdx;
    __shared__ int currentSpmIdx;
    __shared__ int currentSpoIdx;
    __shared__ bool isBarrel;


    if (blockIdx.x == 0 && threadIdx.x == 0) {
        d_Out->m_nSeeds = 0;
    }

    __syncthreads();
    
    for (int currentTripletIdx = blockIdx.x; currentTripletIdx<nTriplets; currentTripletIdx += gridDim.x) {

        if (threadIdx.x == 0) {
            nConfirming = 0;
            currentSpiIdx = d_Triplets->m_innerIndex[currentTripletIdx];
            currentSpmIdx = d_Triplets->m_middleIndex[currentTripletIdx];
            currentSpoIdx = d_Triplets->m_outerIndex[currentTripletIdx];
            
            // Barrel triplets don't require confirmation
            isBarrel = isTripletInBarrel(currentSpiIdx, currentSpmIdx, currentSpoIdx, dSpacepoints);

        }
        __syncthreads();

        // Empty output
        if (currentSpiIdx == currentSpoIdx) continue;


        if (isBarrel) {
            if (threadIdx.x == 0) {
                int k = atomicAdd(&d_Out->m_nSeeds, 1); 
                d_Out->m_innerIndex[k] = dSpacepoints->m_index[currentSpiIdx];
                d_Out->m_middleIndex[k] = dSpacepoints->m_index[currentSpmIdx];
                d_Out->m_outerIndex[k] = dSpacepoints->m_index[currentSpoIdx];
                d_Out->m_Q[k] = d_Triplets->m_Q[currentTripletIdx];
            }
            continue;
        }

        __syncthreads();

        for(int otherTripletIdx = threadIdx.x; otherTripletIdx<nTriplets; otherTripletIdx+=blockDim.x) {
            if (otherTripletIdx == currentTripletIdx) continue;

            int otherSpiIdx = d_Triplets->m_innerIndex[otherTripletIdx];
            int otherSpmIdx = d_Triplets->m_middleIndex[otherTripletIdx];
            int otherSpoIdx = d_Triplets->m_outerIndex[otherTripletIdx];

            // Check if triplets are from the same track - share two spacepoints
            if (!((currentSpiIdx == otherSpiIdx && currentSpmIdx == otherSpmIdx) // inner-inner and middle-middle
                ||
                (currentSpiIdx == otherSpiIdx && currentSpoIdx == otherSpmIdx) // inner-inner and outer-middle
                ||
                (currentSpiIdx == otherSpiIdx && currentSpoIdx == otherSpoIdx) // inner-inner and outer-outer
                ||
                (currentSpiIdx == otherSpiIdx && currentSpmIdx == otherSpoIdx) // inner-inner and middle-outer
                ||
                (currentSpiIdx == otherSpmIdx && currentSpmIdx == otherSpoIdx) // inner-middle and middle-outer
                ||
                (currentSpiIdx == otherSpmIdx && currentSpoIdx == otherSpoIdx) // inner-middle and outer-outer
                ||
                (currentSpmIdx == otherSpmIdx && currentSpoIdx == otherSpoIdx) // middle-middle and outer-outer
            )) continue;

            // Triplet from the same track will have the same curvature direction
            if (d_Triplets->m_pT[currentTripletIdx]  * d_Triplets->m_pT[otherTripletIdx] < 0) continue;

            // Triplet from the same track will have the same pt within stddev (based on 1GeV single muon)
            float dPt = std::abs(1./d_Triplets->m_pT[currentTripletIdx] - 1./d_Triplets->m_pT[otherTripletIdx]);
            if (dPt > 0.00015243) continue;

            atomicAdd(&nConfirming, 1); 
        }

        __syncthreads();

        // Save triplet if it has two confirming and is not already saved as a part of another track
        if (threadIdx.x == 0 && nConfirming > 1) {
            int k = atomicAdd(&d_Out->m_nSeeds, 1); 
            d_Out->m_innerIndex[k] = dSpacepoints->m_index[currentSpiIdx];
            d_Out->m_middleIndex[k] = dSpacepoints->m_index[currentSpmIdx];
            d_Out->m_outerIndex[k] = dSpacepoints->m_index[currentSpoIdx];
            d_Out->m_Q[k] = d_Triplets->m_Q[currentTripletIdx];
        }
    }
    
}


#endif