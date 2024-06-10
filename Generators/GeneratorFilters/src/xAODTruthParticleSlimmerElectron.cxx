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

#include "GeneratorFilters/xAODTruthParticleSlimmerElectron.h"
#include "TruthUtils/HepMCHelpers.h"

xAODTruthParticleSlimmerElectron::xAODTruthParticleSlimmerElectron(const std::string &name, ISvcLocator *svcLoc)
    : AthAlgorithm(name, svcLoc)
{
    declareProperty("xAODTruthParticleContainerName", m_xaodTruthParticleContainerName = "TruthParticles");
    declareProperty("xAODTruthParticleContainerNameElectron", m_xaodTruthParticleContainerNameElectron = "TruthElectrons");
    declareProperty("xAODTruthEventContainerName", m_xaodTruthEventContainerName = "TruthEvents");
    declareProperty("el_pt_selection", m_el_pt_selection = 1. * Gaudi::Units::GeV); //User provides units in MeV!
    declareProperty("abseta_selection", m_abseta_selection = 5.);
}

StatusCode xAODTruthParticleSlimmerElectron::initialize()
{
    ATH_MSG_INFO("xAOD input TruthParticleContainer name = " << m_xaodTruthParticleContainerName);
    ATH_MSG_INFO("xAOD output TruthParticleContainerElectron name = " << m_xaodTruthParticleContainerNameElectron);
    ATH_MSG_INFO("xAOD input xAODTruthEventContainerName name = " << m_xaodTruthEventContainerName);
    return StatusCode::SUCCESS;
}

StatusCode xAODTruthParticleSlimmerElectron::execute()
{
    // If the containers already exists then assume that nothing needs to be done
    if (evtStore()->contains<xAOD::TruthParticleContainer>(m_xaodTruthParticleContainerNameElectron))
    {
        ATH_MSG_WARNING("xAOD Electron Truth Particles are already available in the event");
        return StatusCode::SUCCESS;
    }

    // Create new output container
    xAOD::TruthParticleContainer *xTruthParticleContainerElectron = new xAOD::TruthParticleContainer();
    CHECK(evtStore()->record(xTruthParticleContainerElectron, m_xaodTruthParticleContainerNameElectron));
    xAOD::TruthParticleAuxContainer *xTruthParticleAuxContainerElectron = new xAOD::TruthParticleAuxContainer();
    CHECK(evtStore()->record(xTruthParticleAuxContainerElectron, m_xaodTruthParticleContainerNameElectron + "Aux."));
    xTruthParticleContainerElectron->setStore(xTruthParticleAuxContainerElectron);
    ATH_MSG_INFO("Recorded TruthParticleContainerElectron with key: " << m_xaodTruthParticleContainerNameElectron);

    // Retrieve full TruthEventContainer container
    const xAOD::TruthEventContainer *xTruthEventContainer=NULL;
    if (evtStore()->retrieve(xTruthEventContainer, m_xaodTruthEventContainerName).isFailure())
    {
        ATH_MSG_ERROR("No TruthEvent collection with name " << m_xaodTruthEventContainerName << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    // Set up decorators if needed
    xAOD::TruthEventContainer::const_iterator itr;
    for (itr = xTruthEventContainer->begin(); itr!=xTruthEventContainer->end(); ++itr) {

        unsigned int nPart = (*itr)->nTruthParticles();
        std::vector<int> uniqueID_list;
        int zero_uniqueID=0;
        int dup_uniqueID=0;
        for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
            const xAOD::TruthParticle* theParticle =  (*itr)->truthParticle(iPart);
            int my_uniqueID = HepMC::uniqueID(theParticle);
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
            

            //Save stable Electrons
            if (MC::isStable(theParticle) && MC::isElectron(theParticle))
            {
                xAOD::TruthParticle *xTruthParticle = new xAOD::TruthParticle();
                xTruthParticleContainerElectron->push_back( xTruthParticle );

                // Fill with numerical content
                *xTruthParticle=*theParticle;
            }   
        }
        if (zero_uniqueID!=0 || dup_uniqueID!=0) ATH_MSG_INFO("Found " << zero_uniqueID << " uniqueID=0 particles and " << dup_uniqueID <<" duplicated");
    }

    return StatusCode::SUCCESS;
}
