/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERMUON_H
#define GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERMUON_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthMetaDataContainer.h"

/// @brief Algorithm to skim the xAOD truth particle container for xAOD muons filter
///
/// This algorithm is used to copy and skim the particles from the xAOD TruthParticles container,
/// keeping just relevant muons from the event.
/// The design of this class heavily mirrors the DerivationFramework::TruthCollectionMaker.
///
/// @author Jeff Dandoy <Jeff.Dandoy@cern.ch>
class xAODTruthParticleSlimmerMuon : public AthAlgorithm
{
public:
    /// Regular algorithm constructor
    xAODTruthParticleSlimmerMuon(const std::string &name, ISvcLocator *svcLoc);
    /// Function initialising the algorithm
    virtual StatusCode initialize();
    /// Function executing the algorithm
    virtual StatusCode execute();

private:
    SG::ReadHandleKey<xAOD::TruthEventContainer> m_xaodTruthEventContainerName
    {this,"xAODTruthEventContainerName","TruthEvents","Name of Truth Events container"};
     /// The key for the output xAOD truth containers
    SG::WriteHandleKey<xAOD::TruthParticleContainer> m_xaodTruthParticleContainerNameMuon
    {this, "xAODTruthParticleContainerNameMuon","TruthMuons","Name of Truth Muons contatiner from the slimmer"};

    /// Selection values for keeping muons

}; // class xAODTruthParticleSlimmerMuon

#endif //GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERMUON_H
