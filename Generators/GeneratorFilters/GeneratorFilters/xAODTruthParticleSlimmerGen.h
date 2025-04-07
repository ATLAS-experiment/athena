/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERPHOGEN_H
#define GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERPHOGEN_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthMetaDataContainer.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

/// @brief Algorithm to skim the xAOD truth particle container for generators filters
///        in fact we want to remove barcode 0 particles and particles with duplicated barcodes
/// This algorithm is used to copy and skim the particles from the xAOD TruthParticles container,
/// The design of this class heavily mirrors the DerivationFramework::TruthCollectionMaker.
///
class xAODTruthParticleSlimmerGen : public AthAlgorithm
{
public:
    /// Regular algorithm constructor
    xAODTruthParticleSlimmerGen(const std::string &name, ISvcLocator *svcLoc);
    /// Function initialising the algorithm
    virtual StatusCode initialize();
    /// Function executing the algorithm
    virtual StatusCode execute();

private:
    /// The key for the output xAOD truth containers
  SG::WriteHandleKey<xAOD::TruthParticleContainer> m_xaodTruthParticleContainerNameGen{this, "xAODTruthParticleContainerNameGen",  "TruthGen"};
  SG::ReadHandleKey<xAOD::TruthEventContainer> m_xaodTruthEventContainerName{this, "xAODTruthEventContainerName", "TruthEvents"};

  PublicToolHandle<IMCTruthClassifier> m_classif{this, "MCTruthClassifier", "MCTruthClassifier/DFCommonTruthClassifier"};
}; // class xAODTruthParticleSlimmerGen

#endif //GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERPHOGEN_H
