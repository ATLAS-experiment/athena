/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "AthLinks/ElementLink.h"

#include "GeneratorObjects/xAODTruthParticleLink.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

#include "TruthUtils/HepMCHelpers.h"

#include "GeneratorFilters/xAODTruthParticleSlimmerGen.h"

#include "MCTruthClassifier/IMCTruthClassifier.h"

xAODTruthParticleSlimmerGen::xAODTruthParticleSlimmerGen(const std::string &name, ISvcLocator *svcLoc)
    : AthAlgorithm(name, svcLoc)
    , m_classif("MCTruthClassifier/DFCommonTruthClassifier")
{
    declareProperty("xAODTruthParticleContainerName", m_xaodTruthParticleContainerName = "TruthParticles");
    declareProperty("xAODTruthParticleContainerNameGen", m_xaodTruthParticleContainerNameGen = "TruthGen");
    declareProperty("xAODTruthEventContainerName", m_xaodTruthEventContainerName = "TruthEvents");
}

StatusCode xAODTruthParticleSlimmerGen::initialize()
{
    ATH_MSG_INFO("xAOD input TruthParticleContainer name = " << m_xaodTruthParticleContainerName);
    ATH_MSG_INFO("xAOD output TruthParticleContainerGen name = " << m_xaodTruthParticleContainerNameGen);

    ATH_CHECK(m_classif.retrieve());

    return StatusCode::SUCCESS;
}

StatusCode xAODTruthParticleSlimmerGen::execute()
{
    // If the containers already exists then assume that nothing needs to be done
    if (evtStore()->contains<xAOD::TruthParticleContainer>(m_xaodTruthParticleContainerNameGen))
    {
        ATH_MSG_WARNING("xAOD Gen Truth Particles are already available in the event");
        return StatusCode::SUCCESS;
    }

    // Create new output container
    xAOD::TruthParticleContainer *xTruthParticleContainerGen = new xAOD::TruthParticleContainer();
    CHECK(evtStore()->record(xTruthParticleContainerGen, m_xaodTruthParticleContainerNameGen));
    xAOD::TruthParticleAuxContainer *xTruthParticleAuxContainerGen = new xAOD::TruthParticleAuxContainer();
    CHECK(evtStore()->record(xTruthParticleAuxContainerGen, m_xaodTruthParticleContainerNameGen + "Aux."));
    xTruthParticleContainerGen->setStore(xTruthParticleAuxContainerGen);
    ATH_MSG_INFO("Recorded TruthParticleContainerGen with key: " << m_xaodTruthParticleContainerNameGen);

    // Retrieve full TruthParticle container
    const xAOD::TruthParticleContainer *xTruthParticleContainer;
    if (evtStore()->retrieve(xTruthParticleContainer, m_xaodTruthParticleContainerName).isFailure())
    {
        ATH_MSG_ERROR("No TruthParticle collection with name " << m_xaodTruthParticleContainerName << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    // Retrieve full TruthEventContainer container
    const xAOD::TruthEventContainer *xTruthEventContainer=NULL;
    if (evtStore()->retrieve(xTruthEventContainer, m_xaodTruthEventContainerName).isFailure())
    {
        ATH_MSG_ERROR("No TruthEvent collection with name " << m_xaodTruthEventContainerName << " found in StoreGate!");
        return StatusCode::FAILURE;
    }

    // Loop over full TruthParticle container
    xAOD::TruthEventContainer::const_iterator itr;
    for (itr = xTruthEventContainer->begin(); itr!=xTruthEventContainer->end(); ++itr) {

        unsigned int nPart = (*itr)->nTruthParticles();
        std::vector<int> uniqueID_list;
        int zero_uniqueID=0;
        int dup_uniqueID=0;

        for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
            const xAOD::TruthParticle* theParticle =  (*itr)->truthParticle(iPart);

            const int my_uniqueID = HepMC::uniqueID(theParticle);
            if (my_uniqueID == HepMC::UNDEFINED_ID ) {
                zero_uniqueID++;
                continue;
            }
           bool found = false;
           if (uniqueID_list.size() > 0){
             found = (std::find(uniqueID_list.begin(), uniqueID_list.end(), my_uniqueID) != uniqueID_list.end());
             if(found) {
                       dup_uniqueID++;
                       continue;}
           }
           uniqueID_list.push_back(my_uniqueID);



          xAOD::TruthParticle *xTruthParticle = new xAOD::TruthParticle();
          xTruthParticleContainerGen->push_back( xTruthParticle );
          // Fill with numerical content
          *xTruthParticle=*theParticle;

        }
        if (zero_uniqueID != 0 || dup_uniqueID != 0) ATH_MSG_INFO("Found " << zero_uniqueID << " uniqueID=0 particles and " << dup_uniqueID << " duplicated");
    }

    return StatusCode::SUCCESS;
}


