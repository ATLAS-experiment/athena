// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#include "Surface.h"
#include "SCT_EndCapCluster.h"
#include "TrkBaseNode.h"
#include "TrkFilteringNodes.h"

SCT_EndCapCluster::SCT_EndCapCluster(std::unique_ptr<const Surface> pS) : SCT_Cluster(std::move(pS))
{
}

void SCT_EndCapCluster::setParameters(float* par)
{
	m_R=par[0];
	m_m=par[1];
	m_cov=par[2];
}

TrkBaseNode* SCT_EndCapCluster::createDkfNode(void) const
{
	double cov,m,R;
	cov=m_cov;
	m=m_m;
	R=m_R;

	TrkPlanarSurface* pS = m_pSurface->createDkfSurface();
	TrkBaseNode* pN = new TrkEndCapClusterNode(pS,200.0f,R,m,cov);
	return pN;
}
