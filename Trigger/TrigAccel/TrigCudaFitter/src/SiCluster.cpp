// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#include "Surface.h"
#include "SiCluster.h"

SiCluster::SiCluster(const Surface* pS) : m_pSurface(pS)
{
}

SiCluster::~SiCluster(void)
{
	delete m_pSurface;
}


