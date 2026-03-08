///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// METTruthTool.cxx 
// Implementation file for class METTruthTool
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo, AS Mete
/////////////////////////////////////////////////////////////////// 

// METReconstruction includes
#include "METReconstruction/METTruthTool.h"

// MET EDM
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETAuxContainer.h"
#include "xAODMissingET/MissingETAuxComponentMap.h"

// Truth EDM
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"

// Base EDM
#include "AthContainers/ConstDataVector.h"

// Truth Utilities
#include "TruthUtils/HepMCHelpers.h"

namespace met {

  using std::vector;
  //
  using xAOD::IParticle;
  //
  using xAOD::TruthEvent;
  using xAOD::TruthEventContainer;
  using xAOD::TruthParticle;
  using xAOD::TruthParticleContainer;
  //
  using xAOD::MissingET;
  using xAOD::MissingETComposition;
  using xAOD::MissingETComponentMap;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 

  // Constructors
  ////////////////
  METTruthTool::METTruthTool(const std::string& name) : 
    AsgTool(name),
    METBuilderTool(name)
  {
  }

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode METTruthTool::initialize()
  {
    ATH_CHECK( METBuilderTool::initialize() );
    ATH_MSG_VERBOSE ("Initializing " << name() << "...");

    if(m_inputType=="NonInt") {
      m_truth_type = MissingETBase::Source::TruthType::NonInt;
    } else if(m_inputType=="Int") {
      m_truth_type = MissingETBase::Source::TruthType::Int;
    } else if(m_inputType=="IntOut") {
      m_truth_type = MissingETBase::Source::TruthType::IntOut;
    } else if(m_inputType=="IntMuons") {
      m_truth_type = MissingETBase::Source::TruthType::TruthMuons;
    } else {
      ATH_MSG_FATAL("Invalid input type provided");
      return StatusCode::FAILURE;
    }

    ATH_CHECK( m_truthEventKey.initialize());

    return StatusCode::SUCCESS;
  }

  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 

  bool METTruthTool::accept(const xAOD::IParticle* object) const
  {
    // Apply cuts 
    ATH_MSG_VERBOSE("Check if truth particle is accepted");
    if(!object){return false;}
    if(object->type() != xAOD::Type::TruthParticle) { 
      ATH_MSG_WARNING("METTruthTool::accept given an object of type" << object->type());
      return false;
    }
    const xAOD::TruthParticle* truth = static_cast<const xAOD::TruthParticle*>(object);

    if(truth->pt()<1e-9) return false; // reject particles with no pt
    ATH_MSG_VERBOSE("My truth type: " << static_cast<MissingETBase::Types::bitmask_t>(m_truth_type));
    switch(m_truth_type) {
    case MissingETBase::Source::TruthType::NonInt:
      return accept_nonint(truth);
    case MissingETBase::Source::TruthType::Int:
      return accept_int(truth);
    case MissingETBase::Source::TruthType::IntOut:
      return accept_intout(truth);
    case MissingETBase::Source::TruthType::TruthMuons:
      return accept_intmuons(truth);
    default:
      return false;
    }
    // should not get here
    return false;
  }

  bool METTruthTool::accept_nonint(const xAOD::TruthParticle* truth) const
  {
    ATH_MSG_VERBOSE("Check nonint");
    // stable and non-interacting
    return truth->isGenStable() && !MC::isInteracting(truth->pdgId());
  }

  bool METTruthTool::accept_int(const xAOD::TruthParticle* truth) const
  {
    ATH_MSG_VERBOSE("Check int");
    // not muon
    if(truth->isMuon()) return false;
    // stable
    if(!truth->isGenStable()) return false;
    // interacting
    if(!MC::isInteracting(truth->pdgId())) return false;
    // in acceptance
    if(fabs(truth->eta())>m_det_maxEta) return false;

    return true;
  }
  
  bool METTruthTool::accept_intout(const xAOD::TruthParticle* truth) const
  {
    ATH_MSG_VERBOSE("Check intout");
    // muon outside MS acceptance
    if( truth->isMuon() ) {
      if( fabs(truth->eta())<m_truthmu_maxEta) return false;
    } else {
      // other particle outside calo acceptance
      if( (fabs(truth->eta())<m_det_maxEta) ) return false;
    }
    // stable
    if(!truth->isGenStable()) return false;
    // interacting
    if(!MC::isInteracting(truth->pdgId())) return false;

    return true;
  }

  bool METTruthTool::accept_intmuons(const xAOD::TruthParticle* truth) const
  {
    ATH_MSG_VERBOSE("Check intmuon");
    // muon
    if(!truth->isMuon()) return false;
    // stable
    if(!truth->isGenStable()) return false;
    // in acceptance
    if(truth->pt()<m_truthmu_minPt || fabs(truth->eta())>m_truthmu_maxEta) return false;

    return true;
  }

  StatusCode METTruthTool::executeTool(xAOD::MissingET* metTerm, xAOD::MissingETComponentMap* metMap) const {

    ATH_MSG_DEBUG ("In execute: " << name() << "...");

    metTerm->setSource(static_cast<MissingETBase::Types::bitmask_t>(m_truth_type));

    // Retrieve the truth container
    SG::ReadHandle<xAOD::TruthEventContainer> truthEvents(m_truthEventKey);

    if (!truthEvents.isValid()) {
      ATH_MSG_WARNING("Unable to retrieve input truth event container ");
      return StatusCode::SUCCESS;
    }

    // First truth event is the hard scatter
    const TruthEvent* hsevent = truthEvents->front();
    ConstDataVector<TruthParticleContainer> truthCont(SG::VIEW_ELEMENTS);
    for(size_t itp=0; itp<hsevent->nTruthParticles(); ++itp) {
      truthCont.push_back(hsevent->truthParticle(itp));
      ATH_MSG_VERBOSE("Extracted truth particle with index " << hsevent->truthParticle(itp)->index());
    }

    vector<const IParticle*> signalList;
    signalList.reserve(truthCont.size());
    // Loop over all truth particles
    for( const auto truthp : truthCont ) {
      // Check if truth particles satisfies the requirements
      if( this->accept(truthp) ) {
	// Add the selected truth particles to the list
	signalList.push_back(truthp);
      }
    } // end loop over truth particles

    // Loop over the content and add to MET 
    MissingETBase::Types::weight_t unitWeight(1.,1.,1.);
    MissingETBase::Types::weight_t minusWeight(-1.,-1.,1.);
    vector<const IParticle*> dummyList;
    for(const auto *iPart : signalList) {
      if(m_truth_type==MissingETBase::Source::TruthType::NonInt) {
	// flip direction for nonint
	this->addToMET(iPart,dummyList,metTerm,metMap,minusWeight);
      } else {
	this->addToMET(iPart,dummyList,metTerm,metMap,unitWeight);
      }
    }

    ATH_MSG_DEBUG( "Map contains " << (*MissingETComposition::find(metMap,metTerm))->objects().size() << " truth particles" );
 
    return StatusCode::SUCCESS;
  }

}
