/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "AthLinks/ElementLink.h"

#include "GeneratorObjects/McEventCollection.h"
#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "TruthUtils/HepMCHelpers.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

#include "MCTruthClassifier/IMCTruthClassifier.h"

#include "GeneratorFilters/xAODTruthParticleSlimmerTau.h"
#include "GeneratorFilters/Common.h"

using namespace std;

xAODTruthParticleSlimmerTau::xAODTruthParticleSlimmerTau(const string &name, ISvcLocator *svcLoc)
    : AthAlgorithm(name, svcLoc)
{
}

StatusCode xAODTruthParticleSlimmerTau::initialize()
{
  ATH_CHECK(m_classifier.retrieve());
  ATH_CHECK(m_xaodTruthParticleContainerName.initialize());
  ATH_MSG_INFO("xAOD input TruthParticleContainer name = " << m_xaodTruthParticleContainerName.key());
  ATH_CHECK(m_xaodTruthTauParticleContainerName.initialize());
  ATH_MSG_INFO("xAOD output TruthTauParticleContainer name = " << m_xaodTruthTauParticleContainerName.key());
  return StatusCode::SUCCESS;
}

CLHEP::HepLorentzVector xAODTruthParticleSlimmerTau::sumDaughterNeutrinos(const xAOD::TruthParticle *part)
{
  CLHEP::HepLorentzVector nu(0, 0, 0, 0);
  if (MC::isSMNeutrino(part) && MC::isPhysical(part))
  {
    nu.setPx(part->px());
    nu.setPy(part->py());
    nu.setPz(part->pz());
    nu.setE(part->e());
  }
  if (!part->hasDecayVtx())
    return nu;

  for (size_t n = 0; n < part->nChildren(); ++n)
    nu += sumDaughterNeutrinos(part->child(n));

  return nu;
}

StatusCode xAODTruthParticleSlimmerTau::execute()
{

  CLHEP::HepLorentzVector nutau;

  // If the containers already exists then assume that nothing needs to be done
  if (evtStore()->contains<xAOD::TruthParticleContainer>(m_xaodTruthTauParticleContainerName.key()))
  {
    ATH_MSG_WARNING("xAOD Tau Truth Particles are already available in the event");
    return StatusCode::SUCCESS;
  }

  // Create new output container
  SG::WriteHandle<xAOD::TruthParticleContainer> xTruthTauParticleContainer(m_xaodTruthTauParticleContainerName);
  ATH_CHECK(xTruthTauParticleContainer.record(std::make_unique<xAOD::TruthParticleContainer>(), std::make_unique<xAOD::TruthParticleAuxContainer>()));
  ATH_MSG_INFO("Recorded TruthTauParticleContainer with key: " << m_xaodTruthTauParticleContainerName.key());

  // Retrieve full TruthParticle container
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_xaodTruthParticleContainerName};
  if ( !xTruthParticleContainer.isValid() )
    {
      ATH_MSG_ERROR("No TruthParticle collection with name " << m_xaodTruthParticleContainerName.key() << " found in StoreGate!");
      return StatusCode::FAILURE;
    }

  // Set up decorators

  const static SG::AuxElement::Decorator<unsigned int> originDecorator("classifierParticleOrigin");
  const static SG::AuxElement::Decorator<unsigned int> typeDecorator("classifierParticleType");
  const static SG::AuxElement::Decorator<unsigned int> outcomeDecorator("classifierParticleOutCome");
  const static SG::AuxElement::Decorator<unsigned int> classificationDecorator("Classification");
  const static SG::AuxElement::Decorator<int> parenthadronPIDDecorator("parentHadronID");

  // sum of neutrinos 4-vector in tau decay products
  const static SG::AuxElement::Decorator<CLHEP::HepLorentzVector> nuDecorator("nuVector");
  const static SG::AuxElement::Decorator<int> tauTypeDecorator("tauType");

  // Loop over full TruthParticle container
  
  std::vector<int> uniqueID_list;  
  int zero_uniqueID=0;
  int dup_uniqueID = 0;

  unsigned int nParticles = xTruthParticleContainer->size();

  for (unsigned int iPart = 0; iPart < nParticles; ++iPart)
  {
    ElementLink<xAOD::TruthParticleContainer> eltp(*xTruthParticleContainer, iPart);
    const xAOD::TruthParticle *theParticle = (*xTruthParticleContainer)[iPart];
             
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

    float this_abseta = theParticle->abseta();
    float this_pt = theParticle->pt();


    //Save Taus above 0.001 GeV, & with any eta (may be changed on JOs level eg. to dectector acceptance of eta 4.5)
    // see GeneratorFilters/share/common/xAODTauFilter_Common.py
    // we want to avoid status 3 taus
    if (MC::isPhysical(theParticle) && MC::isTau(theParticle) && this_pt >= m_tau_pt_selection && this_abseta < m_abseta_selection)
    {
      xAOD::TruthParticle *xTruthParticle = new xAOD::TruthParticle();

      xTruthTauParticleContainer->push_back(xTruthParticle);
      // Fill with numerical content
      *xTruthParticle=*theParticle;
      const xAOD::TruthParticle *tau = theParticle;
      nutau = sumDaughterNeutrinos(tau);

      nuDecorator(*xTruthParticle) = nutau;

      int tauType = 0;
      for (size_t n = 0; n < tau->nChildren(); ++n)
      {
        if (tau->child(n)->absPdgId() == MC::NU_E)
          tauType = 1; //Tau decays into an electron
        else if (tau->child(n)->absPdgId() == MC::NU_MU)
          tauType = 2; //Tau decays into a muon
        else if (MC::isTau(tau->child(n)))
          tauType = 11; //Tau radiates a particle and decays into another tau
      }
      tauTypeDecorator(*xTruthParticle) = tauType;

unsigned int particleOutCome;
unsigned int result;
unsigned int particleType;
unsigned int particleOrigin;
int hadron_pdg;
Common::classify(m_classifier,theParticle,particleOutCome,result,hadron_pdg,particleType,particleOrigin );
      typeDecorator(*xTruthParticle) = particleType;
      originDecorator(*xTruthParticle) = particleOrigin;
      outcomeDecorator(*xTruthParticle) = particleOutCome;

      classificationDecorator(*xTruthParticle) = result;
      parenthadronPIDDecorator(*xTruthParticle) = hadron_pdg;
    }

  } //end of loop over particles
  if(zero_uniqueID!=0 || dup_uniqueID != 0) ATH_MSG_INFO("Found " << zero_uniqueID << " uniqueID=0 particles and " << dup_uniqueID << " duplicated");
  return StatusCode::SUCCESS;
}
