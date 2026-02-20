// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#include "SiCluster.h"

SiCluster::SiCluster(std::unique_ptr<const Surface> pS) : m_pSurface(std::move(pS))
{
}
