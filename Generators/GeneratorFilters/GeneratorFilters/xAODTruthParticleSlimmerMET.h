/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERPHOMET_H
#define GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERPHOMET_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthMetaDataContainer.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

/// @brief Algorithm to skim the xAOD truth particle container for xAOD MET filter
///
/// This algorithm is used to copy and skim the particles from the xAOD TruthParticles container,
/// keeping just relevant MET particles from the event.
/// The design of this class heavily mirrors the DerivationFramework::TruthCollectionMaker.
///
/// @author Jeff Dandoy <Jeff.Dandoy@cern.ch>
class xAODTruthParticleSlimmerMET : public AthAlgorithm
{
public:
    /// Regular algorithm constructor
    xAODTruthParticleSlimmerMET(const std::string &name, ISvcLocator *svcLoc);
    /// Function initialising the algorithm
    virtual StatusCode initialize();
    /// Function executing the algorithm
    virtual StatusCode execute();

private:
  SG::ReadHandleKey<xAOD::TruthEventContainer> m_xaodTruthEventContainerName
  {this, "xAODTruthEventContainerName", "TruthEvents"};
  /// The key for the output xAOD truth containers
  SG::WriteHandleKey<xAOD::TruthParticleContainer> m_xaodTruthParticleContainerNameMET
    {this, "xAODTruthParticleContainerNameMET","TruthMET"};

  PublicToolHandle<IMCTruthClassifier> m_classif{this, "MCTruthClassifier", "MCTruthClassifier/DFCommonTruthClassifier"};
}; // class xAODTruthParticleSlimmerMET

#endif //GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERPHOMET_H
