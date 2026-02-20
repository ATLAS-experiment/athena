// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#include "Surface.h"
#include "SCT_BarrelCluster.h"
#include "TrkBaseNode.h"
#include "TrkFilteringNodes.h"

SCT_BarrelCluster::SCT_BarrelCluster(std::unique_ptr<const Surface> pS) : SCT_Cluster(std::move(pS))
{
}

void SCT_BarrelCluster::setParameters(float* par)
{
	m_m=par[0];m_cov=par[1];
}

TrkBaseNode* SCT_BarrelCluster::createDkfNode(void) const
{
	double cov,m;
	cov=m_cov;
	m=m_m;

	TrkPlanarSurface* pS = m_pSurface->createDkfSurface();
	TrkBaseNode* pN = new TrkClusterNode(pS,200.0f,m,cov);
	return pN;
}
