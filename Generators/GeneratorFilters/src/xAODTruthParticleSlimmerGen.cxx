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

#include "TruthUtils/HepMCHelpers.h"

#include "GeneratorFilters/xAODTruthParticleSlimmerGen.h"

#include "MCTruthClassifier/IMCTruthClassifier.h"

xAODTruthParticleSlimmerGen::xAODTruthParticleSlimmerGen(const std::string &name, ISvcLocator *svcLoc)
    : AthAlgorithm(name, svcLoc)
{
}

StatusCode xAODTruthParticleSlimmerGen::initialize()
{
  ATH_CHECK(m_xaodTruthParticleContainerNameGen.initialize());
  ATH_MSG_INFO("xAOD output TruthParticleContainerGen name = " << m_xaodTruthParticleContainerNameGen.key());
  ATH_CHECK(m_xaodTruthEventContainerName.initialize());
  ATH_CHECK(m_classif.retrieve());

    return StatusCode::SUCCESS;
}

StatusCode xAODTruthParticleSlimmerGen::execute()
{
  const EventContext& ctx = Gaudi::Hive::currentContext(); // As not re-entrant
    // Create new output container
    SG::WriteHandle<xAOD::TruthParticleContainer> xTruthParticleContainerGen(m_xaodTruthParticleContainerNameGen, ctx);
    ATH_CHECK(xTruthParticleContainerGen.record(std::make_unique<xAOD::TruthParticleContainer>(),
                                                 std::make_unique<xAOD::TruthParticleAuxContainer>()));
    ATH_MSG_DEBUG("Recorded TruthParticleContainerGen with key: " << m_xaodTruthParticleContainerNameGen.key());

    // Retrieve full TruthEventContainer container
    SG::ReadHandle<xAOD::TruthEventContainer> xTruthEventContainer{m_xaodTruthEventContainerName};
    if ( !xTruthEventContainer.isValid() )
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
        if (zero_uniqueID != 0 || dup_uniqueID != 0) ATH_MSG_DEBUG("Found " << zero_uniqueID << " uniqueID=0 particles and " << dup_uniqueID << " duplicated");
    }

    return StatusCode::SUCCESS;
}


