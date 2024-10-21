/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "InDetPerformanceMonitoring/PerfMonServices.h"

// Names of the various containers.
const std::string PerfMonServices::s_sContainerNames[NUM_CONTAINERS] = { "MuidMuonCollection",
									 "StacoMuonCollection",
									 "Muons",
									 "Electrons",// "ElectronCllection"
									 "PhotonCollection",
									 "MET_Final",
									 "TrackParticleCandidate",
									 //	 "VxPrimaryCandidate"};
									 "PrimaryVertices"};
const std::string PerfMonServices::s_sAtlfastContainerNames[NUM_CONTAINERS] = { "AtlfastMuonCollection",
										 "AtlfastMuonCollection",
										 "AtlfastMuons",
										 "AtlfastElectronCollection",
										 "AtlfastPhotonCollection",
										 "AtlfastMissingEt",
										 "Placeholder"                };

//===========================================================================
// Constructors & destructors
//===========================================================================

PerfMonServices::PerfMonServices()
{
}

PerfMonServices::~PerfMonServices()
{
}
