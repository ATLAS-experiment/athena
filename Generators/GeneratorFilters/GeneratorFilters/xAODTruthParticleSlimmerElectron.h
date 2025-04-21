/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERELECTRON_H
#define GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERELECTRON_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthMetaDataContainer.h"
#include "GaudiKernel/SystemOfUnits.h"

/// @brief Algorithm to skim the xAOD truth particle container for xAOD electron filter
///
/// This algorithm is used to copy and skim the particles from the xAOD TruthParticles container,
/// keeping just relevant taus from the event.
/// The design of this class heavily mirrors the DerivationFramework::TruthCollectionMaker.
///
/// @author Jeff Dandoy <Jeff.Dandoy@cern.ch>
class xAODTruthParticleSlimmerElectron : public AthAlgorithm
{
public:
    /// Regular algorithm constructor
    xAODTruthParticleSlimmerElectron(const std::string &name, ISvcLocator *svcLoc);
    /// Function initialising the algorithm
    virtual StatusCode initialize();
    /// Function executing the algorithm
    virtual StatusCode execute();

private:
  SG::ReadHandleKey<xAOD::TruthEventContainer> m_xaodTruthEventContainerName
    {this, "xAODTruthEventContainerName", "TruthEvents"};
  /// The key for the output xAOD truth containers
  SG::WriteHandleKey<xAOD::TruthParticleContainer> m_xaodTruthParticleContainerNameElectron
    {this, "xAODTruthParticleContainerNameElectron","TruthElectrons","Name of Truth Electrons contatiner from the slimmer"};

  /// Selection values for keeping taus and leptons
  DoubleProperty m_el_pt_selection{this, "el_pt_selection", 1. * Gaudi::Units::GeV}; //in GeV
  DoubleProperty m_abseta_selection{this, "abseta_selection", 5.};

}; // class xAODTruthParticleSlimmerElectron

#endif //GENERATORFILTERS_XAODTRUTHPARTICLESLIMMERELECTRON_H
