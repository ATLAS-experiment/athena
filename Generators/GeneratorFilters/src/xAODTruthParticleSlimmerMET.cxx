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

#include "GeneratorFilters/xAODTruthParticleSlimmerMET.h"
#include "GeneratorFilters/Common.h"

#include "MCTruthClassifier/IMCTruthClassifier.h"

xAODTruthParticleSlimmerMET::xAODTruthParticleSlimmerMET(const std::string &name, ISvcLocator *svcLoc)
    : AthAlgorithm(name, svcLoc)
{
}

StatusCode xAODTruthParticleSlimmerMET::initialize()
{
  ATH_CHECK(m_xaodTruthParticleContainerNameMET.initialize());
  ATH_MSG_INFO("xAOD output TruthParticleContainerMET name = " << m_xaodTruthParticleContainerNameMET.key());
  ATH_CHECK(m_xaodTruthEventContainerName.initialize());
  ATH_MSG_INFO("xAOD input xAODTruthEventContainerName name = " << m_xaodTruthEventContainerName.key());
  ATH_CHECK(m_classif.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode xAODTruthParticleSlimmerMET::execute()
{
  // If the containers already exists then assume that nothing needs to be done
  if (evtStore()->contains<xAOD::TruthParticleContainer>(m_xaodTruthParticleContainerNameMET.key()))
    {
      ATH_MSG_WARNING("xAOD MET Truth Particles are already available in the event");
      return StatusCode::SUCCESS;
    }

  // Create new output container
  SG::WriteHandle<xAOD::TruthParticleContainer> xTruthParticleContainerMET(m_xaodTruthParticleContainerNameMET);
  ATH_CHECK(xTruthParticleContainerMET.record(std::make_unique<xAOD::TruthParticleContainer>(), std::make_unique<xAOD::TruthParticleAuxContainer>()));
  ATH_MSG_INFO("Recorded TruthParticleContainerMET with key: " << m_xaodTruthParticleContainerNameMET.key());

  // Retrieve full TruthEventContainer container
  SG::ReadHandle<xAOD::TruthEventContainer> xTruthEventContainer{m_xaodTruthEventContainerName};
  if ( !xTruthEventContainer.isValid() )
    {
      ATH_MSG_ERROR("No TruthEvent collection with name " << m_xaodTruthEventContainerName.key() << " found in StoreGate!");
      return StatusCode::FAILURE;
    }

    // Set up decorators if needed
    const static SG::AuxElement::Decorator<bool> isPrompt("isPrompt");

    // Loop over full TruthParticle container
    xAOD::TruthEventContainer::const_iterator itr;
    for (itr = xTruthEventContainer->begin(); itr!=xTruthEventContainer->end(); ++itr) {

        unsigned int nPart = (*itr)->nTruthParticles();
        std::vector<int> uniqueID_list;
        int zero_uniqueID=0;
        int dup_uniqueID=0;

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
            


          // stable and non-interacting, implemented from DerivationFramework 
          //https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/DerivationFramework/DerivationFrameworkMCTruth/python/MCTruthCommon.py#L183
          // which in turn use the implementation from Reconstruction
          //https://gitlab.cern.ch/atlas/athena/blob/21.0/Reconstruction/MET/METReconstruction/Root/METTruthTool.cxx#L143
          if (!theParticle->isGenStable()) continue;
          if (MC::isInteracting(theParticle)) continue;


          xAOD::TruthParticle *xTruthParticle = new xAOD::TruthParticle();
          xTruthParticleContainerMET->push_back( xTruthParticle );

          // Fill with numerical content
          *xTruthParticle=*theParticle;

          //Decorate
          isPrompt(*xTruthParticle) = Common::prompt(theParticle,m_classif);
        }
        if(zero_uniqueID != 0 || dup_uniqueID !=0 ) ATH_MSG_INFO("Found " << zero_uniqueID << " uniqueID=0 particles and " <<dup_uniqueID <<"duplicated");
    }

    return StatusCode::SUCCESS;
}


