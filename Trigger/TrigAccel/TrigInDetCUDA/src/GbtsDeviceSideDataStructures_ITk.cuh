/*
	Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_GBTSDEVICESIDEDATASTRUCTURES_ITK_CUH
#define TRIGINDETCUDA_GBTSDEVICESIDEDATASTRUCTURES_ITK_CUH

struct edgeState { //TO-DO add specialized global mv and shared mv methods 
	
	__device__ inline void initialize(const float4&, const float4&);

	float m_Cx[3][3], m_Cy[2][2];	

	float m_X[3], m_Y[2];
	float m_c, m_s, m_refX, m_refY; 

	float m_J;

	float m_head_node_type;

	int m_mini_idx;
	int m_edge_idx;
	int m_length;
};

#endif
