// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#include "Surface.h"
#include "PixelCluster.h"
#include "TrkBaseNode.h"
#include "TrkFilteringNodes.h"

PixelCluster::PixelCluster(std::unique_ptr<const Surface> pS) : SiCluster(std::move(pS))
{
}

void PixelCluster::setParameters(float* par)
{
	m_m[0]=par[0];m_m[1]=par[1];
	m_cov[0][0]=par[2];
	m_cov[1][1]=par[3];
	m_cov[0][1]=m_cov[1][0]=0.0;
}

TrkBaseNode* PixelCluster::createDkfNode(void) const
{
	double cov[4],m[2];
	cov[0]=m_cov[0][0];
	cov[3]=m_cov[1][1];
	cov[1]=cov[2]=0.0;
	m[0]=m_m[0];m[1]=m_m[1];
	TrkPlanarSurface* pS = m_pSurface->createDkfSurface();
	TrkBaseNode* pN = new TrkPixelNode(pS,200.0,m,cov);
	return pN;
}
