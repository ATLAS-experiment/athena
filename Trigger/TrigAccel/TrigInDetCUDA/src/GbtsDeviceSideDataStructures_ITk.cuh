/*
	Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_GBTSDEVICESIDEDATASTRUCTURES_ITK_CUH
#define TRIGINDETCUDA_GBTSDEVICESIDEDATASTRUCTURES_ITK_CUH

#include<cuda_fp16.h>
#include <cuda_runtime.h>

struct edgeState { //TO-DO add specialized global mv and shared mv methods 
	
	__device__ inline void initialize(const float4&, const float4&);
	
	//upper triangle of the Cov matrix for the parabola in the x,y plane since symetry gives the rest
	float m_Cx[5]; //(0,0), (0,1), (0,2), (1,1), (1,2), (2,2)
	//Cov matrix for the linear fit of eta and z
	float m_Cy[3]; //(0,0), (0,1), (1,1)

	float m_X[3], m_Y[2];
	float m_c, m_s, m_refX, m_refY; 

	float m_J;

	float m_head_node_type;

	int m_mini_idx;
	int m_edge_idx;
	char m_length;
};

//offsets for the unrolled matrixies in edgeState
static constexpr unsigned char M3_0_0 = 0;
static constexpr unsigned char M3_0_1 = 1;
static constexpr unsigned char M3_0_2 = 2;
static constexpr unsigned char M3_1_1 = 3;
static constexpr unsigned char M3_1_2 = 4;
static constexpr unsigned char M3_2_2 = 5;

static constexpr unsigned char M2_0_0 = 0;
static constexpr unsigned char M2_0_1 = 1;
static constexpr unsigned char M2_1_1 = 2;

struct __align__(8) half4 {
	__half x,y,z,w;
};

inline __device__ __host__ half4 make_half4(const __half x, const __half y, const __half z, const __half w) {
	half4 t;
	t.x = x;
	t.y = y;
	t.z = z;
	t.w = w;
	return t;
}

#endif
