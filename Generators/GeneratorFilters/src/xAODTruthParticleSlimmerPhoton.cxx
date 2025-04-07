/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

#include "GeneratorFilters/xAODTruthParticleSlimmerPhoton.h"
#include "TruthUtils/HepMCHelpers.h"

xAODTruthParticleSlimmerPhoton::xAODTruthParticleSlimmerPhoton(const std::string &name, ISvcLocator *svcLoc)
    : AthAlgorithm(name, svcLoc)
{
}

StatusCode xAODTruthParticleSlimmerPhoton::initialize()
{
  ATH_CHECK(m_xaodTruthParticleContainerNamePhoton.initialize());
  ATH_MSG_INFO("xAOD output TruthParticleContainerPhoton name = " << m_xaodTruthParticleContainerNamePhoton.key());
  ATH_CHECK(m_xaodTruthEventContainerName.initialize());
  ATH_MSG_INFO("xAOD input xAODTruthEventContainerName name = " << m_xaodTruthEventContainerName.key());
  return StatusCode::SUCCESS;
}

StatusCode xAODTruthParticleSlimmerPhoton::execute()
{
  // If the containers already exists then assume that nothing needs to be done
  if (evtStore()->contains<xAOD::TruthParticleContainer>(m_xaodTruthParticleContainerNamePhoton.key()))
    {
      ATH_MSG_WARNING("xAOD Photon Truth Particles are already available in the event");
      return StatusCode::SUCCESS;
    }

  // Create new output container
  SG::WriteHandle<xAOD::TruthParticleContainer> xTruthParticleContainerPhoton(m_xaodTruthParticleContainerNamePhoton);
  ATH_CHECK(xTruthParticleContainerPhoton.record(std::make_unique<xAOD::TruthParticleContainer>(), std::make_unique<xAOD::TruthParticleAuxContainer>()));
  ATH_MSG_INFO("Recorded TruthParticleContainerPhoton with key: " << m_xaodTruthParticleContainerNamePhoton.key());

  // Retrieve full TruthEventContainer container
  SG::ReadHandle<xAOD::TruthEventContainer> xTruthEventContainer{m_xaodTruthEventContainerName};
  if ( !xTruthEventContainer.isValid() )
    {
      ATH_MSG_ERROR("No TruthEvent collection with name " << m_xaodTruthEventContainerName.key() << " found in StoreGate!");
      return StatusCode::FAILURE;
    }
    // Set up decorators if needed
    xAOD::TruthEventContainer::const_iterator itr;
    for (itr = xTruthEventContainer->begin(); itr!=xTruthEventContainer->end(); ++itr) {

        std::vector<int> uniqueID_list;  
        int zero_uniqueID=0;
        int dup_uniqueID = 0;

        unsigned int nPart = (*itr)->nTruthParticles();

        for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
            const xAOD::TruthParticle* theParticle =  (*itr)->truthParticle(iPart);
             
            int my_uniqueID = HepMC::uniqueID(theParticle);
            if ( my_uniqueID == HepMC::UNDEFINED_ID ) {
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
          

            //Save stable Photons
            if (MC::isStable(theParticle) && MC::isPhoton(theParticle))
            {
                xAOD::TruthParticle *xTruthParticle = new xAOD::TruthParticle();
                xTruthParticleContainerPhoton->push_back( xTruthParticle );

                // Fill with numerical content
                *xTruthParticle=*theParticle;
            }   
        }
     if (zero_uniqueID != 0 || dup_uniqueID != 0) ATH_MSG_INFO("Found " << zero_uniqueID << " uniqueID zero particles and  " <<dup_uniqueID<< " duplicated");
    }

    return StatusCode::SUCCESS;
}
