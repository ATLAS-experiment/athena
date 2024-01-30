/*
	Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_DOUBLETCOUNTINGKERNELCUDA_ITK_CUH
#define TRIGINDETCUDA_DOUBLETCOUNTINGKERNELCUDA_ITK_CUH

#include <cuda_runtime.h>

#include "SeedMakingDataStructures_ITk.h"
#include "DoubletHelperFunctionsCuda_ITk.cuh"

__global__ static void doubletCountingKernel_ITk(TrigAccel::ITk::SEED_FINDER_SETTINGS* dSettings, 
							 TrigAccel::ITk::SPACEPOINT_STORAGE* dSpacepoints, 
							 TrigAccel::ITk::DETECTOR_MODEL* dDetModel,
							 DOUBLET_INFO_ITk* d_Info, 
							 int nLayers, int nSlices) {

	__shared__ int spBegin;
	__shared__ int spEnd;
	__shared__ int nMiddleSPs;
	
	__shared__ int nInner[NUM_MIDDLE_THREADS_ITk];
	__shared__ int nOuter[NUM_MIDDLE_THREADS_ITk];

	const float zTolerance = 3.0; 
	const float maxEta = dSettings->m_maxEta;
	const float maxDoubletLength = dSettings->m_maxDoubletLength;
	const float minDoubletLength = dSettings->m_minDoubletLength;
	const float maxOuterRadius = 550.0;
	 
	const int sliceIdx = blockIdx.x;
	const int layerIdx = dDetModel->m_middleSpacePointLayers[blockIdx.y];

	const TrigAccel::ITk::SILICON_LAYER& layerGeo =  dDetModel->m_layers[layerIdx];
	bool isBarrel = (layerGeo.m_type == 0);

	if(threadIdx.x == 0 && threadIdx.y == 0) {
		const TrigAccel::ITk::SPACEPOINT_LAYER_RANGE& slr = dSpacepoints->m_phiSlices[sliceIdx];
		spBegin = slr.m_layerBegin[layerIdx];
		spEnd = slr.m_layerEnd[layerIdx];
		nMiddleSPs = spEnd-spBegin;
	}
	__syncthreads();

	if(nMiddleSPs == 0) return;

	const float zMinus = dSettings->m_zedMinus - zTolerance; 
	const float zPlus  = dSettings->m_zedPlus  + zTolerance; 
	const float maxTheta = 2*atan(exp(-maxEta));
	const float maxCtg = cos(maxTheta)/sin(maxTheta);
	const float minZ0     = dSettings->m_zedMinus;
	const float maxZ0     = dSettings->m_zedPlus;
	const float minOuterZ = minZ0 - maxOuterRadius*maxCtg - zTolerance; 
	const float maxOuterZ = maxZ0 + maxOuterRadius*maxCtg + zTolerance;

	const double ptCoeff = 0.29997*dSettings->m_magFieldZ/2.0;// ~0.3*B/2 - assumes nominal field of 2*T
	const float tripletPtMin = dSettings->m_tripletPtMin; // Retrieve from settings
	const float minR_squ = tripletPtMin*tripletPtMin/std::pow(ptCoeff,2);
	const float maxKappa_high_eta          = 0.8/minR_squ;
	const float maxKappa_low_eta           = 0.6/minR_squ;

	//1. get a tile of middle spacepoints

	for(int spmIdx=threadIdx.x+spBegin;spmIdx<spEnd;spmIdx+=blockDim.x) {
 
		float zm = dSpacepoints->m_z[spmIdx];
		float rm = dSpacepoints->m_r[spmIdx];

		float minTau = 0;
		float maxTau = 100;

		if (isBarrel) {
			float clusterWidth = dSpacepoints->m_clusterWidth[spmIdx];
			minTau = 6.7*(clusterWidth - 0.2);
			maxTau = 1.6 + 0.15/(clusterWidth + 0.2) + 6.1*(clusterWidth - 0.2);
		}

		if(threadIdx.y ==0) {
			nInner[threadIdx.x] = 0;
			nOuter[threadIdx.x] = 0; 
			d_Info->m_nInner[spmIdx] = 0;
			d_Info->m_nOuter[spmIdx] = 0;
		}

		__syncthreads();

		//2. loop over other phi-bins / layers

		for(int deltaPhiIdx=-1;deltaPhiIdx<=1;deltaPhiIdx++) {
			int nextPhiIdx = sliceIdx + deltaPhiIdx;
			if(nextPhiIdx>=nSlices) nextPhiIdx = 0;
			if(nextPhiIdx<0) nextPhiIdx=nSlices-1;
			const TrigAccel::ITk::SPACEPOINT_LAYER_RANGE& next_slr = dSpacepoints->m_phiSlices[nextPhiIdx];

			for(int nextLayerIdx=0;nextLayerIdx<nLayers;nextLayerIdx++) {
				if(nextLayerIdx == layerIdx) continue;

				int next_spBegin = next_slr.m_layerBegin[nextLayerIdx];
				int next_spEnd = next_slr.m_layerEnd[nextLayerIdx];

				if(next_spEnd == next_spBegin) continue;//no spacepoints in this layer

				// Check if SPs from the nextLayer (currently processed layer) and the current SP would exceed the maximum doublet length
				const TrigAccel::ITk::SILICON_LAYER& layerGeoSp =  dDetModel->m_layers[nextLayerIdx];
				bool isBarrelSp = (layerGeoSp.m_type == 0);
				float refCoord = layerGeoSp.m_refCoord; // Position of the layer in r (barrel) or z (end cap)
				if(isBarrelSp && std::abs(refCoord-rm)>maxDoubletLength) continue;

				// Calculate the nextLayer boundaries by projecting the current spacepoint within the beamspot limits
				float minCoord = 10000.0;
				float maxCoord =-10000.0;

				if(isBarrelSp) {
					minCoord = zMinus + refCoord*(zm-zMinus)/rm;
					maxCoord = zPlus + refCoord*(zm-zPlus)/rm;
				}
				else {
					minCoord = rm*(refCoord-zMinus)/(zm-zMinus);
					maxCoord = rm*(refCoord-zPlus)/(zm-zPlus);
				}

				if(minCoord>maxCoord) {
					float tmp = maxCoord;maxCoord = minCoord;minCoord = tmp;
				}

				if(layerGeoSp.m_maxBound<minCoord || layerGeoSp.m_minBound>maxCoord) continue;

				//3. get a tile of inner/outer spacepoints

				for(int spIdx=threadIdx.y+next_spBegin;spIdx<next_spEnd;spIdx+=blockDim.y) {

					float zsp = dSpacepoints->m_z[spIdx];
					float rsp = dSpacepoints->m_r[spIdx];

					// Check if SP is within the doublet boundaries
					float spCoord = (isBarrelSp) ? zsp : rsp;
					if(spCoord<minCoord || spCoord>maxCoord) continue;

					float dr = rsp - rm;
					float dz = zsp - zm;

					// Cut on doublet length
					float dL = std::sqrt( dr*dr + dz*dz);
					float maxDL = GPUTrackSeedingItkHelpers::getMaxDeltaLEta(GPUTrackSeedingItkHelpers::getEta(dr, dz, dL));
					if(std::abs(dL)>maxDL || std::abs(dL)<minDoubletLength) continue;
								
					// Cut on tau = cot(theta)
					float tau = dz/dr;
					float ftau = std::abs(tau);
					if(ftau>maxCtg) continue;

					// Cut on pixel width
					if (isBarrelSp) {
						if(ftau < minTau || ftau > maxTau) continue;
					}

					// Cut on Z
					float z0 = zsp - rsp*tau;
					if(z0 < minZ0 || z0 > maxZ0) continue;

					float outZ = zsp + (maxOuterRadius-rsp)*tau; 
					if(outZ < minOuterZ || outZ > maxOuterZ) continue;

					// Cut on curvature
					float xm = dSpacepoints->m_x[spmIdx];
					float ym = dSpacepoints->m_y[spmIdx];
					float xsp = dSpacepoints->m_x[spIdx];
					float ysp = dSpacepoints->m_y[spIdx];

					float dx = xsp - xm;
					float dy = ysp - ym;
					float L2 = 1/(dx*dx+dy*dy);
					float D = (ysp*xm - ym*xsp)/(rm*rsp);
					float kappa = D*D*L2;

					if (std::abs(tau) < 4.0) {//eta = 2.1
						if (kappa > maxKappa_low_eta) {
							continue;
						}
					} else {
						if (kappa > maxKappa_high_eta) {
							continue;
						}
					}

					if(dr > 0) //outer doublet
						atomicAdd(&nOuter[threadIdx.x],1);
					else 
						atomicAdd(&nInner[threadIdx.x],1);
				}
			}
		}

		__syncthreads();
				 
		if(threadIdx.y == 0) {
			d_Info->m_nInner[spmIdx] = nInner[threadIdx.x];
			d_Info->m_nOuter[spmIdx] = nOuter[threadIdx.x];
			d_Info->m_good[spmIdx] =  (nInner[threadIdx.x] > 0 && nOuter[threadIdx.x] > 0) ? 1 : 0; 
		}
		__syncthreads();
	}

}

#endif
